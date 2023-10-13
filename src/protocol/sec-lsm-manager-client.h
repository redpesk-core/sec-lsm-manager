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
#ifndef SEC_LSM_MANAGER_H
#define SEC_LSM_MANAGER_H

#include <features.h>

typedef struct sec_lsm_manager sec_lsm_manager_t;

/**
 * @brief Create a client for server sec_lsm_manager
 * The client is created but not connected. The connection is made on need.
 *
 * @param[in] sec_lsm_manager   pointer to sec_lsm_manager client handler
 * @param[in] socketspec specification of the socket to connect to or NULL for
 *                   using the default
 *
 * @return 0 in case of success and in that case *sec_lsm_manager is filled
 *         a negative -errno value and *sec_lsm_manager is set to NULL
 *
 * @see sec_lsm_manager_destroy
 */
extern int sec_lsm_manager_create(sec_lsm_manager_t **sec_lsm_manager, const char *socketspec) __wur;

/**
 * @brief Destroy sec_lsm_manager client handler and release its memory
 *
 * @param[in] sec_lsm_manager sec_lsm_manager client handler
 *
 * @see sec_lsm_manager_create
 */
extern void sec_lsm_manager_destroy(sec_lsm_manager_t *sec_lsm_manager) __nonnull();

/**
 * Ask the sec_lsm_manager client handler to disconnect from the server.
 * The client will reconnect if needed.
 *
 * @param[in] sec_lsm_manager sec_lsm_manager client handler
 */
extern void sec_lsm_manager_disconnect(sec_lsm_manager_t *sec_lsm_manager) __nonnull();

/**
 * @brief Set id of sec_lsm_manager client handler
 *
 * @param[in] sec_lsm_manager sec_lsm_manager client handler
 * @param[in] id The id to define
 * @return 0 in case of success or a negative -errno value
 */
extern int sec_lsm_manager_set_id(sec_lsm_manager_t *sec_lsm_manager, const char *id) __nonnull() __wur;

/**
 * @brief Add a path to sec_lsm_manager client handler
 *
 * @param sec_lsm_manager sec_lsm_manager client handler
 * @param path The path to add
 * @param path_type The path_type to add
 * @return 0 in case of success or a negative -errno value
 */
extern int sec_lsm_manager_add_path(sec_lsm_manager_t *sec_lsm_manager, const char *path, const char *path_type)
    __nonnull() __wur;

/**
 * @brief Add a plug to sec_lsm_manager client handler
 *
 * @param sec_lsm_manager sec_lsm_manager client handler
 * @param expdir   exported directory
 * @param impid    import appid
 * @param impdir   import directory
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur
extern int sec_lsm_manager_add_plug(sec_lsm_manager_t *sec_lsm_manager, const char *expdir, const char *impid, const char *impdir);

/**
 * @brief Add a permission to sec_lsm_manager client handler
 *
 * @param[in] sec_lsm_manager sec_lsm_manager client handler
 * @param[in] permission The permission to add
 * @return 0 in case of success or a negative -errno value
 */
extern int sec_lsm_manager_add_permission(sec_lsm_manager_t *sec_lsm_manager, const char *permission) __nonnull() __wur;

/**
 * @brief Clear the sec_lsm_manager client handler
 * Return in the create state
 *
 * @param sec_lsm_manager sec_lsm_manager client handler
 * @return 0 in case of success or a negative -errno value
 */
extern int sec_lsm_manager_clear(sec_lsm_manager_t *sec_lsm_manager) __nonnull() __wur;

/**
 * @brief Install an application with all defined paramters in the security manager handle
 * You need at least to set the id
 *
 * @param[in] sec_lsm_manager sec_lsm_manager client handler
 * @return 0 in case of success or a negative -errno value
 */
extern int sec_lsm_manager_install(sec_lsm_manager_t *sec_lsm_manager) __nonnull() __wur;

/**
 * @brief Uninstall an application (cynagora permissions, paths)
 * You need at least to set the id
 *
 * @param[in] sec_lsm_manager sec_lsm_manager client handler
 * @return 0 in case of success or a negative -errno value
 */
extern int sec_lsm_manager_uninstall(sec_lsm_manager_t *sec_lsm_manager) __nonnull() __wur;

/**
 * @brief Query or set the logging of requests
 *
 * @param[in] sec_lsm_manager  sec_lsm_manager client handler
 * @param[in] on                should set on
 * @param[in] off               should set off
 *
 * @return 0 if not logging, 1 if logging or a negative -errno value
 */
extern int sec_lsm_manager_log(sec_lsm_manager_t *sec_lsm_manager, int on, int off) __nonnull() __wur;

/**
 * @brief Display the actual state security manager handle
 *
 * @param[in] sec_lsm_manager sec_lsm_manager client handler
 * @return 0 in case of success or a negative -errno value
 */
extern int sec_lsm_manager_display(sec_lsm_manager_t *sec_lsm_manager) __nonnull() __wur;

/**
 * @brief Get copy of the lastest error message. The returned message
 *        must be freed using free.
 *
 * @param[in] sec_lsm_manager sec_lsm_manager client handler
 * @param[inout] message      pointer to where store the pointer to the string
 *
 * @return 0 in case of success or a negative -errno value: -ENOMEM in case
 * of memory depletion or -EINVAL if latest result was not an error.
 */
__nonnull()
extern int sec_lsm_manager_error_message(sec_lsm_manager_t *sec_lsm_manager, char **message);

#endif
