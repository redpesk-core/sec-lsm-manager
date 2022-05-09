/*
 * Copyright (C) 2020-2022 IoT.bzh Company
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
        ERROR("%s not end with %s", label, suffix_exec)
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
 * @brief Label a file
 *
 * @param[in] path The path of the file
 * @param[in] label The label of the file
 * @param[in] is_executable The file is an executable
 * @param[in] is_transmute The directory is transmute
 * @return 0 in case of success or a negative -errno value
 */
__nonnull((1, 2)) __wur
static int label_path(const char *path, const char *label, int is_executable, int is_transmute) {
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
            ERROR("set_label(%s,%s,%s) : %d %s ", path, XATTR_NAME_SMACKTRANSMUTE, "TRUE", -rc, strerror(-rc));
            return rc;
        }
    }

    return 0;
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
        rc = label_path(path->path, DROP_LABEL, 0, 0);
        if (rc < 0) {
            ERROR("label_path((%s,%s),%s) : %d %s", secure_app->path_set.paths[i]->path,
                  get_path_type_string(secure_app->path_set.paths[i]->path_type), secure_app->id, -rc, strerror(-rc));
            return rc;
        }
    }

    return 0;
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see smack.h */
int install_smack(const secure_app_t *secure_app) {
    if (secure_app->id[0] == '\0') {
        ERROR("id undefined");
        return -EINVAL;
    }

    int rc = create_smack_rules(secure_app);
    if (rc < 0) {
        ERROR("create_smack_rules : %d %s", -rc, strerror(-rc));
        goto end;
    }

    path_type_definitions_t path_type_definitions[number_path_type];
    init_path_type_definitions(path_type_definitions, secure_app->id);

    rc = smack_set_path_labels(secure_app, path_type_definitions);
    if (rc < 0) {
        ERROR("smack_process_paths : %d %s", -rc, strerror(-rc));
        goto error;
    }

    DEBUG("install smack success");

    goto end;

error:
    if (remove_smack_rules(secure_app) < 0) {
        ERROR("remove_smack_rules");
    }
end:
    return rc;
}

/* see smack.h */
int uninstall_smack(const secure_app_t *secure_app) {
    if (secure_app->id[0] == '\0') {
        ERROR("id undefined");
        return -EINVAL;
    }

    int rc = remove_smack_rules(secure_app);
    if (rc < 0) {
        ERROR("remove_smack_rules: %d %s", -rc, strerror(-rc));
        return rc;
    }

    rc = smack_drop_path_labels(secure_app);
    if (rc < 0) {
        ERROR("smack_drop_path_labels: %d %s", -rc, strerror(-rc));
        return rc;
    }

    return 0;
}
