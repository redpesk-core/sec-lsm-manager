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

#include "selinux-label.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "paths.h"

char suffix_id[] = "_t";
char suffix_lib[] = "_lib_t";
char suffix_conf[] = "_conf_t";
char suffix_exec[] = "_exec_t";
char suffix_icon[] = "_icon_t";
char suffix_data[] = "_data_t";
char suffix_http[] = "_http_t";
char public_app[] = "redpesk_public_t";

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see selinux-label.h */
bool selinux_enabled() {
#ifndef SIMULATE_MAC
    if (is_selinux_enabled() == 1) {
        return true;
    }
#endif
    return false;
}

/* see selinux-label.h */
int generate_label(char **label, const char *id, const char *suffix) {
    if (!id) {
        ERROR("id undefined");
        return -EINVAL;
    } else if (!id && !suffix) {
        ERROR("id and suffix undefined");
        return -EINVAL;
    }

    size_t len_label = 0;
    size_t len_id = 0;
    size_t len_suffix = 0;

    if (id)
        len_id = strlen(id);

    if (suffix)
        len_suffix = strlen(suffix);

    len_label = len_id + len_suffix;

    *label = (char *)malloc(len_label + 1);

    if (*label == NULL) {
        ERROR("malloc label");
        return -ENOMEM;
    }
    memset(*label, 0, len_label + 1);

    if (id)
        memcpy(*label, id, len_id);
    if (suffix)
        memcpy(*label + len_id, suffix, len_suffix);

    return 0;
}

/* see selinux-label.h */
int get_path_type_info(enum path_type path_type, char **suffix, bool *is_public) {
    if (!valid_path_type(path_type)) {
        ERROR("path_type invalid");
        return -EINVAL;
    }

    *is_public = false;
    switch (path_type) {
        case type_conf:
            *suffix = suffix_conf;
            return 0;
        case type_data:
            *suffix = suffix_data;
            return 0;
        case type_exec:
            *suffix = suffix_exec;
            return 0;
        case type_http:
            *suffix = suffix_http;
            return 0;
        case type_icon:
            *suffix = suffix_icon;
            return 0;
        case type_id:
            *suffix = "_t";
            return 0;
        case type_lib:
            *suffix = suffix_lib;
            return 0;
        case type_public:
            *is_public = 1;
            return 0;
        default:
            break;
    }

    ERROR("Path type invalid");
    return -EINVAL;
}
