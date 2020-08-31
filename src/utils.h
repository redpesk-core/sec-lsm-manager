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
bool check_file_exists(const char *path) __wur __nonnull();

/**
 * @brief Check the type of a file
 *
 * @param[in] path The path of the file
 * @param[in] type_file the type of the file
 * @return true if good type
 * @return false if not
 */
bool check_file_type(const char *path, const unsigned short type_file) __wur __nonnull();

/**
 * @brief Check if a file is executable by owner
 *
 * @param[in] path The path of the file
 * @return true if executable
 * @return false if not
 */
bool check_executable(const char *path) __wur __nonnull();

/**
 * @brief Remove a file
 *
 * @param[in] file The path of the file
 * @return 0 in case of success or a negative -errno value
 */
int remove_file(const char *path) __wur __nonnull();

#endif
