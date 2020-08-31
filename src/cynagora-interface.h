/*
 * Copyright (C) 2020 IoT.bzh Company
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

#ifndef SECURITY_MANAGER_CYNAGORA_INTERFACE_H
#define SECURITY_MANAGER_CYNAGORA_INTERFACE_H

#include "policies.h"

/**
 * @brief Drop old policies of cynagora for an id (client)
 *
 * @param[in] cynagora cynagora admin client
 * @param[in] client the id of the application for which to remove permissions
 * @return 0 in case of success or a negative -errno value
 */
int cynagora_drop_policies(cynagora_t *cynagora, const char *client) __wur __nonnull();

/**
 * @brief Define new permissions in cynagora
 *
 * @param[in] cynagora cynagora admin client
 * @param[in] policies array of policies_t
 * @return 0 in case of success or a negative -errno value
 */
int cynagora_set_policies(cynagora_t *cynagora, const policies_t *policies) __wur;
#endif
