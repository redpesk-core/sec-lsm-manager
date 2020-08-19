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

#include "security-manager-operation.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "log.h"

typedef struct security_manager_operation {
    int (*install)(const secure_app_t *secure_app);
    int (*uninstall)(const secure_app_t *secure_app);
} security_manager_operation_t;

#ifdef WITH_SMACK
#include "smack.h"
static security_manager_operation_t security_manager_operation = {.install = install_smack,
                                                                  .uninstall = uninstall_smack};
#endif

#ifdef WITH_SELINUX
#include "selinux.h"
static security_manager_operation_t security_manager_operation = {.install = install_selinux,
                                                                  .uninstall = uninstall_selinux};
#endif

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Update the policy (drop the old and set the new)
 *
 * @param[in] sm_handle security_manager_handle handler
 * @return 0 in case of success or a negative -errno value
 */
static int update_policy(security_manager_handle_t *sm_handle) {
    if (!sm_handle) {
        ERROR("sm_handle undefined");
        return -EINVAL;
    } else if (!sm_handle->secure_app) {
        ERROR("secure_app undefined");
        return -EINVAL;
    } else if (!sm_handle->secure_app->id) {
        ERROR("id undefined");
        return -EINVAL;
    } else if (!sm_handle->cynagora_admin_client) {
        ERROR("cynagora_admin_client undefined");
        return -EINVAL;
    }

    // drop old policies
    int rc = cynagora_drop_policies(sm_handle->cynagora_admin_client, sm_handle->secure_app->id);
    if (rc < 0) {
        ERROR("cynagora_drop_policies");
        return rc;
    }

    // apply new policies
    rc = cynagora_set_policies(sm_handle->cynagora_admin_client, &(sm_handle->secure_app->policies));
    if (rc < 0) {
        ERROR("cynagora_set_policies");
        return rc;
    }

    return 0;
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see security-manager-operation.h */
int init_security_manager_handle(security_manager_handle_t *sm_handle) {
    if (!sm_handle) {
        ERROR("sm_handle undefined");
        return -EINVAL;
    }

    int rc = create_secure_app(&(sm_handle->secure_app));
    if (rc < 0) {
        ERROR("create_secure_app");
        sm_handle->secure_app = NULL;
        return rc;
    }

    rc = cynagora_create(&(sm_handle->cynagora_admin_client), cynagora_Admin, 1, 0);

    if (rc < 0) {
        ERROR("cynagora_create");
        sm_handle->cynagora_admin_client = NULL;
        destroy_secure_app(sm_handle->secure_app);
        sm_handle->secure_app = NULL;
        return rc;
    }

    return 0;
}

/* see security-manager-operation.h */
void free_security_manager_handle(security_manager_handle_t *sm_handle) {
    if (sm_handle) {
        destroy_secure_app(sm_handle->secure_app);
        sm_handle->secure_app = NULL;
        cynagora_destroy(sm_handle->cynagora_admin_client);
        sm_handle->cynagora_admin_client = NULL;
    }
}

/* see security-manager-operation.h */
int security_manager_handle_clean(security_manager_handle_t *sm_handle) {
    if (!sm_handle) {
        ERROR("sm_handle undefined");
        return -EINVAL;
    }

    free_secure_app(sm_handle->secure_app);
    return 0;
}

/* see security-manager-operation.h */
int security_manager_handle_set_id(security_manager_handle_t *sm_handle, const char *id) {
    if (!sm_handle) {
        ERROR("sm_handle undefined");
        return -EINVAL;
    } else if (!id) {
        ERROR("id undefined");
        return -EINVAL;
    }

    int rc = secure_app_set_id(sm_handle->secure_app, id);
    if (rc < 0) {
        ERROR("secure_app_set_id");
        return rc;
    }

    return 0;
}

/* see security-manager-operation.h */
int security_manager_handle_add_permission(security_manager_handle_t *sm_handle, const char *permission) {
    if (!sm_handle) {
        ERROR("sm_handle undefined");
        return -EINVAL;
    } else if (!sm_handle->secure_app) {
        ERROR("secure_app undefined");
        return -EINVAL;
    } else if (!permission) {
        ERROR("permission undefined");
        return -EINVAL;
    }

    int rc = secure_app_add_permission(sm_handle->secure_app, permission);
    if (rc < 0) {
        ERROR("secure_app_add_permission");
        return rc;
    }

    LOG("secure_app_add_permission success");
    return 0;
}

/* see security-manager-operation.h */
int security_manager_handle_add_path(security_manager_handle_t *sm_handle, const char *path, enum path_type path_type) {
    if (!sm_handle) {
        ERROR("sm_handle undefined");
        return -EINVAL;
    } else if (!sm_handle->secure_app) {
        ERROR("secure_app undefined");
        return -EINVAL;
    } else if (!path) {
        ERROR("path undefined");
        return -EINVAL;
    } else if (!valid_path_type(path_type)) {
        ERROR("path_type invalid");
        return -EINVAL;
    }

    int rc = secure_app_add_path(sm_handle->secure_app, path, path_type);
    if (rc < 0) {
        ERROR("secure_app_add_permission");
        return rc;
    }

    LOG("secure_app_add_path success");
    return 0;
}

/* see security-manager-operation.h */
int security_manager_handle_install(security_manager_handle_t *sm_handle) {
    if (!sm_handle) {
        ERROR("sm_handle undefined");
        return -EINVAL;
    } else if (!sm_handle->secure_app) {
        ERROR("secure_app undefined");
        return -EINVAL;
    } else if (!sm_handle->secure_app->id) {
        ERROR("secure_app id undefined");
        return -EINVAL;
    } else if (!sm_handle->cynagora_admin_client) {
        ERROR("Not valid cynagora_admin_client");
        return -EINVAL;
    }

    int rc = update_policy(sm_handle);
    if (rc < 0) {
        ERROR("update_policy");
        return rc;
    }

    LOG("update_policy success");

    rc = security_manager_operation.install(sm_handle->secure_app);
    if (rc < 0) {
        ERROR("install");
        int rc2 = cynagora_drop_policies(sm_handle->cynagora_admin_client, sm_handle->secure_app->id);
        if (rc2 < 0) {
            ERROR("cannot delete policy : %d %s", -rc2, strerror(-rc2));
        }
        return rc;
    }

    LOG("install success");

    return 0;
}

/* see security-manager-operation.h */
int security_manager_handle_uninstall(security_manager_handle_t *sm_handle) {
    if (!sm_handle) {
        ERROR("sm_handle undefined");
        return -EINVAL;
    } else if (!sm_handle->secure_app) {
        ERROR("secure_app undefined");
        return -EINVAL;
    } else if (sm_handle->secure_app->id == NULL) {
        ERROR("secure_app id undefined");
        return -EINVAL;
    } else if (!sm_handle->cynagora_admin_client) {
        ERROR("Not valid cynagora_admin_client");
        return -EINVAL;
    }

    int rc = cynagora_drop_policies(sm_handle->cynagora_admin_client, sm_handle->secure_app->id);

    if (rc < 0) {
        ERROR("cynagora_drop_policies : %d %s", -rc, strerror(-rc));
        return rc;
    }

    rc = security_manager_operation.uninstall(sm_handle->secure_app);

    if (rc < 0) {
        ERROR("uninstall");
        return rc;
    }

    LOG("uninstall success");
    return 0;
}
