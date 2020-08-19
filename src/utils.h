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

#ifndef SECURITY_MANAGER_UTILS_H
#define SECURITY_MANAGER_UTILS_H

#include <stdbool.h>
#include <sys/types.h>

/**
 * @brief Check if file exists
 *
 * @param[in] path The path of the file
 * @return true if exists
 * @return false if not
 */
bool check_file_exists(const char *path) __wur;

/**
 * @brief Check the type of a file
 *
 * @param[in] path The path of the file
 * @param[in] type_file the type of the file
 * @return true if good type
 * @return false if not
 */
bool check_file_type(const char *path, const unsigned short type_file) __wur;

/**
 * @brief Check if a file is executable by owner
 *
 * @param[in] path The path of the file
 * @return true if executable
 * @return false if not
 */
bool check_executable(const char *path) __wur;

/**
 * @brief Remove a file
 *
 * @param[in] file The path of the file
 * @return 0 in case of success or a negative -errno value
 */
int remove_file(const char *file) __wur;

#endif