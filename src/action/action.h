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

#ifndef SEC_LSM_MANAGER_ACTION_H
#define SEC_LSM_MANAGER_ACTION_H

#include "cynagora-interface.h"
#include "context/context.h"

/**
 * @brief Install the application
 *
 * @param[in] context the application to be installed
 * @param[in] cynagora handler to cynagora access
 * @return
 *    * 0        success
 *    * -EINVAL  the application identifier is missing
 *    * -EPERM   no permission to install plugin
 *    * -ENOTRECOVERABLE state unrecoverable
 *    * other negative values are possible
 */
__nonnull() __wur
extern int action_install(context_t *context, cynagora_t *cynagora);

/**
 * @brief Uninstall the application
 *
 * @param[in] context the application to be uninstalled
 * @param[in] cynagora handler to cynagora access
 * @return
 *    * 0        success
 *    * -EINVAL  the application identifier is missing
 *    * -ENOTRECOVERABLE state unrecoverable
 *    * other negative values are possible
 */
__nonnull() __wur
extern int action_uninstall(context_t *context, cynagora_t *cynagora);

#endif
