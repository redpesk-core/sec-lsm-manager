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

#ifndef SECURITY_MANAGER_OPERATION_H
#define SECURITY_MANAGER_OPERATION_H

#include "secure-app.h"

typedef struct security_manager_handle {
    cynagora_t *cynagora_admin_client;
    secure_app_t *secure_app;
} security_manager_handle_t;

/**
 * @brief Init and allocate components of security_manager handle
 * The pointer is not allocate.
 * @param[in] sm_handle security_manager_handle_t handler
 * @return 0 in case of success or a negative -errno value
 *
 * @see free_security_manager_handle security_manager_handle_clean
 */
int init_security_manager_handle(security_manager_handle_t *sm_handle) __wur;

/**
 * @brief Free components of security_manager
 * The pointer is not free
 * @param[in] sm_handle security_manager_handle_t handler
 * @see init_security_manager_handle security_manager_handle_clean
 */
void free_security_manager_handle(security_manager_handle_t *sm_handle);

/**
 * @brief Reset security_manager handle to define a new secure app
 *
 * @param[in] sm_handle security_manager_handle_t handler
 * @return 0 in case of success or a negative -errno value
 *
 * @see init_security_manager_handle free_security_manager_handle
 */
int security_manager_handle_clean(security_manager_handle_t *sm_handle) __wur;

/**
 * @brief Set id of the secure app structure contains in security_manager_handle
 *
 * @param[in] sm_handle security_manager_handle_t handler
 * @param[in] id id of the application
 * @return 0 in case of success or a negative -errno value
 *
 * @see security_manager_handle_install security_manager_handle_uninstall security_manager_handle_add_permission
 */
int security_manager_handle_set_id(security_manager_handle_t *sm_handle, const char *id) __wur;

/**
 * @brief Add a permission that will be emit to cynagora
 * To add a permission, id need to be set
 *
 * @param[in] sm_handle security_manager_handle_t handler
 * @param[in] permission string permission
 * @return 0 in case of success or a negative -errno value
 */
int security_manager_handle_add_permission(security_manager_handle_t *sm_handle, const char *permission) __wur;

/**
 * @brief Add a path that will set MAC label
 * (SMACK or SELINUX)
 *
 * @param[in] sm_handle security_manager_handle_t handler
 * @param[in] path path to target file
 * @param[in] path_type type of the file (conf, bin, data, ...)
 * @return 0 in case of success or a negative -errno value
 */
int security_manager_handle_add_path(security_manager_handle_t *sm_handle, const char *path,
                                     enum path_type path_type) __wur;

/**
 * @brief Install application (SMACK or SELinux)
 *
 * @param[in] sm_handle security_manager_handle_t handler
 * @return 0 in case of success or a negative -errno value
 */
int security_manager_handle_install(security_manager_handle_t *sm_handle) __wur;

/**
 * @brief Uninstall application (SMACK or SELinux)
 *
 * @param[in] sm_handle security_manager_handle_t handler
 * @return 0 in case of success or a negative -errno value
 */
int security_manager_handle_uninstall(security_manager_handle_t *sm_handle) __wur;

#endif