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

#ifndef SECURITY_MANAGER_SMACK_H
#define SECURITY_MANAGER_SMACK_H

#include "secure-app.h"

/**
 * @brief Install a secure app for smack
 *
 * @param[in] secure_app secure app handler
 * @return 0 in case of success or a negative -errno value
 */
int install_smack(const secure_app_t *secure_app) __wur;

/**
 * @brief Uninstall a secure app for smack
 *
 * @param[in] secure_app secure app handler
 * @return 0 in case of success or a negative -errno value
 */
int uninstall_smack(const secure_app_t *secure_app) __wur;
#endif