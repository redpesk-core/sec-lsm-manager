/*
 * Copyright (C) 2020 "IoT.bzh"
 * Author Arthur Guyader <arthur.guyader@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
static int restorecon(const char *path) {
    if (!path) {
        ERROR("path undefined");
        return -EINVAL;
    }

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
static int apply_selinux_label(const paths_t *paths) {
    if (!paths) {
        ERROR("paths undefined");
        return -EINVAL;
    }

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
    if (!secure_app) {
        ERROR("secure_app undefined");
        return -EINVAL;
    }

    // ################## CREATE ##################
    int rc = create_selinux_rules(secure_app, NULL, NULL, NULL);

    if (rc < 0) {
        ERROR("create_selinux_rules");
        return rc;
    }

    // ############### CHECK AFTER ###############
    rc = check_module_files_exist(secure_app, NULL);
    if (rc < 0) {
        ERROR("check_module_files_exist");
        return rc;
    } else if (rc != 1) {
        ERROR("module files not exist");
        return -1;
    }

    rc = check_module_in_policy(secure_app);
    if (rc < 0) {
        ERROR("check_module_in_policy");
        return rc;
    } else if (rc != 1) {
        ERROR("module not in the policy");
        return -1;
    }

    // force label
    rc = apply_selinux_label(&(secure_app->paths));
    if (rc < 0) {
        ERROR("apply_selinux_label");
        return rc;
    }

    return 0;
}

/* see selinux.h */
int uninstall_selinux(const secure_app_t *secure_app) {
    if (!secure_app) {
        ERROR("secure_app undefined");
        return -EINVAL;
    }

    // ############### CHECK BEFORE ###############
    int rc = check_module_files_exist(secure_app, NULL);
    if (rc < 0) {
        ERROR("check_module_files_exist");
    } else if (rc != 1) {
        ERROR("module files not exist");
    }

    rc = check_module_in_policy(secure_app);
    if (rc < 0) {
        ERROR("check_module_in_policy");
    } else if (rc != 1) {
        ERROR("module not in the policy");
    }

    // ################## REMOVE ##################
    rc = remove_selinux_rules(secure_app, NULL);

    if (rc < 0) {
        ERROR("remove_selinux_rules");
        return rc;
    }

    // ############### CHECK AFTER ###############
    rc = check_module_files_exist(secure_app, NULL);
    if (rc < 0) {
        ERROR("check_module_files_exist");
        return rc;
    } else if (rc != 0) {
        ERROR("module files exist");
        return -1;
    }

    rc = check_module_in_policy(secure_app);
    if (rc < 0) {
        ERROR("check_module_in_policy");
        return rc;
    } else if (rc != 0) {
        ERROR("module in the policy");
        return -1;
    }

    return 0;
}