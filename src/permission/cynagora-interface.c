/*
 * Copyright (C) 2020-2023 IoT.bzh Company
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

#include "cynagora-interface.h"

#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "log.h"

#if SIMULATE_CYNAGORA
#include "simulation/cynagora/cynagora.h"
#include "simulation/cynagora/cynagora.c"
#else
#include <cynagora.h>
#endif

#define CYNAGORA_SELECT_ALL "#"
#define CYNAGORA_INSERT_ALL "*"
#define CYNAGORA_AUTHORIZED "yes"


/** cynagora client used by all client */
cynagora_t *cynagora_handler = NULL;

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

static int get(cynagora_t **handler)
{
    if (cynagora_handler == NULL) {
        int rc = cynagora_create(&cynagora_handler, cynagora_Admin, 1, 0);
        if (rc < 0) {
            ERROR("cynagora_create: %d %s", -rc, strerror(-rc));
            cynagora_handler = NULL;
            return rc;
        }
    }
    *handler = cynagora_handler;
    return 0;
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see cynagora-interface.h */
__wur __nonnull((1))
int cynagora_set_policies(const char *label, const permission_set_t *permission_set, int drop_before) {
    size_t i;
    int rc2, rc;
    cynagora_key_t key;
    cynagora_value_t val;
    cynagora_t *cynagora;

    /* get cynagora common handler */
    rc = get(&cynagora);
    if (rc < 0)
        return rc;

    /* enter to modify policies cynagora */
    rc = cynagora_enter(cynagora);
    if (rc < 0) {
        ERROR("cynagora_enter : %d %s", -rc, strerror(-rc));
        return rc;
    }

    /* init client */
    key.client = label;

    /* drop previous rules for the application of label */
    if (drop_before) {
        key.session = CYNAGORA_SELECT_ALL;
        key.user = CYNAGORA_SELECT_ALL;
        key.permission = CYNAGORA_SELECT_ALL;
        rc = cynagora_drop(cynagora, &key);
        if (rc < 0)
            ERROR("cynagora_drop : %d %s", -rc, strerror(-rc));
    }

    /* add permissions */
    if (permission_set != NULL) {
	/* init key and val */
        key.session = CYNAGORA_INSERT_ALL;
        key.user = CYNAGORA_INSERT_ALL;
        val.value = CYNAGORA_AUTHORIZED;
        val.expire = 0; /* infinite */

        for (i = 0 ; rc >= 0 && i < permission_set->size ; i++) {
            key.permission = permission_set->permissions[i];
            rc = cynagora_set(cynagora, &key, &val);
            if (rc < 0) {
                ERROR("cynagora_set : %d %s", -rc, strerror(-rc));
            }
        }
    }

    /* leave and apply modification */
    rc2 = cynagora_leave(cynagora, rc == 0);
    if (rc2 < 0) {
        ERROR("cynagora_leave : %d %s", -rc2, strerror(-rc2));
        if (rc == 0)
            rc = rc2;
    }

    return rc;
}

/* see cynagora-interface.h */
int cynagora_drop_policies(const char *label) {
    return cynagora_set_policies(label, NULL, 1);
}

/* see cynagora-interface.h */
__nonnull() __wur
int cynagora_check_permission(const char *label, const char *permission)
{
    cynagora_key_t key = {
        .client = label,
        .session = "-",
        .user = "-",
        .permission = permission
    };
    cynagora_t *cynagora;

    /* get cynagora common handler */
    int rc = get(&cynagora);
    if (rc < 0)
        return rc;

    return cynagora_check(cynagora, &key, 0);
}

static void list(void *closure, const cynagora_key_t *key, const cynagora_value_t *value) {
    (void)value;
    int rc = permission_set_add(closure, key->permission);
    (void)rc;
}

__nonnull() __wur
int cynagora_get_policies(const char *label, permission_set_t *permission_set) {
    cynagora_key_t k = {
        .client = label,
        .session = CYNAGORA_SELECT_ALL,
        .user = CYNAGORA_SELECT_ALL,
        .permission = CYNAGORA_SELECT_ALL
    };
    int rc;
    cynagora_t *cynagora;

    /* init */
    permission_set_init(permission_set);

    /* get cynagora common handler */
    rc = get(&cynagora);
    if (rc < 0)
        return rc;

    rc = cynagora_get(cynagora, &k, list, permission_set);
    if (rc < 0)
        return rc;

    return 0;
}

