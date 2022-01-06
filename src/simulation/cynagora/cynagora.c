/*
 * Copyright (C) 2018-2022 IoT.bzh Company
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

#include "cynagora.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../../log.h"

typedef struct asreq asreq_t;
typedef struct ascb ascb_t;
typedef struct agent agent_t;
typedef struct query query_t;
typedef struct cynagora cynagora_t;

/******************************************************************************/
/*** PUBLIC COMMON METHODS                                                  ***/
/******************************************************************************/

/* see cynagora.h */
int cynagora_create(cynagora_t **prcyn, cynagora_type_t type, uint32_t cache_size, const char *socketspec) {
    printf("cynagora_create(%d, %d, %s)\n", type, cache_size, socketspec);
    *prcyn = (cynagora_t *)malloc(sizeof(int));
    if (*prcyn == NULL) {
        ERROR("malloc cynagora_t failed");
        return -ENOMEM;
    }
    return 0;
}

/* see cynagora.h */
void cynagora_disconnect(cynagora_t *cynagora) { printf("cynagora_disconnect(%p)\n", cynagora); }

/* see cynagora.h */
void cynagora_destroy(cynagora_t *cynagora) {
    printf("cynagora_destroy(%p)\n", cynagora);
    free(cynagora);
}

/* see cynagora.h */
int cynagora_async_setup(cynagora_t *cynagora, cynagora_async_ctl_cb_t *controlcb, void *closure) {
    printf("cynagora_async_setup(%p,%p,%p)\n", cynagora, controlcb, closure);
    return 0;
}

/* see cynagora.h */
int cynagora_async_process(cynagora_t *cynagora) {
    printf("cynagora_async_process(%p)\n", cynagora);
    return 0;
}

/* see cynagora.h */
int cynagora_cache_resize(cynagora_t *cynagora, uint32_t size) {
    printf("cynagora_cache_resize(%p, %d)\n", cynagora, size);
    return 0;
}

/* see cynagora.h */
void cynagora_cache_clear(cynagora_t *cynagora) { printf("cynagora_cache_clear(%p)\n", cynagora); }

/* see cynagora.h */
int cynagora_cache_check(cynagora_t *cynagora, const cynagora_key_t *key) {
    printf("cynagora_cache_check(%p ,(%s,%s,%s,%s))\n", cynagora, key->client, key->session, key->user,
           key->permission);
    return 0;
}

/* see cynagora.h */
int cynagora_check(cynagora_t *cynagora, const cynagora_key_t *key, int force) {
    printf("cynagora_check(%p ,(%s,%s,%s,%s), %d)\n", cynagora, key->client, key->session, key->user, key->permission,
           force);
    return 0;
}

/* see cynagora.h */
int cynagora_test(cynagora_t *cynagora, const cynagora_key_t *key, int force) {
    printf("cynagora_test(%p ,(%s,%s,%s,%s), %d)\n", cynagora, key->client, key->session, key->user, key->permission,
           force);
    return 0;
}

/* see cynagora.h */
int cynagora_async_check(cynagora_t *cynagora, const cynagora_key_t *key, int force, int simple,
                         cynagora_async_check_cb_t *callback, void *closure) {
    printf("cynagora_async_check(%p ,(%s,%s,%s,%s), %d, %d, %p, %p)\n", cynagora, key->client, key->session, key->user,
           key->permission, force, simple, callback, closure);
    return 0;
}

/******************************************************************************/
/*** PUBLIC ADMIN METHODS                                                   ***/
/******************************************************************************/

/* see cynagora.h */
int cynagora_get(cynagora_t *cynagora, const cynagora_key_t *key, cynagora_get_cb_t *callback, void *closure) {
    printf("cynagora_get(%p, %s,%s,%s,%s)\n", cynagora, key->client, key->session, key->user, key->permission);
    cynagora_key_t k = {key->client, "*", key->user, "privilege"};
    cynagora_value_t v = {"yes", 0};
    callback(closure, &k, &v);
    cynagora_key_t k2 = {key->client, "*", key->user, "privilege2"};
    cynagora_value_t v2 = {"yes", 0};
    callback(closure, &k2, &v2);
    cynagora_key_t k3 = {key->client, "*", key->user, "privilege3"};
    cynagora_value_t v3 = {"yes", 0};
    callback(closure, &k3, &v3);
    return 0;
}

/* see cynagora.h */
int cynagora_log(cynagora_t *cynagora, int on, int off) {
    printf("cynagora_log(%p, %d, %d)\n", cynagora, on, off);
    return 0;
}

/* see cynagora.h */
int cynagora_enter(cynagora_t *cynagora) {
    printf("cynagora_enter(%p)\n", cynagora);
    return 0;
}

/* see cynagora.h */
int cynagora_leave(cynagora_t *cynagora, int commit) {
    printf("cynagora_leave(%p, %d)\n", cynagora, commit);
    return 0;
}

/* see cynagora.h */
int cynagora_set(cynagora_t *cynagora, const cynagora_key_t *key, const cynagora_value_t *value) {
    printf("cynagora_set(%p ,(%s,%s,%s,%s), (%s, %ld))\n", cynagora, key->client, key->session, key->user,
           key->permission, value->value, value->expire);
    return 0;
}

/* see cynagora.h */
int cynagora_drop(cynagora_t *cynagora, const cynagora_key_t *key) {
    printf("cynagora_drop(%p ,(%s,%s,%s,%s))\n", cynagora, key->client, key->session, key->user, key->permission);
    return 0;
}
