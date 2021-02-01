/*
 * Copyright (C) 2020-2021 IoT.bzh Company
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

#ifndef SECURITY_MANAGER_SERVER_H
#define SECURITY_MANAGER_SERVER_H

#include <sys/cdefs.h>

typedef struct security_manager_server security_manager_server_t;

/**
 * @brief Boolean flag telling whether the server logs or not its received commands
 */
extern int security_manager_server_log;

/**
 * @brief Create a security manager server
 *
 * @param[out] server where to store the handler of the created server
 * @param[in] socket_spec specification of socket
 *
 * @return 0 on success or a negative value
 *
 * @see security_manager_server_destroy
 */
extern int security_manager_server_create(security_manager_server_t **server,
                                          const char *security_manager_socket_spec) __wur;

/**
 * @brief Destroy a created server and release its resources
 *
 * @param[in] server the handler of the server
 *
 * @see security_manager_server_create
 */
extern void security_manager_server_destroy(security_manager_server_t *server) __nonnull();

/**
 * @brief Start the security_manager server and returns only when stopped
 *
 * @param[in] server the handler of the server
 *
 * @return 0 on success or a negative value
 *
 * @see security_manager_server_stop
 */
extern int security_manager_server_serve(security_manager_server_t *server) __nonnull() __wur;

/**
 * @brief Stop the security_manager server
 *
 * @param[in] server the handler of the server
 * @param[in] status the status that the function security_manager_server_serve should return
 *
 * @see security_manager_server_serve
 */
extern void security_manager_server_stop(security_manager_server_t *server, int status) __nonnull();

#endif
