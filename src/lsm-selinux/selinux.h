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

#ifndef SEC_LSM_MANAGER_SELINUX_H
#define SEC_LSM_MANAGER_SELINUX_H

#include "context/context.h"

/**
 * @brief Check if selinux is enabled
 *
 * @return true if enabled
 * @return false if not
 */
extern bool selinux_enabled(void) __wur;

/**
 * @brief Install a context for selinux
 *
 * @param[in] context The handle of context
 * @return 0 in case of success or a negative -errno value
 */
extern int selinux_install(const context_t *context) __wur __nonnull();

/**
 * @brief Uninstall a context for selinux
 *
 * @param[in] context The handle of context
 * @return 0 in case of success or a negative -errno value
 */
extern int selinux_uninstall(const context_t *context) __wur __nonnull();

/**
 * @brief get the security label of the application
 *
 * @param[inout] label array receicing the label
 * @param[in] appid the application identifier
 * @param[in] app_id the application identifier with underscores
 */
__nonnull()
extern void selinux_get_label(char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1], const char *appid);

/************************ FOR TESTING ************************/
#include "selinux-template.h"
__nonnull() __wur
extern int selinux_process_paths(const context_t *context,
                                            path_type_definitions_t path_type_definitions[number_path_type]);
#endif
