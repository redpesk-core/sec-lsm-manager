/*
 * Copyright (C) 2020-2025 IoT.bzh Company
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

#ifndef SEC_LSM_MANAGER_FILE_UTILS_H
#define SEC_LSM_MANAGER_FILE_UTILS_H

#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>

/**
 * @brief Check if file exists
 *
 * @param[in] path The path of the file
 * @param[in] follow follow links
 * @param[out] exists true if file exists
 * @param[out] is_exec true if file is regular and executable
 * @param[out] is_dir true if file is a directory
 */
extern void get_file_informations(const char *path, bool follow, bool *exists, bool *is_exec, bool *is_dir) __nonnull((1));

/**
 * @brief Create a file
 *
 * @param[in] path The path of the file
 * @return 0 in case of success or a negative -errno value
 */
extern int create_file(const char *path) __wur __nonnull();

/**
 * @brief Remove a file
 *
 * @param[in] path The path of the file
 * @return 0 in case of success or a negative -errno value
 */
extern int remove_file(const char *path) __wur __nonnull();

/**
 * @brief Read content of a file
 *
 * @param[in] path The path of the file
 * @return the content of the file in case of success or NULL
 */
extern char *read_file(const char *path);

/**
 * @brief Get property of the path
 *
 * @param[in] path the path of the file
 * @param[in] follow follow links, don't return PATH_LINK
 * @return
 *   - PATH_FILE_DATA == 0  it exists and is a simple file
 *   - PATH_FILE_EXEC == 1  it exists and is an executable file
 *   - PATH_DIRECTORY == 2  it exists and is a directory
 *   - PATH_LINK == 3       it exists and is a link
 *   - -ENOENT  it doesn't exist
 *   - -EACCES  not allowed to access it
 *   - -ENOMEM  no more kernel memory
 */
__nonnull() __wur
extern int get_path_property(const char path[], bool follow);
#define PATH_FILE_DATA 0
#define PATH_FILE_EXEC 1
#define PATH_DIRECTORY 2
#define PATH_LINK      3

/**
 * @brief Check if the path exists
 * @param[in] path the path to check
 * @param[in] follow follow links
 * @return
 *   - 0        it exists
 *   - -ENOENT  it doesn't exist
 *   - -EACCES  not allowed to access it
 *   - -ENOMEM  no more kernel memory
 */
__nonnull() __wur
extern int check_path_exists(const char path[], bool follow);

/**
 * @brief Check if the path exists and is a directory
 * @param[in] path the path to check
 * @param[in] follow follow links
 * @return
 *   - 0        it exists
 *   - -ENOENT  it doesn't exist
 *   - -EACCES  not allowed to access it
 *   - -ENOMEM  no more kernel memory
 *   - -ENOTDIR path exists but is not a directory
 */
__nonnull() __wur
extern int check_directory_exists(const char path[], bool follow);

#endif
