/*
 * Copyright (C) 2020 "IoT.bzh"
 * Author Arthur Guyader <arthur.guyader@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
    if (!path) {
        ERROR("path undefined");
        return false;
    }

    struct stat s;
    memset(&s, 0, sizeof(s));

    if (stat(path, &s) == -1) {
        return false;
    }

    return true;
}

/* see utils.h */
bool check_file_type(const char *path, const unsigned short file_type) {
    struct stat s;
    memset(&s, 0, sizeof(s));

    if (!path) {
        ERROR("path undefined");
        return false;
    }

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
    struct stat s;
    memset(&s, 0, sizeof(s));
    int rc = 0;

    if (!path) {
        ERROR("path undefined");
        return false;
    }

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
int remove_file(const char *file) {
    if (!file) {
        ERROR("undefined file");
        return -EINVAL;
    }
    int rc = remove(file);
    if (rc < 0) {
        rc = -errno;
        ERROR("remove %s %m", file);
        return rc;
    }
    return 0;
}
