/*
 * Copyright (C) 2018-2024 IoT.bzh Company
 * Author: José Bollo <jose.bollo@iot.bzh>
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

#ifndef SEC_LSM_MANAGER_CLIENT_H
#define SEC_LSM_MANAGER_CLIENT_H

#include <features.h>
#include <stdbool.h>
#include <time.h>

/** abstract client type */
typedef struct client client_t;

/**
 * @brief Create a new client instance that use the given file descriptors.
 *        fdin can equal fdout.
 *
 * @param[in] pclient   pointer receiving the pointer to the created instance
 * @param[in] fdin      file descriptor of the input stream
 * @param[in] fdout     file descriptor of the output stream
 * @return 0 incase of success or a negative error code
 */
__wur __nonnull()
extern int client_create(client_t **pclient, int fdin, int fdout);

/**
 * @brief Check if the client is still connected
 *
 * @param[in] client pointer to the client instance
 * @return true is still connected, false if disconnected
 */
__wur __nonnull()
extern bool client_is_connected(client_t *client);

/**
 * @brief Disconnect the client of its pair
 *
 * @param[in] client pointer to the client instance
 */
__nonnull()
extern void client_disconnect(client_t *client);

/**
 * @brief Destroy the client instance, disconnecting it if connected
 *
 * @param[in] client pointer to the client instance
 */
__nonnull()
extern void client_destroy(client_t *client);

/**
 * @brief Process the available input if any.
 * A negative error code different from -EAGAIN
 * should imply a disconnection.
 *
 * @param[in] client pointer to the client instance
 * @return a strict positive number on success,
 *         0 if all input consumed but input terminated and to be closed
 *         -EAGAIN if all available input consumed
 *         -INVAL if disconnected
 *         -EPROTO on protocol violation
 *         -EMSGSIZE on too big messages
 *         other negative values are error
 */
__nonnull()
extern int client_process_input(client_t *client);

#endif /* PROTOCOL_CLIENT_H */

