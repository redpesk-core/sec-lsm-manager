/*
 * Copyright (C) 2020-2023 IoT.bzh Company
 * Author: Arthur Guyader <arthur.guyader@iot.bzh>
 *
 * $RP_BEGIN_LICENSE$
 * Commercial License Usage
 *  Licensees holding valid commercial IoT.bzh licenses may use this file in
 *  accordance with the commercial license agreement provided with the
 *  Software or, alternatively, in accordance with the terms contained in
 *  a written agreement between you and The IoT.bzh Company. For licensing terms
 *  and conditions see https://www.iot.bzh/terms-conditions. For further
 *  information use the contact form at https://www.iot.bzh/contact.
 *
 * GNU General Public License Usage
 *  Alternatively, this file may be used under the terms of the GNU General
 *  Public license version 3. This license is as published by the Free Software
 *  Foundation and appearing in the file LICENSE.GPLv3 included in the packaging
 *  of this file. Please review the following information to ensure the GNU
 *  General Public License requirements will be met
 *  https://www.gnu.org/licenses/gpl-3.0.html.
 * $RP_END_LICENSE$
 */

#include "smack.h"

#include <errno.h>
#include <linux/xattr.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

#include "log.h"
#include "smack-template.h"
#include "utils.h"

#define DROP_LABEL "User:Home"

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Label an executable file
 *
 * @param[in] path The path of the file
 * @param[in] label The label that will be used when exec
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int label_exec(const char *path, const char *label) {
    char *test_exec = strstr(label, suffix_exec);
    if (test_exec == NULL || strcmp(test_exec, suffix_exec)) {
        ERROR("%s not end with %s", label, suffix_exec);
        return -EINVAL;
    }

    // remove :Exec (SMACK64EXEC)
    char *label_no_exec = strdupa(label);
    if (!label_no_exec) {
        return -ENOMEM;
    }

    label_no_exec[strlen(label_no_exec) - strlen(suffix_exec)] = '\0';

    int rc = set_label(path, XATTR_NAME_SMACKEXEC, label_no_exec);
    if (rc < 0) {
        ERROR("set_smack(%s,%s,%s) : %d %s", path, XATTR_NAME_SMACKEXEC, label_no_exec, -rc, strerror(-rc));
        return rc;
    }

    return 0;
}

/**
 * @brief Remove labels of a path entry
 *
 * @param[in] path The path of the file
 * @return 0 in case of success or a negative -errno value
 */
static int unset_smack_labels(const char *path) {
    bool exists, is_exec, is_dir;
    get_file_informations(path, &exists, &is_exec, &is_dir);

    DEBUG("%s : exists=%d ; exec=%d ; dir=%d", path, exists, is_exec, is_dir);

    if (!exists) {
        return -ENOENT;
    }

    // file
    int rc = unset_label(path, XATTR_NAME_SMACK);
    if (rc < 0) {
        ERROR("unlabel(%s): %d %s", path, -rc, strerror(-rc));
        return rc;
    }

    // exec
    if (is_exec) {
        rc = unset_label(path, XATTR_NAME_SMACKEXEC);
        if (rc < 0) {
            ERROR("unlabel exec(%s): %d %s", path, -rc, strerror(-rc));
            return rc;
        }
    }

    // dir
    if (is_dir) {
        rc = unset_label(path, XATTR_NAME_SMACKTRANSMUTE);
        if (rc < 0) {
            ERROR("unlabel transmute(%s): %d %s", path, -rc, strerror(-rc));
            return rc;
        }
    }

    return 0;
}

/**
 * @brief Label a path entry
 *
 * @param[in] path The path of the file
 * @param[in] label The label of the file
 * @param[in] is_executable The file is an executable
 * @param[in] is_transmute The directory is transmute
 * @return 0 in case of success or a negative -errno value
 */
