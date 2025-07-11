/*
 * Copyright (C) 2020-2025 IoT.bzh Company
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
#include "file-utils.h"
#include "xattr-selinux.h"

#if WITH_SELINUX
#include "action/mac-interface.h"
__wur __nonnull()
int mac_install(const context_t *context)
         __attribute__ ((alias ("selinux_install")));

__wur __nonnull()
int mac_uninstall(const context_t *context)
         __attribute__ ((alias ("selinux_uninstall")));

__nonnull()
void mac_get_label(char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1], const char *appid)
         __attribute__ ((alias ("selinux_get_label")));
#endif

/**
 *
 */
__nonnull()
static void trfid(const char *id, char _id_[SEC_LSM_MANAGER_MAX_SIZE_ID + 1])
{
    char c;
    unsigned idx = 0;
    do {
        c = id[idx];
        _id_[idx++] = c == '-' ? '_' : c;
    } while(c && idx < sizeof idx);
    _id_[SEC_LSM_MANAGER_MAX_SIZE_ID] = '\0';
}

/**
 * @brief Label file
 *
 * @param[in] path The path of the file
 * @param[in] label The label to set
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int label_file(const char *path, const char *label) {
    bool exists;
    get_file_informations(path, false, &exists, NULL, NULL);
    if (!exists) {
        DEBUG("%s not exists", path);
        return -ENOENT;
    }

    int rc = set_xattr(path, XATTR_NAME_SELINUX, label);
    if (rc < 0) {
        ERROR("set_xattr(%s,%s,%s) : %d %s", path, XATTR_NAME_SELINUX, label, -rc, strerror(-rc));
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
            ERROR("label_file((%s,%s),%s) : %d %s", path->path, path_type_name(path->path_type), context->id,
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
int selinux_install(const context_t *context) {
    char _id_[SEC_LSM_MANAGER_MAX_SIZE_ID + 1];
    /* TODO: treat the case where !context->need_id */
    if (context->id[0] == '\0') {
        ERROR("id undefined");
        return -EINVAL;
    }

    path_type_definitions_t path_type_definitions[number_path_type];
    trfid(context->id, _id_);
    init_path_type_definitions(path_type_definitions, _id_);

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
int selinux_uninstall(const context_t *context) {
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
void selinux_get_label(char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1], const char *appid)
{
    char _id_[SEC_LSM_MANAGER_MAX_SIZE_ID + 1];
    trfid(appid, _id_);
    snprintf(label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "system_u:system_r:%s_t:s0", _id_);
    label[SEC_LSM_MANAGER_MAX_SIZE_ID] = '\0';
}
