/*
 * Copyright (C) 2020-2024 IoT.bzh Company
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

#ifndef SEC_LSM_MANAGER_XATTR_SELINUX_H
#define SEC_LSM_MANAGER_XATTR_SELINUX_H

#if !SIMULATE_SELINUX
#include "xattr-utils.h"
#else

#include <stdio.h>

__wur __nonnull()
static int set_xattr(const char *path, const char *xattr, const char *value)
{
    printf("set_xattr(%s, %s, %s)\n", path, xattr, value);
    return 0;
}

#if 0
__wur __nonnull()
static int unset_xattr(const char *path, const char *xattr)
{
    printf("unset_xattr(%s, %s)\n", path, xattr);
    return 0;
}
#endif

#endif

#endif
