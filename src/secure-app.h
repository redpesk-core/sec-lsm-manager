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

#ifndef SEC_LSM_MANAGER_SECURE_APP_H
#define SEC_LSM_MANAGER_SECURE_APP_H

#include <sys/types.h>

#include "cynagora-interface.h"
#include "sizes.h"
#include "paths.h"
#include "plugs.h"

typedef struct secure_app {
    char id[SEC_LSM_MANAGER_MAX_SIZE_ID + 1];
    char id_underscore[SEC_LSM_MANAGER_MAX_SIZE_ID + 1];
    char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1];
    permission_set_t permission_set;
    path_set_t path_set;
    plugset_t plugset; /**< set of plug directives */
    bool need_id; /**< flags if id is needed */
    bool error_flag;
} secure_app_t;

/**
 * @brief Initialize the fields 'id', 'id_underscore', 'permission_set', 'path_set' and error_flag
 *
 * @param[in] secure_app handler
 */
__nonnull()
extern void init_secure_app(secure_app_t *secure_app);

/**
 * @brief Create the secure app and init it
 *
 * This secure app need to be destroy at the end
 * if the function succeeded
 *
 * @param[out] pointer to secure_app handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur
extern int create_secure_app(secure_app_t **pointer);

/**
 * @brief Free id, paths and permissions
 * The pointer is not free
 *
 * @param[in] secure_app handler
 */
__nonnull()
extern void clear_secure_app(secure_app_t *secure_app);

/**
 * @brief Destroy the secure app
 * Free secure app handler and content
 *
 * @param[in] secure_app handler
 */
__nonnull()
extern void destroy_secure_app(secure_app_t *secure_app);

/**
 * @brief Alloc and copy id in secure app
 *
 * @param[in] secure_app handler
 * @param[in] id The id to copy
 * @return
 *    * 0              success
 *    * -EINVAL        bad id
 *    * -EEXIST        the id is already set
 *    * -ENOTRECOVERABLE state unrecoverable
 */
__wur __nonnull()
extern int secure_app_set_id(secure_app_t *secure_app, const char *id);

/**
 * @brief Add a new policy in policies field
 *
 * @param[in] secure_app handler
 * @param[in] permission The permission to add
 * @return 0 in case of success or a negative -errno value
 */
__wur __nonnull()
extern int secure_app_add_permission(secure_app_t *secure_app, const char *permission);

/**
 * @brief Add a new path in paths field
 *
 * @param[in] secure_app handler
 * @param[in] path The path to add
 * @param[in] type The path type of the path to add
 * @return
 *    * 0              success
 *    * -EINVAL        bad type or bad path
 *    * -EEXIST        the path is already added
 *    * -ENOMEM        out of memory
 *    * -ENOENT        the path is not existing
 *    * -EACCES        the path can't be accessed
 *    * -ENOTRECOVERABLE state unrecoverable
 */
__wur __nonnull()
extern int secure_app_add_path(secure_app_t *secure_app, const char *path, const char *type);

/**
 * @brief Add a new plug definition
 *
 * @param[in] secure_app handler
 * @param[in] expdir   exported directory
 * @param[in] impid    import appid
 * @param[in] impdir   import directory
 * @return 0 in case of success or a negative -errno value
 */
__wur __nonnull()
extern int secure_app_add_plug(secure_app_t *secure_app, const char *expdir, const char *impid, const char *impdir);

/**
 * @brief Set error_flag
 * The secure_app can't be installed after
 * You need to clear to return in create state
 */
extern void raise_error_flag(secure_app_t *secure_app);

/**
 * @brief Install the application
 *
 * @param[in] secure_app the application to be installed
 * @param[in] cynagora handler to cynagora access
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur
extern int secure_app_install(secure_app_t *secure_app, cynagora_t *cynagora);

/**
 * @brief check if the application has the permission
 *
 * @param[in] secure_app the application to be uninstalled
 * @param[in] permission the permission to check
 * @return 1 when permission is granted or, otherwise, 0
 */
__nonnull() __wur
extern int secure_app_has_permission(const secure_app_t *secure_app, const char *permission);

/**
 * @brief Uninstall the application
 *
 * @param[in] secure_app the application to be uninstalled
 * @param[in] cynagora handler to cynagora access
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur
extern int secure_app_uninstall(secure_app_t *secure_app, cynagora_t *cynagora);

/**
 * @brief Check if application can be installed
 *
 * @param[in] secure_app the application to be checked
 * @param[in] cynagora handler to cynagora access
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur
extern int secure_app_check(secure_app_t *secure_app, cynagora_t *cynagora);

#endif
