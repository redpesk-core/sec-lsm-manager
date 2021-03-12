/*
 * Copyright (C) 2020-2021 IoT.bzh Company
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
__nonnull() static void free_path(path_t *path) {
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
int init_path_set(path_set_t *path_set) {
    path_set->size = 0;
    path_set->paths = NULL;
    return 0;
}

/* see paths.h */
void free_path_set(path_set_t *path_set) {
    if (path_set) {
        for (size_t i = 0; i < path_set->size; i++) {
            free_path(path_set->paths + i);
        }
        path_set->size = 0;
        free(path_set->paths);
        path_set->paths = NULL;
    }
}

/* see paths.h */
int path_set_add_path(path_set_t *path_set, const char *path, enum path_type path_type) {
    if (!valid_path_type(path_type)) {
        ERROR("invalid path type");
        return -EINVAL;
    }

    size_t path_len = strlen(path);

    if (path_len < 2) {
        ERROR("invalid path");
        return -EINVAL;
    }

    path_t *path_set_tmp = (path_t *)realloc(path_set->paths, sizeof(path_t) * (path_set->size + 1));
    if (path_set_tmp == NULL) {
        ERROR("realloc path_set_t");
        return -ENOMEM;
    }
    path_set->paths = path_set_tmp;

    path_set->paths[path_set->size].path = strdup(path);
    if (path_set->paths[path_set->size].path == NULL) {
        ERROR("strdup path");
        return -ENOMEM;
    }

    char *new_path = path_set->paths[path_set->size].path;

    if (new_path[path_len - 1] == '/') {
        new_path[path_len - 1] = '\0';
    }

    path_set->paths[path_set->size].path_type = path_type;
    path_set->size++;

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
enum path_type get_path_type(const char *path_type_string) {
    switch (path_type_string[0]) {
        case 'c':
            if (!strcmp(path_type_string, "conf")) {
                return type_conf;
            }
            break;
        case 'd':
            if (!strcmp(path_type_string, "data")) {
                return type_data;
            }
            break;
        case 'e':
            if (!strcmp(path_type_string, "exec")) {
                return type_exec;
            }
            break;
        case 'h':
            if (!strcmp(path_type_string, "http")) {
                return type_http;
            }
            break;
        case 'i':
            if (!strcmp(path_type_string, "icon")) {
                return type_icon;
            }
            if (!strcmp(path_type_string, "id")) {
                return type_id;
            }
            break;
        case 'l':
            if (!strcmp(path_type_string, "lib")) {
                return type_lib;
            }
            break;
        case 'p':
            if (!strcmp(path_type_string, "public")) {
                return type_public;
            }
            break;
        default:
            break;
    }
    ERROR("Path type invalid: %s", path_type_string);
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
