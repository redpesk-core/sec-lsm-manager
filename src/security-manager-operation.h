/*
 * Copyright (C) 2020 IoT.bzh Company
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
