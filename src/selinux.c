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

#include "selinux.h"

#include <errno.h>
#include <selinux/restorecon.h>

#include "log.h"
#include "selinux-template.h"
#include "utils.h"

/**
 * @brief Restore the selinux context of a file
 *
 * @param[in] path The path of the file
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int restorecon(const char *path) {
    int rc = selinux_restorecon(path, SELINUX_RESTORECON_SET_SPECFILE_CTX);
    if (rc < 0) {
        rc = -errno;
        ERROR("selinux_restorecon");
        return rc;
    }

    return 0;
}

/**
 * @brief Apply selinux label on files contain in paths
 *
 * @param[in] paths paths handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int apply_selinux_label(const path_set_t *paths) {
    for (size_t i = 0; i < paths->size; i++) {
        if (check_file_exists(paths->paths[i].path)) {
            int rc = restorecon(paths->paths[i].path);
            if (rc < 0) {
                ERROR("restorecon");
                return rc;
            }
        }
    }

    return 0;
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see selinux.h */
int install_selinux(const secure_app_t *secure_app) {
    // ################## CREATE ##################
    int rc = create_selinux_rules(secure_app, NULL, NULL, NULL);
    if (rc < 0) {
        ERROR("create_selinux_rules");
        return rc;
    }

    // ############### CHECK AFTER ###############
    if (!check_module_files_exist(secure_app, NULL)) {
        ERROR("module files not exist");
        return -ENOENT;
    }

    if (!check_module_in_policy(secure_app)) {
        ERROR("module not in the policy");
        return -ENOENT;
    }

    // force label
    rc = apply_selinux_label(&(secure_app->path_set));
    if (rc < 0) {
        ERROR("apply_selinux_label");
        return rc;
    }

    return 0;
}

/* see selinux.h */
int uninstall_selinux(const secure_app_t *secure_app) {
    // ############### CHECK BEFORE ###############
    if (!check_module_files_exist(secure_app, NULL)) {
        ERROR("module files not exist");
        return -ENOENT;
    }

    if (!check_module_in_policy(secure_app)) {
        ERROR("module not in the policy");
        return -ENOENT;
    }

    // ################## REMOVE ##################
    int rc = remove_selinux_rules(secure_app, NULL);

    if (rc < 0) {
        ERROR("remove_selinux_rules");
        return rc;
    }

    // ############### CHECK AFTER ###############
    if (check_module_files_exist(secure_app, NULL)) {
        ERROR("module files exist");
        return -1;
    }

    if (check_module_in_policy(secure_app)) {
        ERROR("module in the policy");
        return -1;
    }

    return 0;
}
