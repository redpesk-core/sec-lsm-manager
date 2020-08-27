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

#include "paths.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Free path and set path type to type_none
 * The pointer is not free
 *
 * @param[in] path path handler
 */
static void free_path(path_t *path) {
    if (path) {
        free(path->path);
        path->path = NULL;
        path->path_type = type_none;
    }
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see paths.h */
int init_paths(paths_t *paths) {
    if (!paths) {
        ERROR("policies undefined");
        return -EINVAL;
    }
    paths->size = 0;
    paths->paths = NULL;
    return 0;
}

/* see paths.h */
void free_paths(paths_t *paths) {
    if (paths) {
        for (size_t i = 0; i < paths->size; i++) {
            free_path(paths->paths + i);
        }
        paths->size = 0;
        free(paths->paths);
        paths->paths = NULL;
    }
}

/* see paths.h */
int paths_add_path(paths_t *paths, const char *path, enum path_type path_type) {
    if (!paths) {
        ERROR("paths undefined");
        return -EINVAL;
    } else if (!path) {
        ERROR("path undefined");
        return -EINVAL;
    } else if (!valid_path_type(path_type)) {
        ERROR("invalid path type");
        return -EINVAL;
    }

    if (paths->size == 0) {
        paths->paths = (path_t *)malloc(sizeof(path_t));
        if (paths->paths == NULL) {
            ERROR("malloc path_t");
            return -ENOMEM;
        }
    } else {
        path_t *paths_tmp = (path_t *)realloc(paths->paths, sizeof(path_t) * (paths->size + 1));
        if (paths_tmp == NULL) {
            ERROR("realloc paths_t");
            free_paths(paths);
            return -ENOMEM;
        }
        paths->paths = paths_tmp;
    }

    paths->paths[paths->size].path = strdup(path);
    if (paths->paths[paths->size].path == NULL) {
        ERROR("strdup path");
        free_paths(paths);
        return -ENOMEM;
    }

    paths->paths[paths->size].path_type = path_type;
    paths->size++;

    return 0;
}

/* see paths.h */
bool valid_path_type(enum path_type path_type) {
    if (path_type > type_none && path_type < number_path_type)
        return true;
    else
        return false;
}

/* see paths.h */
enum path_type get_path_type(const char *path_type) {
    if (!path_type) {
        ERROR("path_type undefined");
        return type_none;
    }

    switch (path_type[0]) {
        case 'c':
            if (!strcmp(path_type, "conf")) {
                return type_conf;
            }
            break;
        case 'd':
            if (!strcmp(path_type, "data")) {
                return type_data;
            }
            break;
        case 'e':
            if (!strcmp(path_type, "exec")) {
                return type_exec;
            }
            break;
        case 'h':
            if (!strcmp(path_type, "http")) {
                return type_http;
            }
            break;
        case 'i':
            if (!strcmp(path_type, "icon")) {
                return type_icon;
            }
            if (!strcmp(path_type, "id")) {
                return type_id;
            }
            break;
        case 'l':
            if (!strcmp(path_type, "lib")) {
                return type_lib;
            }
            break;
        case 'p':
            if (!strcmp(path_type, "public")) {
                return type_public;
            }
            break;
        default:
            break;
    }
    ERROR("Path type invalid: %s", path_type);
    return type_none;
}

/* see paths.h */
const char *get_path_type_string(enum path_type path_type) {
    switch (path_type) {
        case type_none:
            return "none";
        case type_conf:
            return "conf";
        case type_data:
            return "data";
        case type_exec:
            return "exec";
        case type_http:
            return "http";
        case type_icon:
            return "icon";
        case type_id:
            return "id";
        case type_lib:
            return "lib";
        case type_public:
            return "public";
        default:
            break;
    }
    ERROR("Path type invalid");
    return "invalid";
}
