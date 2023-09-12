/*
 * Copyright (C) 2020-2023 IoT.bzh Company
 * Author: Arthur Guyader <arthur.guyader@iot.bzh>
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
#include "utils.h"
#include "sizes.h"

#if PERMISSIONS_DISTINCT_CASE
# define compare_permission strcmp
#else
# define compare_permission strcasecmp
#endif

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see permissions.h */
void init_permission_set(permission_set_t *permission_set) {
    permission_set->size = 0;
    permission_set->permissions = NULL;
}

/* see permissions.h */
void free_permission_set(permission_set_t *permission_set) {
    if (permission_set) {
        while (permission_set->size)
            free(permission_set->permissions[--permission_set->size]);
        free(permission_set->permissions);
        permission_set->permissions = NULL;
    }
}

/* see permissions.h */
__nonnull() __wur
int permission_set_has_permission(const permission_set_t *permission_set, const char *permission)
{
    char **parray = permission_set->permissions;
    size_t nrp = permission_set->size;
    size_t idxp = 0;
    while (idxp < nrp)
        if (compare_permission(permission, parray[idxp]) == 0)
            return 1;
        else
            idxp++;
    return 0;
}

/* see permissions.h */
int permission_set_add_permission(permission_set_t *permission_set, const char *permission) {

    size_t size;
    void *ptr, *perm;

    /* avoid duplication of permisssion */
    if (permission_set_has_permission(permission_set, permission))
        return 0;

    /* check length */
    size = strlen(permission);
    if (size < SEC_LSM_MANAGER_MIN_SIZE_PERMISSION
     || size > SEC_LSM_MANAGER_MAX_SIZE_PERMISSION) {
        ERROR("invalid permission size: %ld", size);
        return -EINVAL;
    }

    /* copy the permission */
    perm = malloc(1 + size);
    if (perm == NULL) {
        ERROR("malloc perm");
        return -ENOMEM;
    }
    memcpy(perm, permission, 1 + size);

    /*
     * ensure rooms for storing the fresh permission copy
     * allocation is made by block of 8 items
     */
    if ((permission_set->size & 7) == 0) {
        size = (permission_set->size + 8) * sizeof * permission_set->permissions;
        ptr = realloc(permission_set->permissions, size);
        if (ptr == NULL) {
            free(perm);
            ERROR("realloc ptr");
            return -ENOMEM;
        }
        permission_set->permissions = ptr;
    }
    permission_set->permissions[permission_set->size++] = perm;

    return 0;
}
