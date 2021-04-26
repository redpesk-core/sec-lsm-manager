/*
 * Copyright (C) 2020-2021 IoT.bzh Company
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

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Label file
 *
 * @param[in] path The path of the file
 * @param[in] label The label to set
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int label_file(const char *path, const char *label) {
    if (!check_file_exists(path)) {
        DEBUG("%s not exist", path);
        return -EINVAL;
    }

    int rc = set_label(path, XATTR_NAME_SMACK, label);
    if (rc < 0) {
        ERROR("set_smack(%s,%s,%s) : %d %s", path, XATTR_NAME_SMACK, label, -rc, strerror(-rc));
        return rc;
    }

    return 0;
}

/**
 * @brief Label a directory to be transmute
 *
 * @param[in] path The path of the directory
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int label_dir_transmute(const char *path) {
    if (!check_file_type(path, __S_IFDIR)) {
        DEBUG("%s not directory", path);
        return 0;
    }

    int rc = set_label(path, XATTR_NAME_SMACKTRANSMUTE, "TRUE");
    if (rc < 0) {
        ERROR("set_smack(%s,%s,%s)", path, XATTR_NAME_SMACKTRANSMUTE, "TRUE");
        return rc;
    }

    return 0;
}

/**
 * @brief Label an executable file
 *
 * @param[in] path The path of the file
 * @param[in] label The label that will be used when exec
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int label_exec(const char *path, const char *label) {
    if (!check_file_type(path, __S_IFREG)) {
        DEBUG("%s not regular file", path);
        return 0;
    }

    if (!check_executable(path)) {
        ERROR("%s not executable", path);
        return 0;  // Check that it should not be restricted.
    }

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
    int rc = label_file(path, label);
    if (rc < 0) {
        ERROR("label file : %d %s", -rc, strerror(-rc));
        return rc;
    }

    if (is_executable) {
        rc = label_exec(path, label);
        if (rc < 0) {
            ERROR("label exec : %d %s", -rc, strerror(-rc));
            return rc;
        }
    }

    if (is_transmute) {
        rc = label_dir_transmute(path);
        if (rc < 0) {
            ERROR("label dir : %d %s", -rc, strerror(-rc));
            return rc;
        }
    }

    return 0;
}

/**
 * @brief Apply smack on a secure app
 *
 * @param[in] secure_app secure app handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int smack_process_paths(const secure_app_t *secure_app,
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

    rc = smack_process_paths(secure_app, path_type_definitions);
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
        ERROR("remove_app_rules : %d %s", -rc, strerror(-rc));
        return rc;
    }

    return 0;
}
