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

#ifndef SEC_LSM_MANAGER_SELINUX_COMPILE_H
#define SEC_LSM_MANAGER_SELINUX_COMPILE_H

#if !defined(COMPILE_SCRIPT_DIR)
#define COMPILE_SCRIPT_DIR "/usr/share/sec-lsm-manager/script"
#endif

#if !defined(COMPILE_SCRIPT_NAME)
#define COMPILE_SCRIPT_NAME "build-module.sh"
#endif

#define COMPILE_SCRIPT COMPILE_SCRIPT_DIR "/" COMPILE_SCRIPT_NAME

#include <sys/cdefs.h>

/**
 * @brief Launch COMPILE_SCRIPT
 *
 * @param[in] id id of module to compile
 * @return 0 in case of success or a negative -errno value
 */
extern int launch_compile(const char* id) __wur;

#endif
