/*
 * Copyright (C) 2020-2023 IoT.bzh Company
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

#ifndef SEC_LSM_MANAGER_MAC_INTERFACE_H
#define SEC_LSM_MANAGER_MAC_INTERFACE_H

#include "context/context.h"

 __wur __nonnull() extern int mac_install(const context_t *context);
 __wur __nonnull() extern int mac_uninstall(const context_t *context);
__nonnull() extern void mac_get_label(char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1], const char *appid);

#endif
