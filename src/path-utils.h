/*
 * Copyright (C) 2020-2025 IoT.bzh Company
 * Author: José Bollo <jose.bollo@iot.bzh>
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

#ifndef SECURITY_MANAGER_PATH_UTILS_H
#define SECURITY_MANAGER_PATH_UTILS_H

#include <stddef.h>
#include <sys/cdefs.h>

/**
 * @brief make the standard path and return its length
 *
 * @param buffer where to store the standard path (might be NULL if size == 0)
 * @param size size in bytes of the buffer
 * @param path the path to standardize
 *
 * @return the length of the standardized path
 */
__nonnull((3)) __wur
extern size_t path_std(char *buffer, size_t size, const char *path);

#endif

