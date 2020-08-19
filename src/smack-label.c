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

#include "smack-label.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "paths.h"

char prefix_app[] = "App:";
char suffix_lib[] = ":Lib";
char suffix_conf[] = ":Conf";
char suffix_exec[] = ":Exec";
char suffix_icon[] = ":Icon";
char suffix_data[] = ":Data";
char suffix_http[] = ":Http";
char user_home[] = "User:Home";
char public_app[] = "_";

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see smack-label.h */
int smack_enabled() {
    int rc = 0;
#ifndef SIMULATE_MAC
    if (smack_smackfs_path() == NULL) {
        rc = 0;
    }
    rc = 1;
#endif

    return rc;
}

/* see smack-label.h */
int generate_label(char **label, const char *id, const char *prefix, const char *suffix) {
    if (!id) {
        ERROR("id undefined");
        return -EINVAL;
    } else if (!id && !prefix && !suffix) {
        ERROR("id, prefix and suffix undefined");
        return -EINVAL;
    }

    size_t len_label = 0;
    size_t len_prefix = 0;
    size_t len_id = 0;
    size_t len_suffix = 0;

    if (prefix)
        len_prefix = strlen(prefix);

    if (id)
        len_id = strlen(id);

    if (suffix)
        len_suffix = strlen(suffix);

    len_label = len_prefix + len_id + len_suffix;
    *label = (char *)malloc(len_label + 1);

    if (*label == NULL) {
        ERROR("malloc label");
        return -ENOMEM;
    }

    memset(*label, 0, len_label + 1);

    if (prefix)
        memcpy(*label, prefix, len_prefix);

    if (id)
        memcpy(*label + len_prefix, id, len_id);

    if (suffix)
        memcpy(*label + len_prefix + len_id, suffix, len_suffix);

    int rc = (int)smack_label_length(*label);
    if (rc <= 0) {
        ERROR("Invalid SMACK label %s", *label);
        free(*label);
        return rc;
    }

    return 0;
}

/* see smack-label.h */
int get_path_type_info(enum path_type path_type, char **suffix, bool *is_executable, bool *is_transmute,
                       bool *is_public) {
    if (!valid_path_type(path_type)) {
        ERROR("path_type invalid");
        return -EINVAL;
    }

    *is_executable = false;
    *is_transmute = false;
    *is_public = false;

    switch (path_type) {
        case type_conf:
            *suffix = suffix_conf;
            return 0;
        case type_data:
            *suffix = suffix_data;
            *is_transmute = true;
            return 0;
        case type_exec:
            *suffix = suffix_exec;
            *is_executable = true;
            return 0;
        case type_http:
            *suffix = suffix_http;
            *is_transmute = true;
            return 0;
        case type_icon:
            *suffix = suffix_icon;
            return 0;
        case type_id:
            *suffix = "";
            return 0;
        case type_lib:
            *suffix = suffix_lib;
            *is_executable = true;
            return 0;
        case type_public:
            *is_public = true;
            *suffix = NULL;
            return 0;
        default:
            break;
    }

    ERROR("Path type invalid");
    return -EINVAL;
}
