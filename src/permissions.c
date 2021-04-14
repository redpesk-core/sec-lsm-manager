/*
 * Copyright (C) 2020-2021 IoT.bzh Company
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
        permission_set->size = 0;
        free(permission_set->permissions);
        permission_set->permissions = NULL;
    }
}

/* see permissions.h */
int permission_set_add_permission(permission_set_t *permission_set, const char *permission) {
    size_t permission_len = strlen(permission);
    if (permission_len < 2 || permission_len >= SEC_LSM_MANAGER_MAX_SIZE_PERMISSION) {
        ERROR("invalid permission size : %ld", permission_len);
        return -EINVAL;
    }

    size_t size = (permission_set->size + 1) * (sizeof(char) * SEC_LSM_MANAGER_MAX_SIZE_PERMISSION);

    char(*permissions_tmp)[SEC_LSM_MANAGER_MAX_SIZE_PERMISSION] = realloc(permission_set->permissions, size);
    if (permissions_tmp == NULL) {
        ERROR("realloc char[SEC_LSM_MANAGER_MAX_SIZE_PERMISSION]");
        return -ENOMEM;
    }
    permission_set->permissions = permissions_tmp;

    secure_strncpy(permission_set->permissions[permission_set->size], permission, SEC_LSM_MANAGER_MAX_SIZE_PERMISSION);
    permission_set->size++;

    return 0;
}
