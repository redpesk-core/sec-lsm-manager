/*
 * Copyright (C) 2020 IoT.bzh Company
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
