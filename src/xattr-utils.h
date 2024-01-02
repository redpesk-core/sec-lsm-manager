/*
 * Copyright (C) 2020-2024 IoT.bzh Company
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

#ifndef SEC_LSM_MANAGER_XATTR_UTILS_H
#define SEC_LSM_MANAGER_XATTR_UTILS_H

#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>

/**
 * @brief Set extended attribute on file
 *
 * @param[in] path the path of the file
 * @param[in] xattr name of the extended attribute
 * @param[in] value value of the extended attribute
 * @return 0 in case of success or a negative -errno value
 */
extern int set_xattr(const char *path, const char *xattr, const char *value) __wur __nonnull();

/**
 * @brief Unset extended attribute on file
 *
 * @param[in] path the path of the file
 * @param[in] xattr name of the extended attribute
 * @return 0 in case of success or a negative -errno value
 */
extern int unset_xattr(const char *path, const char *xattr) __wur __nonnull();

#endif
