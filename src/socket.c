/*
 * Copyright (C) 2018-2023 IoT.bzh Company
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
/******************************************************************************/
/******************************************************************************/
/* IMPLEMENTATION OF SOCKET OPENING FOLLOWING SPECIFICATION                   */
/******************************************************************************/
/******************************************************************************/

#include <stddef.h>
#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "utils.h"

#if defined(WITH_SYSTEMD)
#include <systemd/sd-daemon.h>
#endif

#include "socket.h"

#define BACKLOG 8

/******************************************************************************/

/**
 * known types
 */
enum type {
    /** type internet */
    Type_Inet,

    /** type systemd */
    Type_Systemd,

    /** type Unix */
    Type_Unix
};

/**
 * Structure for known entries
 */
struct entry {
    /** the known prefix */
    const char *prefix;

    /** the type of the entry */
    unsigned type : 2;

    /** should not set SO_REUSEADDR for servers */
    unsigned noreuseaddr : 1;

    /** should not call listen for servers */
    unsigned nolisten : 1;
};

/**
 * The known entries with the default one at the first place
 */
static struct entry entries[] = {{.prefix = "unix:", .type = Type_Unix},
                                 {.prefix = "tcp:", .type = Type_Inet},
                                 {.prefix = "sd:", .type = Type_Systemd, .noreuseaddr = 1, .nolisten = 1}};

/******************************************************************************/

/**
 * open a unix domain socket for client or server
 *
 * @param spec the specification of the path (prefix with @ for abstract)
 * @param server 0 for client, server otherwise
 *
 * @return the file descriptor number of the socket or -1 in case of error
 */
static int open_unix(const char *spec, int server) {
    int fd, rc, abstract;
    struct sockaddr_un addr;
    size_t length;

    abstract = spec[0] == '@';

    /* check the length */
    length = strlen(spec);
    if (length >= 108) {
        errno = ENAMETOOLONG;
        return -1;
    }

    /* create a  socket */
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        return fd;

    /* remove the file on need */
    if (server && !abstract)
        unlink(spec);

    /* prepare address  */
    memset(&addr, 0, sizeof addr);
    addr.sun_family = AF_UNIX;
    secure_strncpy(addr.sun_path, spec, 108);
    if (abstract)
        addr.sun_path[0] = 0; /* implement abstract sockets */

    length += offsetof(struct sockaddr_un, sun_path) + !abstract;
    if (server) {
        rc = bind(fd, (struct sockaddr *) &addr, (socklen_t)length);
    } else {
        rc = connect(fd, (struct sockaddr *) &addr, (socklen_t)length);
    }
    if (rc < 0) {
        close(fd);
        return rc;
    }
    return fd;
}

/**
 * open a tcp socket for client or server
 *
 * @param spec the specification of the host:port/...
 * @param server 0 for client, server otherwise
 *
 * @return the file descriptor number of the socket or -1 in case of error
 */
static int open_tcp(const char *spec, int server) {
    int rc, fd;
    const char *service, *host, *tail;
    struct addrinfo hint, *rai, *iai;

    /* scan the uri */
    tail = strchrnul(spec, '/');
    service = strchr(spec, ':');
    if (service == NULL || tail < service) {
        errno = EINVAL;
        return -1;
    }
    host = strndupa(spec, (size_t)(service++ - spec));
    service = strndupa(service, (size_t)(tail - service));

    /* get addr */
    memset(&hint, 0, sizeof hint);
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    rc = getaddrinfo(host, service, &hint, &rai);
    if (rc != 0) {
        errno = EINVAL;
        return -1;
    }

    /* get the socket */
    iai = rai;
    while (iai != NULL) {
        fd = socket(iai->ai_family, iai->ai_socktype, iai->ai_protocol);
        if (fd >= 0) {
            if (server) {
                rc = bind(fd, iai->ai_addr, iai->ai_addrlen);
            } else {
                rc = connect(fd, iai->ai_addr, iai->ai_addrlen);
            }
            if (rc == 0) {
                freeaddrinfo(rai);
                return fd;
            }
            close(fd);
        }
        iai = iai->ai_next;
    }
    freeaddrinfo(rai);
    return -1;
}

/**
 * open a systemd socket for server
 *
 * @param spec the specification of the systemd name
 *
 * @return the file descriptor number of the socket or -1 in case of error
 */
static int open_systemd(__attribute__((unused)) const char *spec) {
#if defined(WITH_SYSTEMD)
    char **names;
    int fd = -1;
    int c = sd_listen_fds_with_names(0, &names);
    if (c < 0)
        errno = -c;
    else if (names) {
        for (c = 0; names[c]; c++) {
            if (!strcmp(names[c], spec))
                fd = SD_LISTEN_FDS_START + c;
            free(names[c]);
        }
        free(names);
    }
    if (fd < 0 && '0' <= *spec && *spec <= '9')
        fd = SD_LISTEN_FDS_START + atoi(spec);
    return fd;
#else
    errno = EAFNOSUPPORT;
    return -1;
#endif
}

/******************************************************************************/

/**
 * Get the entry of the uri by searching to its prefix
 *
 * @param uri the searched uri
 * @param offset where to store the prefix length
 *
 * @return the found entry or the default one
 */
static struct entry *get_entry(const char *uri, int *offset) {
    int l, i = (int)(sizeof entries / sizeof *entries);

    for (;;) {
        if (!i) {
            l = 0;
            break;
        }
        i--;
        l = (int)strlen(entries[i].prefix);
        if (!strncmp(uri, entries[i].prefix, (size_t)l))
            break;
    }

    *offset = l;
    return &entries[i];
}

/**
 * open socket for client or server
 *
 * @param uri the specification of the socket
 * @param server 0 for client, server otherwise
 *
 * @return the file descriptor number of the socket or -1 in case of error
 */
int socket_open(const char *uri, int server) {
    int fd, rc, offset;
    struct entry *e;

    /* search for the entry */
    e = get_entry(uri, &offset);

    /* get the names */
    uri += offset;

    /* open the socket */
    switch (e->type) {
        case Type_Unix:
            fd = open_unix(uri, server);
            break;
        case Type_Inet:
            fd = open_tcp(uri, server);
            break;
        case Type_Systemd:
            if (server)
                fd = open_systemd(uri);
            else {
                errno = EINVAL;
                fd = -1;
            }
            break;
        default:
            errno = EAFNOSUPPORT;
            fd = -1;
            break;
    }
    if (fd < 0)
        return -1;

    /* set it up */
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    fcntl(fd, F_SETFL, O_NONBLOCK);
    if (server) {
        if (!e->noreuseaddr) {
            rc = 1;
            setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &rc, sizeof rc);
        }
        if (!e->nolisten)
            listen(fd, BACKLOG);
    }
    return fd;
}
