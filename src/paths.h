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

#ifndef SECURITY_MANAGER_PATHS_H
#define SECURITY_MANAGER_PATHS_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/cdefs.h>

/**
 * @brief several type path
 *
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
    type_none,
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
    char *path;
    enum path_type path_type;
} path_t;

/**
 * @brief Structure of paths
 * paths contains several path
 *
 */
typedef struct paths {
    path_t *paths;
    size_t size;
} paths_t;

/**
 * @brief Initialize the fields 'size' and 'paths'
 *
 * @param[in] paths paths handler
 * @return int 0 in case of success or a negative -errno value
 */
int init_paths(paths_t *paths) __wur;

/**
 * @brief Free paths that have been added
 * The pointer is not free
 *
 * @param[in] paths paths handler
 */
void free_paths(paths_t *paths);

/**
 * @brief Add a path to paths
 *
 * @param paths[in] paths handler
 * @param path[in] The path to add
 * @param path_type[in] The path_type to add
 * @return 0 in case of success or a negative -errno value
 */
int paths_add_path(paths_t *paths, const char *path, enum path_type path_type) __wur;

/**
 * @brief Check if path type is valid
 *
 * @param[in] path_type The path_type to check
 * @return int 0 if not valid or 1 if valid
 */
bool valid_path_type(enum path_type path_type) __wur;

/**
 * @brief Get the path type object associate to a string path type
 *
 * @param[in] path_type The string path type
 * @return enum path_type The enumaration associate
 */
enum path_type get_path_type(const char *path_type);

/**
 * @brief Get the path type string associate to a enum path_type
 *
 * @param[in] path_type The path type enumaration
 * @return const char* The string associate
 */
const char *get_path_type_string(enum path_type path_type);

#endif