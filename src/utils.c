/*
 * Copyright (C) 2020-2021 IoT.bzh Company
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

#include "utils.h"

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>

#include "log.h"

static const size_t BLOCKSIZE = 8192;

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see utils.h */
bool valid_label(const char *s) {
    if (!s)
        return false;

    for (size_t i = 0; i < strlen(s); i++) {
        if (!isalnum(s[i]) && s[i] != '-' && s[i] != '_') {
            ERROR("invalid label : need to only contain alphanumeric or '-' or '_'");
            return false;
        }
    }

    return true;
}

/* see utils.h */
int set_label(const char *path, const char *xattr, const char *value) {
    int rc = lsetxattr(path, xattr, value, strlen(value), 0);
    if (rc < 0) {
        rc = -errno;
        ERROR("lsetxattr('%s','%s','%s',%ld,%d) : %m", path, xattr, value, strlen(value), 0);
        return rc;
    }

    DEBUG("set %s=%s on %s", xattr, value, path);

    return 0;
}

/* see utils.h */
bool check_file_exists(const char *path) {
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

    int rc = stat(path, &s);
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

    rc = stat(path, &s);
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
    int rc = remove(path);
    if (rc < 0) {
        rc = -errno;
        ERROR("remove %s %m", path);
        return rc;
    }
    return 0;
}

/* see utils.h */
char *read_file(const char *filename) {
    int f;
    struct stat s;
    char *result;
    size_t size, pos;
    ssize_t rc;

    result = NULL;
    f = open(filename, O_RDONLY);
    if (f < 0) {
        fprintf(stderr, "Can't open file: %s\n", filename);
        goto end;
    }

    fstat(f, &s);
    switch (s.st_mode & S_IFMT) {
        case S_IFREG:
            size = (size_t)s.st_size;
            break;
        case S_IFSOCK:
        case S_IFIFO:
            size = BLOCKSIZE;
            break;
        default:
            fprintf(stderr, "Bad file: %s\n", filename);
            goto error;
    }

    pos = 0;
    result = malloc(size + 1);
    do {
        if (result == NULL) {
            fprintf(stderr, "Out of memory\n");
            goto end2;
        }
        rc = read(f, &result[pos], (size - pos) + 1);
        if (rc < 0) {
            fprintf(stderr, "Error while reading %s\n", filename);
            goto error;
        }
        if (rc > 0) {
            pos += (size_t)rc;
            if (pos > size) {
                size = pos + BLOCKSIZE;
                result = realloc(result, size + 1);
            }
        }
    } while (rc > 0);

    result[pos] = 0;

    goto end2;

error:
    free(result);
    result = NULL;
end2:
    close(f);
end:
    return result;
}
