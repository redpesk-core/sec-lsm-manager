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

#ifndef SEC_LSM_MANAGER_SELINUX_TEMPLATE_H
#define SEC_LSM_MANAGER_SELINUX_TEMPLATE_H

#include "secure-app.h"

#if SIMULATE_SELINUX
#include "simulation/selinux/selinux.h"
#else
#include <selinux/restorecon.h>
#include <selinux/selinux.h>
#include <semanage/semanage.h>
#endif

typedef struct path_type_definitions {
    char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL];
} path_type_definitions_t;

/**
 * @brief Get the selinux te template file
 *
 * @param[in] value some value or NULL for getting default
 * @return the selinux te template file specification
 */
extern const char *get_selinux_te_template_file(const char *value) __wur;

/**
 * @brief Get the selinux if template file
 *
 * @param[in] value some value or NULL for getting default
 * @return the selinux if template file specification
 */
extern const char *get_selinux_if_template_file(const char *value) __wur;

/**
 * @brief Get the selinux rules directory
 *
 * @param[in] value some value or NULL for getting default
 * @return the selinux rules directory specification
 */
extern const char *get_selinux_rules_dir(const char *value) __wur;

/**
 * @brief Init differents labels for all path type
 *
 * @param[in] path_type_definitions Array definition to complete
 * @param[in] id to generate label of an application
 *
 */
extern void init_path_type_definitions(path_type_definitions_t path_type_definitions[number_path_type], const char *id)
    __nonnull((2));

/**
 * @brief Create selinux rules
 *
 * @param[in] secure_app secure app handler to install
 * @return 0 in case of success or a negative -errno value
 */
extern int create_selinux_rules(const secure_app_t *secure_app,
                         path_type_definitions_t path_type_definitions[number_path_type]) __wur __nonnull();

/**
 * @brief Check if the files of an application exists in the selinux rules directory
 *
 * @param[in] secure_app secure app handler
 * @param[in] selinux_rules_dir some value or NULL for getting default
 * @return true if exists, false if not
 */
extern bool check_module_files_exist(const secure_app_t *secure_app) __wur __nonnull((1));

/**
 * @brief Check if module is in the policy
 *
 * @param[in] secure_app secure app handler
 * @return true if exists, false if not
 */
extern bool check_module_in_policy(const secure_app_t *secure_app) __wur __nonnull();

/**
 * @brief Remove selinux rules (in the selinux rules directory and in the policy)
 *
 * @param[in] secure_app secure app handler
 * @return 0 in case of success or a negative -errno value
 */
extern int remove_selinux_rules(const secure_app_t *secure_app) __wur __nonnull();

#endif
