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

#ifndef SECURITY_MANAGER_SELINUX_LABEL_H
#define SECURITY_MANAGER_SELINUX_LABEL_H

#include <selinux/selinux.h>
#include <semanage/semanage.h>
#include <stdbool.h>

#include "paths.h"

extern char suffix_id[];
extern char suffix_lib[];
extern char suffix_conf[];
extern char suffix_exec[];
extern char suffix_icon[];
extern char suffix_data[];
extern char suffix_http[];
extern char public_app[];

/**
 * @brief Check if selinux is enabled
 *
 * @return true if enabled
 * @return false if not
 */
bool selinux_enabled() __wur;

/**
 * @brief Generate selinux label
 *
 * @param[out] label Alloc and set the label
 * @param[in] id The id of the application
 * @param[in] suffix The suffix to add at the end of the label
 * @return 0 in case of success or a negative -errno value
 */
int generate_label(char **label, const char *id, const char *suffix) __wur;

/**
 * @brief Get path type info in a selinux context
 *
 * @param[in] path_type The path type information
 * @param[out] suffix The suffix to choose
 * @param[out] is_public true if path is public
 * @return 0 in case of success or a negative -errno value
 */
int get_path_type_info(enum path_type path_type, char **suffix, bool *is_public) __wur;

#endif
