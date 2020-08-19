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

#include "secure-app.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "log.h"

#ifdef WITH_SMACK
#include "smack-label.h"
#endif

#include "utils.h"

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Initialize the fields 'id', 'policies' and 'paths'
 *
 * @param[in] secure_app handler
 * @return 0 in case of success or a negative -errno value
 */
static int init_secure_app(secure_app_t *secure_app) {
    if (!secure_app) {
        ERROR("secure_app undefined");
        return -EINVAL;
    }

    secure_app->id = NULL;
    int rc = init_paths(&(secure_app->paths));
    if (rc < 0) {
        ERROR("init_paths");
        return rc;
    }

    rc = init_policies(&(secure_app->policies));
    if (rc < 0) {
        ERROR("init_policies");
        return rc;
    }

    return 0;
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see secure-app.h */
int create_secure_app(secure_app_t **secure_app) {
    *secure_app = (secure_app_t *)malloc(sizeof(secure_app_t));
    if (*secure_app == NULL) {
        ERROR("malloc secure_app_t");
        return -ENOMEM;
    }

    int rc = init_secure_app(*secure_app);
    if (rc < 0) {
        ERROR("init_secure_app");
        free(*secure_app);
        *secure_app = NULL;
        return rc;
    }

    return 0;
}

/* see secure-app.h */
void free_secure_app(secure_app_t *secure_app) {
    if (secure_app) {
        free((void *)secure_app->id);
        secure_app->id = NULL;

        free_policies(&(secure_app->policies));
        free_paths(&(secure_app->paths));
    }
}

/* see secure-app.h */
void destroy_secure_app(secure_app_t *secure_app) {
    if (secure_app) {
        free_secure_app(secure_app);
        free(secure_app);
    }
}

/* see secure-app.h */
int secure_app_set_id(secure_app_t *secure_app, const char *id) {
    if (!secure_app) {
        ERROR("secure_app is undefined");
        return -EINVAL;
    } else if (!id) {
        ERROR("id undefined");
        return -EINVAL;
    } else if (secure_app->id) {
        return 1;
    }

    secure_app->id = strdup(id);
    if (secure_app->id == NULL) {
        ERROR("strdup id");
        return -ENOMEM;
    }

    return 0;
}

/* see secure-app.h */
int secure_app_add_permission(secure_app_t *secure_app, const char *permission) {
    if (!secure_app) {
        ERROR("secure_app undefined");
        return -EINVAL;
    } else if (!permission) {
        ERROR("permission undefined");
        return -EINVAL;
    } else if (!secure_app->id) {
        ERROR("secure_app id undefined");
        return -EINVAL;
    }

    cynagora_key_t k = {secure_app->id, INSERT_ALL, INSERT_ALL, permission};
    cynagora_value_t v = {AUTHORIZED, 0};

    int rc = policies_add_policy(&(secure_app->policies), &k, &v);
    if (rc < 0) {
        ERROR("policies_add_policy");
        return rc;
    }

    return 0;
}

/* see secure-app.h */
int secure_app_add_path(secure_app_t *secure_app, const char *path, enum path_type path_type) {
    if (!secure_app) {
        ERROR("secure_app undefined");
        return -EINVAL;
    } else if (!path) {
        ERROR("path undefined");
        return -EINVAL;
    } else if (!valid_path_type(path_type)) {
        ERROR("path_type invalid");
        return -EINVAL;
    }

    int rc = paths_add_path(&(secure_app->paths), path, path_type);
    if (rc < 0) {
        ERROR("paths_add_path");
        return rc;
    }

    return 0;
}
