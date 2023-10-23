/*
 * Copyright (C) 2018-2023 IoT.bzh Company
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

#ifndef PROTOCOL_CLIENT_H
#define PROTOCOL_CLIENT_H

#include <features.h>
#include <stdbool.h>
#include <time.h>

/** abstract client type */
typedef struct client client_t;

/* see client.h */
__wur __nonnull()
extern int client_create(client_t **pclient, int fd, int pollfd);

/* see client.h */
__wur __nonnull()
extern bool client_is_connected(client_t *client);

/* see client.h */
__nonnull()
extern void client_disconnect(client_t *client);

/* see client.h */
__nonnull()
extern void client_disconnect_older(client_t *client, time_t trigger);

/* see client.h */
__nonnull()
extern void client_destroy(client_t *client);


#endif /* PROTOCOL_CLIENT_H */
