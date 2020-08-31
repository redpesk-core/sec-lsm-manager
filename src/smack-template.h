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

#ifndef SECURITY_MANAGER_SMACK_TEMPLATE_H
#define SECURITY_MANAGER_SMACK_TEMPLATE_H

#include <sys/cdefs.h>

#include "secure-app.h"

/**
 * @brief Get the selinux template file
 *
 * @param[in] value some value or NULL for getting default
 * @return the selinux template file specification
 */
const char *get_smack_template_file(const char *value) __wur;

/**
 * @brief Get the smack rules directory
 *
 * @param[in] value some value or NULL for getting default
 * @return the smack rules directory specification
 */
const char *get_smack_rules_dir(const char *value) __wur;

/**
 * @brief Create smack rules
 *
 * @param[in] secure_app secure app handler to install
 * @param[in] smack_template_file some value or NULL for getting default
 * @param[in] smack_rules_dir some value or NULL for getting default
 * @return 0 in case of success or a negative -errno value
 */
int create_smack_rules(const secure_app_t *secure_app, path_type_definitions_t path_type_definitions[number_path_type],
                       const char *smack_template_file, const char *smack_rules_dir) __wur __nonnull((1));

/**
 * @brief Remove smack rules
 *
 * @param[in] secure_app secure app handler
 * @param[in] smack_rules_dir some value or NULL for getting default
 * @return 0 in case of success or a negative -errno value
 */
int remove_smack_rules(const secure_app_t *secure_app, const char *smack_rules_dir) __wur __nonnull((1));

#endif
