/*
 * Copyright (C) 2020 "IoT.bzh"
 * Author José Bollo <jose.bollo@iot.bzh>
 * Author Arthur Guyader <arthur.guyader@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef SECURITY_MANAGER_H
#define SECURITY_MANAGER_H

#include <stdint.h>

typedef struct security_manager security_manager_t;
typedef struct security_manager_handle security_manager_handle_t;

/**
 * @brief Create a client for server security_manager
 * The client is created but not connected. The connection is made on need.
 *
 * @param[in] security_manager   pointer to security_manager client handler
 * @param[in] socketspec specification of the socket to connect to or NULL for
 *                   using the default
 *
 * @return 0 in case of success and in that case *security_manager is filled
 *         a negative -errno value and *security_manager is set to NULL
 *
 * @see security_manager_destroy
 */
extern int security_manager_create(security_manager_t **security_manager, const char *socketspec);

/**
 * @brief Destroy security_manager client handler and release its memory
 *
 * @param[in] security_manager security_manager client handler
 *
 * @see security_manager_create
 */
extern void security_manager_destroy(security_manager_t *security_manager);

/**
 * Ask the security_manager client handler to disconnect from the server.
 * The client will reconnect if needed.
 *
 * @param[in] security_manager security_manager client handler
 */
extern void security_manager_disconnect(security_manager_t *security_manager);

/**
 * @brief Set id of security_manager client handler
 *
 * @param[in] security_manager security_manager client handler
 * @param[in] id The id to define
 * @return 0 in case of success or a negative -errno value
 */
extern int security_manager_set_id(security_manager_t *security_manager, const char *id);

/**
 * @brief Add a path to security_manager client handler
 *
 * @param security_manager security_manager client handler
 * @param path The path to add
 * @param path_type The path_type to add
 * @return 0 in case of success or a negative -errno value
 */
extern int security_manager_add_path(security_manager_t *security_manager, const char *path, const char *path_type);

/**
 * @brief Add a permission to security_manager client handler
 * You need to have set id before add a permission
 *
 * @param[in] security_manager security_manager client handler
 * @param[in] permission The permission to add
 * @return 0 in case of success or a negative -errno value
 */
extern int security_manager_add_permission(security_manager_t *security_manager, const char *permission);

/**
 * @brief Clean the security_manager client handler
 *
 * @param security_manager security_manager client handler
 * @return 0 in case of success or a negative -errno value
 */
extern int security_manager_clean(security_manager_t *security_manager);

/**
 * @brief Install an application with all defined paramters in the security manager handle
 * You need at least to set the id
 *
 * @param[in] security_manager security_manager client handler
 * @return 0 in case of success or a negative -errno value
 */
extern int security_manager_install(security_manager_t *security_manager);

/**
 * @brief Uninstall an application (cynagora permissions, paths)
 * You need at least to set the id
 *
 * @param[in] security_manager security_manager client handler
 * @return 0 in case of success or a negative -errno value
 */
extern int security_manager_uninstall(security_manager_t *security_manager);

/**
 * @brief Query or set the logging of requests
 *
 * @param[in] security_manager  security_manager client handler
 * @param[in] on                should set on
 * @param[in] off               should set off
 *
 * @return 0 if not logging, 1 if logging or a negative -errno value
 */
extern int security_manager_log(security_manager_t *security_manager, int on, int off);

/**
 * @brief Display the actual state security manager handle
 *
 * @param[in] security_manager security_manager client handler
 * @return 0 in case of success or a negative -errno value
 */
extern int security_manager_display(security_manager_t *security_manager);

#endif
