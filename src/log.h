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

#ifndef SEC_LSM_MANAGER_LOG_H
#define SEC_LSM_MANAGER_LOG_H

/**
 * @brief Log (>>) a message and arguments
 *
 * @param msg The message to display
 * @param ... arguments
 */
extern void log_function(const char *msg, ...) __attribute__((format(printf, 1, 2)));

#define LOG(...) log_function(__VA_ARGS__)

/**
 * @brief Print a message and arguments when debugging mode
 *
 * @param msg The message to display
 * @param ... arguments
 */
extern void debug_function(const char *msg, ...) __attribute__((format(printf, 1, 2)));

#define DEBUG(...) debug_function(__VA_ARGS__)

/**
 * @brief Display an error with details
 *
 * @param file The file where occurs the error
 * @param line The line where occurs the error
 * @param msg The message to display
 * @param ... arguments
 */
extern void error_function(const char *file, const int line, const char *msg, ...) __attribute__((format(printf, 3, 4)));

#define ERROR(...) error_function(__FILE__, __LINE__, __VA_ARGS__)

#endif
