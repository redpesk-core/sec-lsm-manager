/*
 * Copyright (C) 2020-2021 IoT.bzh Company
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

const char _sec_lsm_manager_[] = "sec-lsm-manager", _done_[] = "done", _error_[] = "error", _log_[] = "log",
           _id_[] = "id", _permission_[] = "permission", _path_[] = "path", _install_[] = "install",
           _uninstall_[] = "uninstall", _display_[] = "display", _clear_[] = "clear", _on_[] = "on", _off_[] = "off",
           _string_[] = "string";

#if !defined(DEFAULT_SOCKET_SCHEME)
#define DEFAULT_SOCKET_SCHEME "unix"
#endif

#if !defined(DEFAULT_SOCKET_DIR)
#define DEFAULT_SOCKET_DIR "/var/run/"
#endif

#define DEF_PREFIX DEFAULT_SOCKET_SCHEME ":" DEFAULT_SOCKET_DIR "/"

#if !defined(DEFAULT_SOCKET_BASE)
#define DEFAULT_SOCKET_BASE "sec-lsm-manager.socket"
#endif

#if !defined(DEFAULT_SOCKET_SPEC)
#define DEFAULT_SOCKET_SPEC DEF_PREFIX DEFAULT_SOCKET_BASE
#endif

const char sec_lsm_manager_default_socket_scheme[] = DEFAULT_SOCKET_SCHEME,
           sec_lsm_manager_default_socket_dir[] = DEFAULT_SOCKET_DIR,
           sec_lsm_manager_default_socket_base[] = DEFAULT_SOCKET_BASE,
           sec_lsm_manager_default_socket_spec[] = DEFAULT_SOCKET_SPEC;

/* see sec-lsm-manager-protocol.h */
const char *sec_lsm_manager_get_socket(const char *value) {
    return value ?: secure_getenv("SEC_LSM_MANAGER_SOCKET") ?: sec_lsm_manager_default_socket_spec;
}
