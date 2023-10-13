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

#ifndef SEC_LSM_MANAGER_SMACK_TEMPLATE_H
#define SEC_LSM_MANAGER_SMACK_TEMPLATE_H

#include <sys/cdefs.h>

#include "context/context.h"

typedef struct path_type_definitions {
    char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL];
    bool is_executable;
    bool is_transmute;
} path_type_definitions_t;

/**
 * @brief Get the selinux template file
 *
 * @param[in] value some value or NULL for getting default
 * @return the selinux template file specification
 */
extern const char *get_smack_template_file(const char *value) __wur;

/**
 * @brief Get the smack policy directory
 *
 * @param[in] value some value or NULL for getting default
 * @return the smack policy directory specification
 */
extern const char *get_smack_policy_dir(const char *value) __wur;

/**
 * @brief Get the smack policy path for the application of id
 *
 * @param[inout] path where to store the computed path
 * @param[in] id id of the application
 * @return the length stored or a negative error code
 */
extern int get_smack_rule_path(char rule_path[SEC_LSM_MANAGER_MAX_SIZE_PATH + 1], const char *id);

/**
 * @brief Check if smack is enabled
 *
 * @return true if enabled
 * @return false if not
 */
extern bool smack_enabled(void) __wur;

/**
 * @brief Init different labels for all path type
 *
 * @param[in] path_type_definitions Array definition to complete
 * @param[in] id to generate label of an application
 *
 */
extern void init_path_type_definitions(path_type_definitions_t path_type_definitions[number_path_type], const char *id)
    __nonnull();

/**
 * @brief Create smack rules
 *
 * @param[in] context context handler to install
 * @return 0 in case of success or a negative -errno value
 */
extern int create_smack_rules(const context_t *context) __wur __nonnull();

/**
 * @brief Remove smack rules
 *
 * @param[in] context context handler
 * @return 0 in case of success or a negative -errno value
 */
extern int remove_smack_rules(const context_t *context) __wur __nonnull();

#endif
