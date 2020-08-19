/*
 * Copyright (C) 2020 "IoT.bzh"
 * Author José Bollo <jose.bollo@iot.bzh>
 * Author Arthur Guyader <arthur.guyader@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "security-manager-protocol.h"

#include <stdio.h>
#include <stdlib.h>

const char _security_manager_[] = "security-manager", _done_[] = "done", _error_[] = "error", _log_[] = "log",
           _id_[] = "id", _permission_[] = "permission", _path_[] = "path", _install_[] = "install",
           _uninstall_[] = "uninstall", _display_[] = "display", _clean_[] = "clean", _on_[] = "on", _off_[] = "off",
           _string_[] = "string";

#if !defined(DEFAULT_SOCKET_SCHEME)
#define DEFAULT_SOCKET_SCHEME "unix"
#endif

#if !defined(DEFAULT_SOCKET_DIR)
#define DEFAULT_SOCKET_DIR "/var/run/"
#endif

#define DEF_PREFIX DEFAULT_SOCKET_SCHEME ":" DEFAULT_SOCKET_DIR "/"

#if !defined(DEFAULT_SOCKET_BASE)
#define DEFAULT_SOCKET_BASE "security-manager.socket"
#endif

#if !defined(DEFAULT_SOCKET_SPEC)
#define DEFAULT_SOCKET_SPEC DEF_PREFIX DEFAULT_SOCKET_BASE
#endif

const char security_manager_default_socket_scheme[] = DEFAULT_SOCKET_SCHEME,
           security_manager_default_socket_dir[] = DEFAULT_SOCKET_DIR,
           security_manager_default_socket_base[] = DEFAULT_SOCKET_BASE,
           security_manager_default_socket_spec[] = DEFAULT_SOCKET_SPEC;

/* see security-manager-protocol.h */
const char *security_manager_get_socket(const char *value) {
    return value ?: secure_getenv("SECURITY_MANAGER_SOCKET") ?: security_manager_default_socket_spec;
}
