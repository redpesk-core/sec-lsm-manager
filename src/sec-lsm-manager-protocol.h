/*
 * Copyright (C) 2020-2023 IoT.bzh Company
 * Author: José Bollo <jose.bollo@iot.bzh>
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

#ifndef SEC_LSM_MANAGER_PROTOCOL_H
#define SEC_LSM_MANAGER_PROTOCOL_H

#define CHECK_NO_NULL(param, param_name)   \
    if (!param) {                          \
        ERROR("%s undefined", param_name); \
        return -EINVAL;                    \
    }

#define CHECK_NO_NULL_NO_RETURN(param, param_name) \
    if (!param) {                                  \
        ERROR("%s undefined", param_name);         \
        return;                                    \
    }

extern const char _clear_[];
extern const char _display_[];
extern const char _done_[];
extern const char _error_[];
extern const char _id_[];
extern const char _install_[];
extern const char _log_[];
extern const char _off_[];
extern const char _on_[];
extern const char _path_[];
extern const char _permission_[];
extern const char _plug_[];
extern const char _sec_lsm_manager_[];
extern const char _string_[];
extern const char _uninstall_[];

/* predefined names */
extern const char sec_lsm_manager_default_socket_scheme[];
extern const char sec_lsm_manager_default_socket_dir[];
extern const char sec_lsm_manager_default_socket_name[];
extern const char sec_lsm_manager_default_socket[];

/**
 * @brief Get the socket specification
 *
 * @param[in] value some value or NULL for getting default
 * @return the socket specification
 */
extern const char *sec_lsm_manager_get_socket(const char *value);

#endif
