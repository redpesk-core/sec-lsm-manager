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

#ifndef SECURITY_MANAGER_SELINUX_TEMPLATE_H
#define SECURITY_MANAGER_SELINUX_TEMPLATE_H

#include "secure-app.h"

/**
 * @brief Get the selinux te template file
 *
 * @param[in] value some value or NULL for getting default
 * @return the selinux te template file specification
 */
const char *get_selinux_te_template_file(const char *value) __wur;

/**
 * @brief Get the selinux if template file
 *
 * @param[in] value some value or NULL for getting default
 * @return the selinux if template file specification
 */
const char *get_selinux_if_template_file(const char *value) __wur;

/**
 * @brief Get the selinux rules directory
 *
 * @param[in] value some value or NULL for getting default
 * @return the selinux rules directory specification
 */
const char *get_selinux_rules_dir(const char *value) __wur;

/**
 * @brief Create selinux rules
 *
 * @param[in] secure_app secure app handler to install
 * @param[in] selinux_te_template_file some value or NULL for getting default
 * @param[in] selinux_if_template_file some value or NULL for getting default
 * @param[in] selinux_rules_dir some value or NULL for getting default
 * @return 0 in case of success or a negative -errno value
 */
int create_selinux_rules(const secure_app_t *secure_app, const char *selinux_te_template_file,
                         const char *selinux_if_template_file, const char *selinux_rules_dir) __wur;

/**
 * @brief Check if the files of an application exists in the selinux rules directory
 *
 * @param[in] secure_app secure app handler
 * @param[in] selinux_rules_dir some value or NULL for getting default
 * @return 1 if exists, 0 if not or a negative -errno value
 */
int check_module_files_exist(const secure_app_t *secure_app, const char *selinux_rules_dir) __wur;

/**
 * @brief Check if module is in the policy
 *
 * @param[in] secure_app secure app handler
 * @return 1 if exists, 0 if not or a negative -errno value
 */
int check_module_in_policy(const secure_app_t *secure_app) __wur;

/**
 * @brief Remove selinux rules (in the selinux rules directory and in the policy)
 *
 * @param[in] secure_app secure app handler
 * @param[in] selinux_rules_dir some value or NULL for getting default
 * @return 0 in case of success or a negative -errno value
 */
int remove_selinux_rules(const secure_app_t *secure_app, const char *selinux_rules_dir) __wur;

#endif
