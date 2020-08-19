/*
 * Copyright (C) 2020 "IoT.bzh"
 * Author José Bollo <jose.bollo@iot.bzh>
 * Author Arthur Guyader <arthur.guyader@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SECURITY_MANAGER_SERVER_H
#define SECURITY_MANAGER_SERVER_H

typedef struct security_manager_server security_manager_server_t;

/**
 * @brief Boolean flag telling whether the server logs or not its received commands
 */
extern int security_manager_server_log;

/**
 * @brief Create a security manager server
 *
 * @param[out] server where to store the handler of the created server
 * @param[in] socket_spec specification of socket
 *
 * @return 0 on success or a negative value
 *
 * @see security_manager_server_destroy
 */
extern int security_manager_server_create(security_manager_server_t **server, const char *security_manager_socket_spec);

/**
 * @brief Destroy a created server and release its resources
 *
 * @param[in] server the handler of the server
 *
 * @see security_manager_server_create
 */
extern void security_manager_server_destroy(security_manager_server_t *server);

/**
 * @brief Start the security_manager server and returns only when stopped
 *
 * @param[in] server the handler of the server
 *
 * @return 0 on success or a negative value
 *
 * @see security_manager_server_stop
 */
extern int security_manager_server_serve(security_manager_server_t *server);

/**
 * @brief Stop the security_manager server
 *
 * @param[in] server the handler of the server
 * @param[in] status the status that the function security_manager_server_serve should return
 *
 * @see security_manager_server_serve
 */
extern void security_manager_server_stop(security_manager_server_t *server, int status);

#endif