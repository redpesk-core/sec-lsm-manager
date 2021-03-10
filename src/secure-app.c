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
 * @brief Initialize the fields 'id', 'permission_set', 'path_set' and error_flag
 *
 * @param[in] secure_app handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int init_secure_app(secure_app_t *secure_app) {
    secure_app->id = NULL;
    int rc = init_path_set(&(secure_app->path_set));
    if (rc < 0) {
        ERROR("init_path_set");
        return rc;
    }

    rc = init_permission_set(&(secure_app->permission_set));
    if (rc < 0) {
        ERROR("init_permission_set");
        return rc;
    }

    secure_app->error_flag = false;

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

        free_permission_set(&(secure_app->permission_set));
        free_path_set(&(secure_app->path_set));
        secure_app->error_flag = false;
    }
}

/* see secure-app.h */
void destroy_secure_app(secure_app_t *secure_app) {
    free_secure_app(secure_app);
    free(secure_app);
}

/* see secure-app.h */
int secure_app_set_id(secure_app_t *secure_app, const char *id) {
    if (secure_app->id) {
        ERROR("id already set");
        return -EINVAL;
    }

    if (secure_app->error_flag) {
        ERROR("error flag has been raised");
        return -EPERM;
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
    if (secure_app->error_flag) {
        ERROR("error flag has been raised");
        return -EPERM;
    }

    for (size_t i = 0; i < secure_app->permission_set.size; i++) {
        if (!strcmp(secure_app->permission_set.permissions[i], permission)) {
            ERROR("permission already defined");
            return -EINVAL;
        }
    }

    int rc = permission_set_add_permission(&(secure_app->permission_set), permission);
    if (rc < 0) {
        ERROR("permission_set_add_permission");
        return rc;
    }

    return 0;
}

/* see secure-app.h */
int secure_app_add_path(secure_app_t *secure_app, const char *path, enum path_type path_type) {
    if (!valid_path_type(path_type)) {
        ERROR("path_type invalid");
        return -EINVAL;
    }

    if (secure_app->error_flag) {
        ERROR("error flag has been raised");
        return -EPERM;
    }

    for (size_t i = 0; i < secure_app->path_set.size; i++) {
        if (!strcmp(secure_app->path_set.paths[i].path, path)) {
            ERROR("path already defined");
            return -EINVAL;
        }
    }

    int rc = path_set_add_path(&(secure_app->path_set), path, path_type);
    if (rc < 0) {
        ERROR("path_set_add_path");
        return rc;
    }

    return 0;
}

/* see secure-app.h */
void raise_error_flag(secure_app_t *secure_app) { secure_app->error_flag = true; }