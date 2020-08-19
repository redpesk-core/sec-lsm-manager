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
int create_smack_rules(const secure_app_t *secure_app, const char *smack_template_file,
                       const char *smack_rules_dir) __wur;

/**
 * @brief Remove smack rules
 *
 * @param[in] secure_app secure app handler
 * @param[in] smack_rules_dir some value or NULL for getting default
 * @return 0 in case of success or a negative -errno value
 */
int remove_smack_rules(const secure_app_t *secure_app, const char *smack_rules_dir) __wur;

#endif