/*
 * Copyright (C) 2020-2022 IoT.bzh Company
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

#if !defined(CYNAGORA_SELECT_ALL)
#define CYNAGORA_SELECT_ALL "#"
#endif

#if !defined(CYNAGORA_INSERT_ALL)
#define CYNAGORA_INSERT_ALL "*"
#endif

#if !defined(CYNAGORA_AUTHORIZED)
#define CYNAGORA_AUTHORIZED "yes"
#endif

#include <errno.h>
#include <string.h>

#include "log.h"

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see cynagora-interface.h */
int cynagora_set_policies(cynagora_t *cynagora, const char *label, const permission_set_t *permission_set) {
    // enter to modify policies cynagora
    int rc = cynagora_enter(cynagora);
    if (rc < 0) {
        ERROR("cynagora_enter : %d %s", -rc, strerror(-rc));
        return rc;
    }

    size_t i = 0;
    cynagora_key_t k = {
        .client = label,
        .session = CYNAGORA_INSERT_ALL,
        .user = CYNAGORA_INSERT_ALL,
        .permission = NULL};
    cynagora_value_t v = {
        .value = CYNAGORA_AUTHORIZED,
        .expire = 0 /* infinite */};

    while (i < permission_set->size) {
        k.permission = permission_set->permissions[i];
        rc = cynagora_set(cynagora, &k, &v);
        if (rc < 0) {
            ERROR("cynagora_set : %d %s", -rc, strerror(-rc));
            break;
        }
        i++;
    }

    // leave and apply modification
    int rc2 = cynagora_leave(cynagora, rc == 0);
    if (rc2 < 0)
        ERROR("cynagora_leave : %d %s", -rc, strerror(-rc));
    if (rc == 0)
        rc = rc2;

    return rc;
}

/* see cynagora-interface.h */
int cynagora_drop_policies(cynagora_t *cynagora, const char *label) {
    // enter to modify policies cynagora
    int rc = cynagora_enter(cynagora);
    if (rc < 0) {
        ERROR("cynagora_enter : %d %s", -rc, strerror(-rc));
        return rc;
    }

    cynagora_key_t key = {
        .client = label,
        .session = CYNAGORA_SELECT_ALL,
        .user = CYNAGORA_SELECT_ALL,
        .permission = CYNAGORA_SELECT_ALL};
    rc = cynagora_drop(cynagora, &key);
    if (rc < 0)
        ERROR("cynagora_drop : %d %s", -rc, strerror(-rc));

    // leave and apply modification
    int rc2 = cynagora_leave(cynagora, rc == 0);
    if (rc2 < 0)
        ERROR("cynagora_leave : %d %s", -rc2, strerror(-rc2));
    if (rc == 0)
        rc = rc2;

    return rc;
}
