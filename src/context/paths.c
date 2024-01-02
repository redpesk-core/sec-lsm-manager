/*
 * Copyright (C) 2020-2024 IoT.bzh Company
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
#include "file-utils.h"
#include "sizes.h"

static const char *type_strings[number_path_type] =
{
[type_unset] = "<unset>",
[type_default] = "default",
[type_conf] = "conf",
[type_data] = "data",
[type_exec] = "exec",
[type_http] = "http",
[type_icon] = "icon",
[type_id] = "id",
[type_lib] = "lib",
[type_plug] = "plug",
[type_public] = "public"
};

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

static path_t *path_set_search(path_set_t *path_set, const char *path)
{
    size_t idx;
    for (idx = 0 ; idx < path_set->size ; idx++) {
        path_t *p = path_set->paths[idx];
        if (0 == strcmp(path, p->path))
            return p;
    }
    return NULL;
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see paths.h */
void path_set_init(path_set_t *path_set) {
    path_set->size = 0;
    path_set->paths = NULL;
}

/* see paths.h */
void path_set_clear(path_set_t *path_set) {
    if (path_set) {
        while(path_set->size)
            free(path_set->paths[--path_set->size]);
        free(path_set->paths);
        path_set->paths = NULL;
    }
}

/* see paths.h */
int path_set_add(path_set_t *path_set, const char *path, enum path_type path_type)
{
    size_t path_len = strlen(path);

    if (path_len < 1 || path_len >= SEC_LSM_MANAGER_MAX_SIZE_PATH) {
        ERROR("invalid path size : %ld", path_len);
        return -EINVAL;
    }

    if (!path_type_is_valid(path_type)) {
        ERROR("invalid path type %d", path_type);
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

    memcpy(path_item->path, path, 1 + path_len);

    path_item->path_type = path_type;
    path_set->paths[path_set->size++] = path_item;

    return 0;
}

/* see paths.h */
bool path_type_is_valid(enum path_type path_type) {
    return path_type > type_unset && path_type < number_path_type;
}

/* see paths.h */
enum path_type path_type_get(const char *path_type_string) {
    enum path_type type = number_path_type;
    while (--type != type_unset && strcmp(path_type_string, type_strings[type]) != 0);
    if (type == type_unset)
        ERROR("Path type invalid: %s", path_type_string);
    return type;
}

/* see paths.h */
const char *path_type_name(enum path_type path_type) {
    if (path_type >= type_unset && path_type < number_path_type)
        return type_strings[path_type];
    ERROR("Path type invalid");
    return "invalid";
}

/* see paths.h */
__wur __nonnull()
bool path_set_has(path_set_t *path_set, const char *path)
{
    return NULL != path_set_search(path_set, path);
}

