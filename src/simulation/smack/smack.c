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

#include "smack.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int ptr = 0;

#if !defined(SMACK_FS_PATH)
#define SMACK_FS_PATH "/sys/fs/smackfs"
#endif

ssize_t smack_label_length(const char *label) {
    printf("smack_label_length(%s)\n", label);
    ssize_t len = (ssize_t)strlen(label);
    if (len >= SMACK_LABEL_LEN) {
        len = -1;
    }
    return len;
}

const char *smack_smackfs_path(void) {
    printf("smack_smackfs_path()\n");
    return SMACK_FS_PATH;
}

int smack_accesses_new(struct smack_accesses **handle) {
    printf("smack_accesses_new()\n");
    *handle = (void *)(intptr_t)(++ptr);
    return 0;
}

int smack_accesses_add(struct smack_accesses *handle, const char *subject, const char *object,
                       const char *access_type) {
    printf("smack_accesses_add(%p,%s,%s,%s)\n", handle, subject, object, access_type);
    return 0;
}

int smack_accesses_apply(struct smack_accesses *handle) {
    printf("smack_accesses_apply(%p)\n", handle);
    return 0;
}

int smack_accesses_save(struct smack_accesses *handle, int fd) {
    printf("smack_accesses_save(%p,%d)\n", handle, fd);
    return 0;
}

int smack_accesses_add_from_file(struct smack_accesses *handle, int fd) {
    printf("smack_accesses_add_from_file(%p,%d)\n", handle, fd);
    return 0;
}

int smack_accesses_clear(struct smack_accesses *handle) {
    printf("smack_accesses_clear(%p)\n", handle);
    return 0;
}

void smack_accesses_free(struct smack_accesses *handle) { printf("smack_smackfs_path(%p)\n", handle); }