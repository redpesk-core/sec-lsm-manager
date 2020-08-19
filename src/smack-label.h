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

#ifndef SECURITY_MANAGER_SMACK_LABEL_H
#define SECURITY_MANAGER_SMACK_LABEL_H

#ifndef SIMULATE_MAC
#include <sys/smack.h>
#else
#include "simulation/sys/smack.h"
#endif

#include "paths.h"

extern char prefix_app[];
extern char suffix_lib[];
extern char suffix_conf[];
extern char suffix_exec[];
extern char suffix_icon[];
extern char suffix_data[];
extern char suffix_http[];
extern char user_home[];
extern char public_app[];

/**
 * @brief Check if smack is enabled
 *
 * @return true if enabled
 * @return false if not
 */
int smack_enabled() __wur;

/**
 * @brief Generate smack label
 *
 * @param[out] label allocate and set the label
 * @param[in] id The id of the application
 * @param[in] prefix The prefix to add at the begin of the label
 * @param[in] suffix The suffix to add at the end of the label
 * @return 0 in case of success or a negative -errno value
 */
int generate_label(char **label, const char *id, const char *prefix, const char *suffix) __wur;

/**
 * @brief Get path type info in a selinux context
 *
 * @param[in] path_type The path type information
 * @param[out] suffix The suffix to choose
 * @param[out] is_executable true if path is executable
 * @param[out] is_transmute true if path is transmute
 * @param[out] is_public true if path is public
 * @return 0 in case of success or a negative -errno value
 */
int get_path_type_info(enum path_type path_type, char **suffix, bool *is_executable, bool *is_transmute,
                       bool *is_public) __wur;

#endif