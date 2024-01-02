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

#ifndef SEC_LSM_MANAGER_XATTR_SMACK_H
#define SEC_LSM_MANAGER_XATTR_SMACK_H

#include "xattr-utils.h"

#if SIMULATE_SMACK

#include <linux/xattr.h>
#include <stdio.h>

#undef XATTR_NAME_SMACK
#undef XATTR_NAME_SMACKEXEC
#undef XATTR_NAME_SMACKTRANSMUTE
#undef XATTR_NAME_SMACKIPIN
#undef XATTR_NAME_SMACKIPOUT
#undef XATTR_NAME_SMACKMMAP

#define XATTR_NAME_SMACK          XATTR_USER_PREFIX "simul." XATTR_SMACK_SUFFIX
#define XATTR_NAME_SMACKEXEC      XATTR_USER_PREFIX "simul." XATTR_SMACK_EXEC
#define XATTR_NAME_SMACKTRANSMUTE XATTR_USER_PREFIX "simul." XATTR_SMACK_TRANSMUTE
#define XATTR_NAME_SMACKIPIN      XATTR_USER_PREFIX "simul." XATTR_SMACK_IPIN
#define XATTR_NAME_SMACKIPOUT     XATTR_USER_PREFIX "simul." XATTR_SMACK_IPOUT
#define XATTR_NAME_SMACKMMAP      XATTR_USER_PREFIX "simul." XATTR_SMACK_MMAP

__wur __nonnull()
static inline int simulate_set_xattr(const char *path, const char *xattr, const char *value)
{
    printf("set_xattr(%s, %s, %s)\n", path, xattr, value);
    return set_xattr(path, xattr, value);
}

__wur __nonnull()
static inline int simulate_unset_xattr(const char *path, const char *xattr)
{
    printf("unset_xattr(%s, %s)\n", path, xattr);
    return unset_xattr(path, xattr);
}

#define set_xattr    simulate_set_xattr
#define unset_xattr  simulate_unset_xattr

#endif

#endif
