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

#ifndef SEC_LSM_MANAGER_POLICIES_H
#define SEC_LSM_MANAGER_POLICIES_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "limits.h"

/**
 * @brief Structure of permission_set
 * permission_set contains several permission
 *
 */
typedef struct permission_set {
    char **permissions;
    size_t size;
} permission_set_t;

/**
 * @brief Initialize the fields 'size' and 'permissions'
 *
 * @param[in] permission_set The permission_set handler
 */
extern void init_permission_set(permission_set_t *permission_set) __nonnull();

/**
 * @brief[in] Free permission_set that have been added
 * The pointer is not free
 * @param policies The permission_set handler
 */
extern void free_permission_set(permission_set_t *permission_set) __nonnull();

/**
 * @brief Add a permission to permission_set struct
 *
 * @param[in] permission_set The permission_set handler
 * @param[in] permission The permission to add
 * @return 0 in case of success or a negative -errno value
 */
extern int permission_set_add_permission(permission_set_t *permission_set, const char *permission) __wur __nonnull();

#endif