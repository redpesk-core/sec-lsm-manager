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

#ifndef SEC_LSM_MANAGER_SIZES_H
#define SEC_LSM_MANAGER_SIZES_H

#include <limits.h>

// dir + path
#define SEC_LSM_MANAGER_MAX_SIZE_DIR 1024
#define SEC_LSM_MANAGER_MAX_SIZE_PATH 4096
//#define SEC_LSM_MANAGER_MAX_SIZE_PATH PATH_MAX

// id
#define SEC_LSM_MANAGER_MIN_SIZE_ID 2
#define SEC_LSM_MANAGER_MAX_SIZE_ID 200

// label
#define SEC_LSM_MANAGER_MAX_SIZE_LABEL 255

// permissions
#define SEC_LSM_MANAGER_MIN_SIZE_PERMISSION 2
#define SEC_LSM_MANAGER_MAX_SIZE_PERMISSION 1024

// line module
#define SEC_LSM_MANAGER_MAX_SIZE_LINE_MODULE (SEC_LSM_MANAGER_MAX_SIZE_PATH + SEC_LSM_MANAGER_MAX_SIZE_LABEL + 50)

// attr value
#define SEC_LSM_MANAGER_MAX_SIZE_XATTR XATTR_SIZE_MAX

#endif
