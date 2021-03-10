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

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Generate selinux label
 *
 * @param[out] label Alloc and set the label
 * @param[in] id The id of the application
 * @param[in] suffix The suffix to add at the end of the label
 * @return 0 in case of success or a negative -errno value
 */
__nonnull((2)) __wur static int generate_label(char **label, const char *id, const char *suffix) {
    size_t len_label = 0;
    size_t len_id = 0;
    size_t len_suffix = 0;

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
    memcpy(*label, id, len_id);

    if (suffix)
        memcpy(*label + len_id, suffix, len_suffix);

    return 0;
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see selinux-label.h */
bool selinux_enabled() {
    if (is_selinux_enabled() == 1) {
        return true;
    }
    return false;
}

/* see selinux-label.h */
void free_path_type_definitions(path_type_definitions_t path_type_definitions[number_path_type]) {
    if (path_type_definitions) {
        for (int i = 1; i < number_path_type; i++) {
            free(path_type_definitions[i].label);
        }
    }
}

/* see selinux-label.h */
int init_path_type_definitions(path_type_definitions_t path_type_definitions[number_path_type], const char *id) {
    int rc = 0;

    // conf
    if (generate_label(&path_type_definitions[type_conf].label, id, suffix_conf) < 0)
        goto error;

    // data
    if (generate_label(&path_type_definitions[type_data].label, id, suffix_data) < 0)
        goto error;

    // exec
    if (generate_label(&path_type_definitions[type_exec].label, id, suffix_exec) < 0)
        goto error;

    // http
    if (generate_label(&path_type_definitions[type_http].label, id, suffix_http) < 0)
        goto error;

    // icon
    if (generate_label(&path_type_definitions[type_icon].label, id, suffix_icon) < 0)
        goto error;

    // id
    if (generate_label(&path_type_definitions[type_id].label, id, suffix_id) < 0)
        goto error;

    // lib
    if (generate_label(&path_type_definitions[type_lib].label, id, suffix_lib) < 0)
        goto error;

    // public
    if (generate_label(&path_type_definitions[type_public].label, public_app, NULL) < 0)
        goto error;

    goto ret;

error:
    free_path_type_definitions(path_type_definitions);
    ERROR("allocation label");
    rc = -ENOMEM;
ret:
    return rc;
}