__nonnull((1, 2)) __wur
static int set_smack_labels(const char *path, const char *label, bool is_executable, bool is_transmute) {
    bool exists, is_exec, is_dir;
    get_file_informations(path, &exists, &is_exec, &is_dir);

    DEBUG("%s : exists=%d ; exec=%d ; dir=%d", path, exists, is_exec, is_dir);

    if (!exists) {
        return -ENOENT;
    }

    // file
    int rc = set_label(path, XATTR_NAME_SMACK, label);
    if (rc < 0) {
        ERROR("set_label(%s,%s,%s) : %d %s", path, XATTR_NAME_SMACK, label, -rc, strerror(-rc));
        return rc;
    }

    // exec
    if (is_executable && is_exec) {
        rc = label_exec(path, label);
        if (rc < 0) {
            ERROR("label exec : %d %s", -rc, strerror(-rc));
            return rc;
        }
    }

    // dir
    if (is_transmute && is_dir) {
        rc = set_label(path, XATTR_NAME_SMACKTRANSMUTE, "TRUE");
        if (rc < 0) {
            ERROR("set_label(%s,%s,%s) : %d %s", path, XATTR_NAME_SMACKTRANSMUTE, "TRUE", -rc, strerror(-rc));
            return rc;
        }
    }

    return 0;
}

/**
 * @brief Label a file
 *
 * @param[in] path The path of the file
 * @param[in] label The label of the file
 * @param[in] is_executable The file is an executable
 * @param[in] is_transmute The directory is transmute
 * @return 0 in case of success or a negative -errno value
 */
__nonnull((1, 2)) __wur
static int label_path(const char *path, const char *label, bool is_executable, bool is_transmute) {
    if (label[0])
        return set_smack_labels(path, label, is_executable, is_transmute);
    else
        return unset_smack_labels(path);
}

/**
 * @brief Set smack labels for secure app
 *
 * @param[in] secure_app secure app handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int smack_set_path_labels(const secure_app_t *secure_app,
                                                 path_type_definitions_t path_type_definitions[number_path_type]) {
    int rc = 0;
    path_t *path = NULL;
    for (size_t i = 0; i < secure_app->path_set.size; i++) {
        path = secure_app->path_set.paths[i];
        rc = label_path(path->path, path_type_definitions[path->path_type].label,
                        path_type_definitions[path->path_type].is_executable,
                        path_type_definitions[path->path_type].is_transmute);

        if (rc < 0) {
            ERROR("label_path((%s,%s),%s) : %d %s", secure_app->path_set.paths[i]->path,
                  get_path_type_string(secure_app->path_set.paths[i]->path_type), secure_app->id, -rc, strerror(-rc));
            return rc;
        }
    }

    return 0;
}

/**
 * @brief Removes smack labels for secure app
 *
 * @param[in] secure_app secure app handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int smack_drop_path_labels(const secure_app_t *secure_app) {
    int rc = 0;
    path_t *path = NULL;
    for (size_t i = 0; i < secure_app->path_set.size; i++) {
        path = secure_app->path_set.paths[i];
        rc = label_path(path->path, DROP_LABEL, false, false);
        if (rc < 0) {
            ERROR("label_path((%s,%s),%s) : %d %s", secure_app->path_set.paths[i]->path,
                  get_path_type_string(secure_app->path_set.paths[i]->path_type), secure_app->id, -rc, strerror(-rc));
            return rc;
        }
    }

    return 0;
}

/**
* @brief Installs the files for plugs
*
* @param[in] secure_app the application specification
 * @return 0 in case of success or a negative -errno value
*/
__nonnull() __wur
static int install_smack_plugs(const secure_app_t *secure_app)
{
    char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1];
    char buffer[PATH_MAX + 1];
    plug_t *plugit;
    int rc2, rc = 0;

    for (plugit = secure_app->plugset ; plugit != NULL ; plugit = plugit->next) {
        rc2 = snprintf(buffer, sizeof buffer, "%s/%s", plugit->impdir, secure_app->id);
        if (rc2 > PATH_MAX)
            rc2 = -ENAMETOOLONG;
        if (rc2 < 0)
            ERROR("install_smack_plugs: can't set name: %d, %s", -rc2, strerror(-rc2));
        else {
            rc2 = symlink(plugit->expdir, buffer);
            if (rc2 < 0) {
                rc2 = -errno;
                ERROR("install_smack_plugs: can't create link: %d, %s", -rc2, strerror(-rc2));
            }
            else {
                app_label_smack(label, secure_app->id, secure_app->id_underscore);
                rc2 = set_label(buffer, XATTR_NAME_SMACK, label);
            }
        }
        if (rc2 < 0 && rc == 0)
            rc = rc2;
    }
    return rc;
}

