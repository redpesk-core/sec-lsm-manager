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
