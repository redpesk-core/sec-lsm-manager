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

#include "utils.h"

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "log.h"

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see utils.h */
bool check_file_exists(const char *path) {
    CHECK_NO_NULL_RETURN_BOOL(path, "path");

    struct stat s;
    memset(&s, 0, sizeof(s));

    if (stat(path, &s) == -1) {
        return false;
    }

    return true;
}

/* see utils.h */
bool check_file_type(const char *path, const unsigned short file_type) {
    CHECK_NO_NULL_RETURN_BOOL(path, "path");

    struct stat s;
    memset(&s, 0, sizeof(s));

    int rc = stat(path, &s) == -1;
    if (rc < 0) {
        ERROR("stat failed : %d %s", errno, strerror(errno));
        return false;
    }

    switch (file_type) {
        case __S_IFDIR:
        case __S_IFCHR:
        case __S_IFBLK:
        case __S_IFREG:
        case __S_IFIFO:
        case __S_IFLNK:
        case __S_IFSOCK:
            break;
        default:
            ERROR("Type undefined");
            return false;
    }

    if (__S_ISTYPE(s.st_mode, file_type) != 0) {
        return true;
    } else {
        return false;
    }
}

/* see utils.h */
bool check_executable(const char *path) {
    CHECK_NO_NULL_RETURN_BOOL(path, "path");

    struct stat s;
    memset(&s, 0, sizeof(s));
    int rc = 0;

    rc = stat(path, &s) == -1;
    if (rc < 0) {
        ERROR("stat failed : %d %s", errno, strerror(errno));
        return false;
    }

    if (s.st_mode & S_IXUSR)
        return true;
    else
        return false;
}

/* see utils.h */
int remove_file(const char *path) {
    CHECK_NO_NULL(path, "path");

    int rc = remove(path);
    if (rc < 0) {
        rc = -errno;
        ERROR("remove %s %m", path);
        return rc;
    }
    return 0;
}
