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

#include "sec-lsm-manager-protocol.h"

#include <stdio.h>
#include <stdlib.h>

const char _clear_[] = "clear";
const char _display_[] = "display";
const char _done_[] = "done";
const char _error_[] = "error";
const char _id_[] = "id";
const char _install_[] = "install";
const char _log_[] = "log";
const char _off_[] = "off";
const char _on_[] = "on";
const char _path_[] = "path";
const char _permission_[] = "permission";
const char _plug_[] = "plug";
const char _sec_lsm_manager_[] = "sec-lsm-manager";
const char _string_[] = "string";
const char _uninstall_[] = "uninstall";


#if !defined(SEC_LSM_MANAGER_SOCKET_SCHEME)
#define SEC_LSM_MANAGER_SOCKET_SCHEME "unix"
#endif

#if !defined(SEC_LSM_MANAGER_SOCKET_DIR)
#define SEC_LSM_MANAGER_SOCKET_DIR "/var/run"
#endif

#define PREFIX SEC_LSM_MANAGER_SOCKET_SCHEME ":" SEC_LSM_MANAGER_SOCKET_DIR "/"

#if !defined(SEC_LSM_MANAGER_SOCKET_NAME)
#define SEC_LSM_MANAGER_SOCKET_NAME "sec-lsm-manager.socket"
#endif

#if !defined(SEC_LSM_MANAGER_SOCKET)
#define SEC_LSM_MANAGER_SOCKET PREFIX SEC_LSM_MANAGER_SOCKET_NAME
#endif

const char sec_lsm_manager_default_socket_scheme[] = SEC_LSM_MANAGER_SOCKET_SCHEME;
const char sec_lsm_manager_default_socket_dir[] = SEC_LSM_MANAGER_SOCKET_DIR;
const char sec_lsm_manager_default_socket_name[] = SEC_LSM_MANAGER_SOCKET_NAME;
const char sec_lsm_manager_default_socket[] = SEC_LSM_MANAGER_SOCKET;

/* see sec-lsm-manager-protocol.h */
const char *sec_lsm_manager_get_socket(const char *value) {
    if (value == NULL) {
        value = secure_getenv("SEC_LSM_MANAGER_SOCKET");
        if (value == NULL)
            value = sec_lsm_manager_default_socket;
    }
    return value;
}
