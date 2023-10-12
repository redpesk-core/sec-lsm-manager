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

#include "selinux.h"

#include <errno.h>
#include <linux/xattr.h>
#include <stdio.h>
#include <string.h>
#include <sys/xattr.h>
#include <stdio.h>

#include "log.h"
#include "selinux-template.h"
#include "utils.h"

#if WITH_SELINUX
__wur __nonnull()
int install_mac(const context_t *context)
         __attribute__ ((alias ("install_selinux")));

__wur __nonnull()
int uninstall_mac(const context_t *context)
         __attribute__ ((alias ("uninstall_selinux")));

__nonnull()
void app_label_mac(char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1], const char *appid, const char *app_id)
         __attribute__ ((alias ("app_label_selinux")));
#endif

/**
 * @brief Label file
 *
 * @param[in] path The path of the file
 * @param[in] label The label to set
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int label_file(const char *path, const char *label) {
    bool exists;
    get_file_informations(path, &exists, NULL, NULL);
    if (!exists) {
        DEBUG("%s not exists", path);
        return -ENOENT;
    }

    int rc = set_label(path, XATTR_NAME_SELINUX, label);
    if (rc < 0) {
        ERROR("set_label(%s,%s,%s) : %d %s", path, XATTR_NAME_SELINUX, label, -rc, strerror(-rc));
        return rc;
    }

    return 0;
}

/**
 * @brief Apply selinux on a context
 *
 * @param[in] context context handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur int selinux_process_paths(const context_t *context,
                                            path_type_definitions_t path_type_definitions[number_path_type]) {
    path_t *path = NULL;
    char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 3];
    for (size_t i = 0; i < context->path_set.size; i++) {
        path = context->path_set.paths[i];
        snprintf(label, SEC_LSM_MANAGER_MAX_SIZE_LABEL + 3, "%s:s0", path_type_definitions[path->path_type].label);
        int rc = label_file(path->path, label);
        if (rc < 0) {
            ERROR("label_file((%s,%s),%s) : %d %s", path->path, get_path_type_string(path->path_type), context->id,
                  -rc, strerror(-rc));
            return rc;
        }
    }

    return 0;
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see selinux-label.h */
bool selinux_enabled(void) {
    if (is_selinux_enabled() == 1) {
        return true;
    }
    return false;
}

/* see selinux.h */
int install_selinux(const context_t *context) {
    /* TODO: treat the case where !context->need_id */
    if (context->id[0] == '\0') {
        ERROR("id undefined");
        return -EINVAL;
    }

    path_type_definitions_t path_type_definitions[number_path_type];
    init_path_type_definitions(path_type_definitions, context->id_underscore);

    // ################## CREATE ##################
    int rc = create_selinux_rules(context, path_type_definitions);
    if (rc < 0) {
        ERROR("create_selinux_rules : %d %s", -rc, strerror(-rc));
        return rc;
    }

    // ############### CHECK AFTER ###############
    if (!check_module_files_exist(context)) {
        ERROR("module files not exist");
        return -ENOENT;
    }

    DEBUG("success check files exist");

    if (!check_module_in_policy(context)) {
        ERROR("module not in the policy");
        return -ENOENT;
    }

    DEBUG("success check module in policy");

    // force label
    rc = selinux_process_paths(context, path_type_definitions);
    if (rc < 0) {
        ERROR("selinux_process_paths : %d %s", -rc, strerror(-rc));
        return rc;
    }

    DEBUG("success apply selinux label");

    return 0;
}

/* see selinux.h */
int uninstall_selinux(const context_t *context) {
    /* TODO: treat the case where !context->need_id */
    if (context->id[0] == '\0') {
        ERROR("id undefined");
        return -EINVAL;
    }

    // ############### CHECK BEFORE ###############
    if (!check_module_files_exist(context)) {
        ERROR("module files not exist");
        return -ENOENT;
    }

    if (!check_module_in_policy(context)) {
        ERROR("module not in the policy");
        return -ENOENT;
    }

    // ################## REMOVE ##################
    int rc = remove_selinux_rules(context);

    if (rc < 0) {
        ERROR("remove_selinux_rules : %d %s", -rc, strerror(-rc));
        return rc;
    }

    DEBUG("success remove selinux module and files");

    // ############### CHECK AFTER ###############
    if (check_module_files_exist(context)) {
        ERROR("module files exist");
        return -1;
    }

    DEBUG("success check files removed");

    if (check_module_in_policy(context)) {
        ERROR("module in the policy");
        return -1;
    }

    DEBUG("success check module removed");

    return 0;
}

/* see selinux.h */
__nonnull()
void app_label_selinux(char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1], const char *appid, const char *app_id)
{
    (void)appid;
    snprintf(label, SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1, "system_u:system_r:%s_t:s0", app_id);
}
