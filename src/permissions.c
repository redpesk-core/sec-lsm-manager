/*
 * Copyright (C) 2020 "IoT.bzh"
 * Author Arthur Guyader <arthur.guyader@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "permissions.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see permissions.h */
int init_permission_set(permission_set_t *permission_set) {
    permission_set->size = 0;
    permission_set->permissions = NULL;
    return 0;
}

/* see permissions.h */
void free_permission_set(permission_set_t *permission_set) {
    if (permission_set) {
        permission_set->size = 0;
        free(permission_set->permissions);
        permission_set->permissions = NULL;
    }
}

/* see permissions.h */
int permission_set_add_permission(permission_set_t *permission_set, const char *permission) {
    char **permissions_tmp = (char **)realloc(permission_set->permissions, (permission_set->size + 1) * sizeof(char *));
    if (permissions_tmp == NULL) {
        ERROR("realloc char**");
        return -ENOMEM;
    }
    permission_set->permissions = permissions_tmp;

    permission_set->permissions[permission_set->size] = strdup(permission);

    permission_set->size++;

    return 0;
}
