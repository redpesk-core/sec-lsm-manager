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

#include "smack-label.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "paths.h"

char prefix_app[] = "App:";
char suffix_lib[] = ":Lib";
char suffix_conf[] = ":Conf";
char suffix_exec[] = ":Exec";  // see label_exec before remove this line
char suffix_icon[] = ":Icon";
char suffix_data[] = ":Data";
char suffix_http[] = ":Http";
char user_home[] = "User:Home";
char public_app[] = "_";

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Generate smack label
 *
 * @param[out] label allocate and set the label
 * @param[in] id The id of the application
 * @param[in] prefix The prefix to add at the begin of the label
 * @param[in] suffix The suffix to add at the end of the label
 * @return 0 in case of success or a negative -errno value
 */
__nonnull((2)) __wur static int generate_label(char **label, const char *id, const char *prefix, const char *suffix) {
    size_t len_label = 0;
    size_t len_prefix = 0;
    size_t len_id = 0;
    size_t len_suffix = 0;

    if (prefix)
        len_prefix = strlen(prefix);

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

    memcpy(*label + len_prefix, id, len_id);

    if (suffix)
        memcpy(*label + len_prefix + len_id, suffix, len_suffix);

    int rc = (int)smack_label_length(*label);
    if (rc <= 0) {
        ERROR("Invalid SMACK label %s", *label);
        free(*label);
        *label = NULL;
        return rc;
    }

    return 0;
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see smack-label.h */
bool smack_enabled() {
    if (smack_smackfs_path() == NULL) {
        return false;
    }
    return true;
}

/* see smack-label.h */
void free_path_type_definitions(path_type_definitions_t path_type_definitions[number_path_type]) {
    if (path_type_definitions) {
        for (int i = 1; i < number_path_type; i++) {
            free(path_type_definitions[i].label);
        }
    }
}

/* see smack-label.h */
int init_path_type_definitions(path_type_definitions_t path_type_definitions[number_path_type], const char *id) {
    int rc = 0;
    // conf
    if (generate_label(&path_type_definitions[type_conf].label, id, prefix_app, suffix_conf) < 0)
        goto error;

    // data
    if (generate_label(&path_type_definitions[type_data].label, id, prefix_app, suffix_data) < 0)
        goto error;

    // exec
    if (generate_label(&path_type_definitions[type_exec].label, id, prefix_app, suffix_exec) < 0)
        goto error;

    // http
    if (generate_label(&path_type_definitions[type_http].label, id, prefix_app, suffix_http) < 0)
        goto error;

    // icon
    if (generate_label(&path_type_definitions[type_icon].label, id, prefix_app, suffix_icon) < 0)
        goto error;

    // id
    if (generate_label(&path_type_definitions[type_id].label, id, prefix_app, NULL) < 0)
        goto error;

    // lib
    if (generate_label(&path_type_definitions[type_lib].label, id, prefix_app, suffix_lib) < 0)
        goto error;

    // public
    if (generate_label(&path_type_definitions[type_public].label, public_app, NULL, NULL) < 0)
        goto error;

    // executable
    path_type_definitions[type_exec].is_executable = true;

    // transmute
    path_type_definitions[type_data].is_transmute = true;
    path_type_definitions[type_http].is_transmute = true;
    path_type_definitions[type_id].is_transmute = true;
    path_type_definitions[type_lib].is_transmute = true;
    path_type_definitions[type_public].is_transmute = true;

    goto ret;
error:
    free_path_type_definitions(path_type_definitions);
    ERROR("allocation label");
    rc = -ENOMEM;
ret:
    return rc;
}
