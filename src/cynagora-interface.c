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

#include "cynagora-interface.h"

#include <errno.h>
#include <string.h>

#include "log.h"

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see cynagora-interface.h */
int cynagora_drop_policies(cynagora_t *cynagora, const char *client) {
    if (!cynagora) {
        ERROR("cynagora undefined");
        return -EINVAL;
    } else if (!client) {
        ERROR("client undefined");
        return -EINVAL;
    }

    // enter to modify policies cynagora
    int rc = cynagora_enter(cynagora);
    if (rc < 0) {
        ERROR("cynagora_enter : %d %s", -rc, strerror(-rc));
        return rc;
    }

    cynagora_key_t key = {client, SELECT_ALL, SELECT_ALL, SELECT_ALL};
    rc = cynagora_drop(cynagora, &key);
    if (rc < 0) {
        ERROR("cynagora_drop : %d %s", -rc, strerror(-rc));
    }

    // leave and apply modification
    int rc2 = cynagora_leave(cynagora, rc == 0);
    if (rc == 0)
        rc = rc2;

    return rc;
}

/* see cynagora-interface.h */
int cynagora_set_policies(cynagora_t *cynagora, const policies_t *policies) {
    if (!cynagora) {
        ERROR("cynagora undefined");
        return -EINVAL;
    } else if (!policies) {
        ERROR("policies undefined");
        return -EINVAL;
    }

    // enter to modify policies cynagora
    int rc = cynagora_enter(cynagora);
    if (rc < 0) {
        ERROR("cynagora_enter : %d %s", -rc, strerror(-rc));
        return rc;
    }

    size_t i = 0;
    while (i < policies->size) {
        rc = cynagora_set(cynagora, &(policies->policies[i].k), &(policies->policies[i].v));
        if (rc < 0) {
            ERROR("cynagora_set : %s %s", -rc, strerror(-rc));
            break;
        }
        i++;
    }

    // leave and apply modification
    int rc2 = cynagora_leave(cynagora, rc == 0);
    if (rc == 0)
        rc = rc2;

    return rc;
}
