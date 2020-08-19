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

#ifndef SECURITY_MANAGER_SECURE_APP_H
#define SECURITY_MANAGER_SECURE_APP_H

#include <sys/types.h>

#include "cynagora-interface.h"
#include "paths.h"

typedef struct secure_app {
    const char *id;
    policies_t policies;
    paths_t paths;
} secure_app_t;

/**
 * @brief Create the secure app and init it
 *
 * This secure app need to be destroy at the end
 * if the function succeeded
 *
 * @param[out] pointer to secure_app handler
 * @return 0 in case of success or a negative -errno value
 */
int create_secure_app(secure_app_t **secure_app) __wur;

/**
 * @brief Free id, paths and permissions
 * The pointer is not free
 *
 * @param[in] secure_app handler
 */
void free_secure_app(secure_app_t *secure_app);

/**
 * @brief Destroy the secure app
 * Free secure app handler and content
 *
 * @param[in] secure_app handler
 */
void destroy_secure_app(secure_app_t *secure_app);

/**
 * @brief Alloc and copy id in secure app
 *
 * @param[in] secure_app handler
 * @param[in] id The id to copy
 * @return 0 in case of success or a negative -errno value
 */
int secure_app_set_id(secure_app_t *secure_app, const char *id) __wur;

/**
 * @brief Add a new policy in policies field
 *
 * @param[in] secure_app handler
 * @param[in] permission The permission to add
 * @return 0 in case of success or a negative -errno value
 */
int secure_app_add_permission(secure_app_t *secure_app, const char *permission) __wur;

/**
 * @brief Add a new path in paths field
 *
 * @param[in] secure_app handler
 * @param[in] path The path to add
 * @param[in] path_type The path type to add
 * @return 0 in case of success or a negative -errno value
 */
int secure_app_add_path(secure_app_t *secure_app, const char *path, enum path_type path_type) __wur;

#endif