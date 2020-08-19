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

#ifndef SECURITY_MANAGER_PROTOCOL_H
#define SECURITY_MANAGER_PROTOCOL_H

extern const char _security_manager_[], _done_[], _error_[], _log_[], _id_[], _permission_[], _path_[], _install_[],
    _uninstall_[], _display_[], _clean_[], _on_[], _off_[], _string_[];

/* predefined names */
extern const char security_manager_default_socket_scheme[], security_manager_default_socket_dir[],
    security_manager_default_socket_base[], security_manager_default_socket_spec[];

/**
 * @brief Get the socket specification
 *
 * @param[in] value some value or NULL for getting default
 * @return the socket specification
 */
extern const char *security_manager_get_socket(const char *value);

#endif