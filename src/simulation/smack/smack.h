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

#ifndef SEC_LSM_MANAGER_SIMULATION_SMACK_H
#define SEC_LSM_MANAGER_SIMULATION_SMACK_H

#define SMACK_LABEL_LEN 255

#include <sys/types.h>
#include <unistd.h>

struct smack_accesses;

ssize_t smack_label_length(const char *label);

const char *smack_smackfs_path(void);

int smack_accesses_new(struct smack_accesses **handle);

int smack_accesses_add(struct smack_accesses *handle, const char *subject, const char *object, const char *access_type);

int smack_accesses_apply(struct smack_accesses *handle);

int smack_accesses_save(struct smack_accesses *handle, int fd);

int smack_accesses_add_from_file(struct smack_accesses *handle, int fd);

int smack_accesses_clear(struct smack_accesses *handle);

void smack_accesses_free(struct smack_accesses *handle);

#endif
