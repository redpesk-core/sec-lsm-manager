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

#ifndef SEC_LSM_MANAGER_PATHS_H
#define SEC_LSM_MANAGER_PATHS_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/cdefs.h>

#include "limits.h"

/**
 * @brief several type path
 *
 * type_unset  : not a type, default init
 * type_default: reset to some default, not linked to id
 * type_conf   : config files ('conf')
 * type_data   : data files ('data')
 * type_exec   : executable files ('exec')
 * type_http   : http files ('http')
 * type_icon   : icon file ('icon)
 * type_id     : basename app directory ('id')
 * type_lib    : libraries files ('lib')
 * type_public : public files ('public')
 *
 */
enum path_type {
    type_unset,
    type_default,
    type_conf,
    type_data,
    type_exec,
    type_http,
    type_icon,
    type_id,
    type_lib,
    type_public,
    number_path_type
};

/**
 * @brief Structure of path
 * path contain a path and is type
 *
 */
typedef struct path {
    enum path_type path_type;
    char path[];
} path_t;

/**
 * @brief Structure of path_set
 * path_set contains several path
 *
 */
typedef struct path_set {
    path_t **paths;
    size_t size;
} path_set_t;

/**
 * @brief Initialize the fields 'size' and 'paths'
 *
 * @param[in] path_set path_set handler
 */
extern void init_path_set(path_set_t *path_set) __nonnull();

/**
 * @brief Free paths that have been added
 * The pointer is not free
 *
 * @param[in] path_set path_set handler
 */
extern void free_path_set(path_set_t *path_set) __nonnull();

/**
 * @brief Add a path to paths
 *
 * @param path_set[in] path_set handler
 * @param path[in] The path to add
 * @param path_type[in] The path_type to add
 * @return 0 in case of success or a negative -errno value
 */
extern int path_set_add_path(path_set_t *path_set, const char *path, enum path_type path_type) __wur __nonnull();

/**
 * @brief Check if path_type is valid
 *
 * @param[in] path_type The path_type to check
 * @return int 0 if not valid or 1 if valid
 */
extern bool valid_path_type(enum path_type path_type) __wur;

/**
 * @brief Get the path type object associate to a string path type
 *
 * @param[in] path_type_string The string path type
 * @return enum path_type The enumeration associate
 */
extern enum path_type get_path_type(const char *path_type_string) __wur __nonnull();

/**
 * @brief Get the path type string associate to a enum path_type
 *
 * @param[in] path_type The path type enumeration
 * @return const char* The string associate
 */
extern const char *get_path_type_string(enum path_type path_type) __wur;

#endif
