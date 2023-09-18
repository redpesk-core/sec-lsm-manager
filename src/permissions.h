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
 * @return
 *    * 0              success
 *    * -EINVAL        bad type or bad path
 *    * -ENOMEM        out of memory
 */
extern int permission_set_add_permission(permission_set_t *permission_set, const char *permission) __wur __nonnull();

/**
 * @brief check if the permission set has the permission
 *
 * @param[in] secure_app the application to be uninstalled
 * @param[in] permission the permission to check
 * @return 1 when permission is granted or, otherwise, 0
 */
__nonnull() __wur
extern int permission_set_has_permission(const permission_set_t *permission_set, const char *permission);

#endif
