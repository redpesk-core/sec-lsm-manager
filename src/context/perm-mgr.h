/*
 * Copyright (C) 2020-2025 IoT.bzh Company
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

#ifndef SEC_LSM_MANAGER_PERM_MGR_H
#define SEC_LSM_MANAGER_PERM_MGR_H

#include "permissions.h"

typedef
struct
{
	/**
	 * @brief Add permissions for a label
	 *
	 * @param[in] label label of the application
	 * @param[in] permission_set array of permission_set_t (can be NULL)
	 * @param[in] drop_before if not zero, a drop is done before
	 * @return 0 in case of success or a negative -errno value
	 */
	__wur __nonnull((1))
	int (*add_permissions)(const char *label,
	                    const permission_set_t *permission_set,
	                    int drop_before);

	/**
	 * @brief Drop permissions of a label
	 *
	 * @param[in] label the label of the application for which to remove permissions
	 * @return 0 in case of success or a negative -errno value
	 */
	__wur __nonnull()
	int (*drop_permissions)(const char *label);

	/**
	 * @brief Check if application of label has the permission
	 *
	 * @param[in] label label of the application
	 * @param[in] permission permission to check
	 * @return 0 when permission is not granted, 1 when it is granted
	 *         or a negative -errno value in case of failure
	 */
	__nonnull() __wur
	int (*check_permission)(const char *label, const char *permission);
}
	perm_mgr_itf_t;

#endif
