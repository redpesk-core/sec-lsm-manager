/*
 * Copyright (C) 2020 "IoT.bzh"
 * Author Arthur Guyader <arthur.guyader@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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