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

#include "paths.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "utils.h"

static const char *type_strings[number_path_type] =
{
[type_none] =	"none",
[type_conf] =	"conf",
[type_data] =	"data",
[type_exec] =	"exec",
[type_http] =	"http",
[type_icon] =	"icon",
[type_id] =	"id",
[type_lib] =	"lib",
[type_public] =	"public"
};

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see paths.h */
void init_path_set(path_set_t *path_set) {
    path_set->size = 0;
    path_set->paths = NULL;
}

/* see paths.h */
void free_path_set(path_set_t *path_set) {
    if (path_set) {
        while(path_set->size)
            free(path_set->paths[--path_set->size]);
        free(path_set->paths);
        path_set->paths = NULL;
    }
}

/* see paths.h */
int path_set_add_path(path_set_t *path_set, const char *path, enum path_type path_type) {
    if (!valid_path_type(path_type)) {
        ERROR("invalid path type %d", path_type);
        return -EINVAL;
    }

    size_t path_len = strlen(path);

    if (path_len < 2 || path_len >= SEC_LSM_MANAGER_MAX_SIZE_PATH) {
        ERROR("invalid path size : %ld", path_len);
        return -EINVAL;
    }

    path_t **path_set_tmp = (path_t **)realloc(path_set->paths, sizeof(path_t*) * (path_set->size + 1));
    if (path_set_tmp == NULL) {
        ERROR("realloc path_set_t");
        return -ENOMEM;
    }
    path_set->paths = path_set_tmp;

    path_t *path_item = (path_t *)malloc(sizeof(path_t) + path_len + 1);
    if (path_item == NULL) {
        ERROR("malloc path_item");
        return -ENOMEM;
    }
    
    secure_strncpy(path_item->path, path, path_len + 1);
    if (path_item->path[path_len - 1] == '/') {
        path_item->path[path_len - 1] = '\0';
    }

    path_item->path_type = path_type;
    path_set->paths[path_set->size++] = path_item;

    return 0;
}

/* see paths.h */
bool valid_path_type(enum path_type path_type) {
    return path_type > type_none && path_type < number_path_type;
}

/* see paths.h */
enum path_type get_path_type(const char *path_type_string) {
    enum path_type type = number_path_type;
    while (--type != type_none && strcmp(path_type_string, type_strings[type]) != 0);
    if (type == type_none)
        ERROR("Path type invalid: %s", path_type_string);
    return type;
}

/* see paths.h */
const char *get_path_type_string(enum path_type path_type) {
    if (path_type >= type_none && path_type < number_path_type)
        return type_strings[path_type];
    ERROR("Path type invalid");
    return "invalid";
}
