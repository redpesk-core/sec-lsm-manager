/*
 * Copyright (C) 2020 IoT.bzh Company
 * Author: José Bollo <jose.bollo@iot.bzh>
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
