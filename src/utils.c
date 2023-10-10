/*
 * Copyright (C) 2020-2023 IoT.bzh Company
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

char *secure_strncpy(char *dest, const char *src, size_t n) {
    char *ret = NULL;
    if (dest != NULL && src != NULL) {
        ret = strncpy(dest, src, n - 1);
        if (n > 0)
            dest[n - 1] = '\0';
    }
    return ret;
}

/* see utils.h */
int set_label(const char *path, const char *xattr, const char *value) {
    int rc = lsetxattr(path, xattr, value, strlen(value), 0);
    if (rc < 0) {
        rc = -errno;
        ERROR("lsetxattr('%s','%s','%s',%ld,%d) : %d %s", path, xattr, value, strlen(value), 0, -rc, strerror(-rc));
        return rc;
    }

    DEBUG("set %s=%s on %s", xattr, value, path);

    return 0;
}

/* see utils.h */
int unset_label(const char *path, const char *xattr) {
    int rc = lremovexattr(path, xattr);
    if (rc < 0 && errno != ENODATA) {
        rc = -errno;
        ERROR("lremovexattr('%s','%s') : %d %s", path, xattr, -rc, strerror(-rc));
        return rc;
    }

    DEBUG("drop %s from %s", xattr, path);

    return 0;
}

void get_file_informations(const char *path, bool *exists, bool *is_exec, bool *is_dir) {
    struct stat s;
    memset(&s, 0, sizeof(s));

    if (exists)
        *exists = false;
    if (is_exec)
        *is_exec = false;
    if (is_dir)
        *is_dir = false;

    if (stat(path, &s) != 0) {
        return;
    }

    if (exists)
        *exists = true;
    if (is_exec)
        *is_exec = __S_ISTYPE(s.st_mode, __S_IFREG) && (s.st_mode & S_IXUSR || s.st_mode & S_IXGRP);
    if (is_dir)
        *is_dir = __S_ISTYPE(s.st_mode, __S_IFDIR);
}

/* see utils.h */
int create_file(const char *path) {
    int rc;
    int fd = creat(path, S_IRWXU | S_IRWXG);
    if (fd < 0) {
        rc = -errno;
        ERROR("creat %s : %d %s", path, -rc, strerror(-rc));
        return rc;
    }
    close(fd);
    return 0;
}

/* see utils.h */
int remove_file(const char *path) {
    int rc = remove(path);
    if (rc < 0) {
        rc = -errno;
        ERROR("remove %s : %d %s", path, -rc, strerror(-rc));
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

/* see utils.h */
__nonnull()
int get_path_property(const char path[])
{
    struct stat s;
    int rc = stat(path, &s);
    if (rc < 0) {
        switch (errno) {
        case ENOENT:
        case ENOTDIR: rc = -ENOENT; break;
        case ENOMEM: rc = -ENOMEM; break;
        default: rc = -EACCES; break;
        }
    }
    else if (S_ISDIR(s.st_mode))
        rc = PATH_DIRECTORY;
    else if ((s.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0)
        rc = PATH_FILE_EXEC;
    else
        rc = PATH_FILE_DATA;
    return rc;
}

/* see utils.h */
int check_path_exists(const char path[])
{
    int rc = get_path_property(path);
    return rc < 0 ? rc : 0;
}

/* see utils.h */
int check_directory_exists(const char path[])
{
    int rc = get_path_property(path);
    return rc < 0 ? rc : rc == PATH_DIRECTORY ? 0 : -ENOTDIR;
}
