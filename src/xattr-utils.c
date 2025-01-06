/*
 * Copyright (C) 2020-2025 IoT.bzh Company
 * Author: Arthur Guyader <arthur.guyader@iot.bzh>
 * Author: José Bollo <jose.bollo@iot.bzh>
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

#include "xattr-utils.h"

#include <errno.h>
#include <string.h>
#include <sys/xattr.h>

#include "log.h"

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see xattr-utils.h */
int set_xattr(const char *path, const char *xattr, const char *value) {
    int rc = lsetxattr(path, xattr, value, strlen(value), 0);
    if (rc < 0) {
        rc = -errno;
        ERROR("lsetxattr('%s','%s','%s',%ld,%d) : %d %s", path, xattr, value, strlen(value), 0, -rc, strerror(-rc));
        return rc;
    }

    DEBUG("set %s=%s on %s", xattr, value, path);

    return 0;
}

/* see xattr-utils.h */
int unset_xattr(const char *path, const char *xattr) {
    int rc = lremovexattr(path, xattr);
    if (rc < 0 && errno != ENODATA) {
        rc = -errno;
        ERROR("lremovexattr('%s','%s') : %d %s", path, xattr, -rc, strerror(-rc));
        return rc;
    }

    DEBUG("drop %s from %s", xattr, path);

    return 0;
}