/**
* @brief Uninstalls the files for plugs
*
* @param[in] secure_app the application specification
 * @return 0 in case of success or a negative -errno value
*/
__nonnull() __wur
static int uninstall_smack_plugs(const secure_app_t *secure_app)
{
    char buffer[PATH_MAX + 1];
    plug_t *plugit;
    int rc2, rc = 0;

    for (plugit = secure_app->plugset ; plugit != NULL ; plugit = plugit->next) {
        rc2 = snprintf(buffer, sizeof buffer, "%s/%s", plugit->impdir, secure_app->id);
        if (rc2 > PATH_MAX)
            rc2 = -ENAMETOOLONG;
        if (rc2 < 0)
            ERROR("uninstall_smack_plugs: can't set name: %d, %s", -rc2, strerror(-rc2));
        else {
            rc2 = unlink(buffer);
            if (rc2 < 0) {
                if (errno == ENOENT)
                    rc2 = 0;
                else {
                    rc2 = -errno;
                    ERROR("uninstall_smack_plugs: can't unlink: %d, %s", -rc2, strerror(-rc2));
                }
            }
        }
        if (rc2 < 0 && rc == 0)
            rc = rc2;
    }
    return rc;
}

/**
 * @brief Uninstall a secure app for smack without stopping on failure
 *
 * @param[in] secure_app secure app handler
 * @return 0 in case of success or a negative -errno value
 */
static int force_uninstall_smack(const secure_app_t *secure_app)
{
    int rc2, rc;

    rc = smack_drop_path_labels(secure_app);
    if (rc < 0)
        ERROR("smack_drop_path_labels: %d %s", -rc, strerror(-rc));

    if (secure_app->id[0] != '\0') {
        rc2 = uninstall_smack_plugs(secure_app);
        if (rc2 < 0) {
            rc = rc2;
            ERROR("uninstall_smack_plugs: %d %s", -rc, strerror(-rc));
        }
        rc2 = remove_smack_rules(secure_app);
        if (rc2 < 0) {
            rc = rc2;
            ERROR("remove_smack_rules: %d %s", -rc, strerror(-rc));
        }
    }

    return rc;
}


/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see smack.h */
int install_smack(const secure_app_t *secure_app) {
    int rc = 0;
    bool has_id;
    path_type_definitions_t path_type_definitions[number_path_type];

    has_id = secure_app->id[0] != '\0';
    if (!has_id && secure_app->need_id) {
        ERROR("id undefined");
        return -EINVAL;
    }

    init_path_type_definitions(path_type_definitions, secure_app->id);
    if (has_id) {
        rc = create_smack_rules(secure_app);
        if (rc < 0)
            ERROR("create_smack_rules failed: %d %s", -rc, strerror(-rc));
        else {
            rc = install_smack_plugs(secure_app);
            if (rc < 0)
                ERROR("install_smack_plugs failed: %d %s", -rc, strerror(-rc));
        }
    }

    if (rc >= 0) {
        rc = smack_set_path_labels(secure_app, path_type_definitions);
        if (rc < 0)
            ERROR("smack_set_path_labels failed: %d %s", -rc, strerror(-rc));
        else
            DEBUG("install smack success");
    }

    if (rc < 0)
        force_uninstall_smack(secure_app);

    return rc;
}

/* see smack.h */
int uninstall_smack(const secure_app_t *secure_app) {
    if (secure_app->id[0] == '\0' && secure_app->need_id) {
        ERROR("id undefined");
        return -EINVAL;
    }

    return force_uninstall_smack(secure_app);
}

/* see smack.h */
__nonnull()
void app_label_smack(char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1], const char *appid, const char *app_id)
{
    (void)app_id;
    snprintf(label, SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1, "App:%s", appid);
}
