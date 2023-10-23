/*
 * Copyright (C) 2018-2023 IoT.bzh Company
 * Author: José Bollo <jose.bollo@iot.bzh>
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

#include "sec-lsm-manager-server.h"

#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "client.h"
#include "log.h"
#include "pollitem.h"
#include "sec-lsm-manager-protocol.h"
#include "socket.h"

#ifndef MAX_CLIENT_COUNT
#define MAX_CLIENT_COUNT 3
#endif

/** log of the protocol */
bool sec_lsm_manager_server_log = false;

/** structure for servers */
struct sec_lsm_manager_server {
    /** the pollfd to use */
    int pollfd;

    /** count of clients */
    int client_count;

    /** stop status */
    int stoprc;

    /** is stopped ? */
    bool stopped;

    /** is deaf ? */
    bool deaf;

    /** the server socket */
    pollitem_t pollitem;

    /** the clients */
    client_t *clients[MAX_CLIENT_COUNT];
};

/**
 * @brief handle server events
 *
 * @param[in] pollitem pollitem of requests
 * @param[in] events events receive
 * @param[in] pollfd pollfd of the server
 */
static void on_server_event(pollitem_t *pollitem, uint32_t events, int pollfd)
{
    int servfd = pollitem->fd;
    int fd, rc;
    struct sockaddr saddr;
    socklen_t slen;
    sec_lsm_manager_server_t *server = (sec_lsm_manager_server_t *)pollitem->closure;

    /* is it a hangup? it shouldn't! */
    if (events & EPOLLHUP) {
        ERROR("unexpected server socket closing");
        sec_lsm_manager_server_stop(server, -EPIPE);
        return;
    }

    /* EPOLLIN is the only expected event but asserting makes fear */
    if (!(events & EPOLLIN))
        return;

    /* accept the connection */
    slen = (socklen_t)sizeof(saddr);
    fd = accept(servfd, &saddr, &slen);
    if (fd < 0) {
        ERROR("can't accept connection: %s", strerror(errno));
        return;
    }
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    fcntl(fd, F_SETFL, O_NONBLOCK);

    /* create a client for the connection */
    rc = client_create(&server->clients[server->client_count], fd, pollfd);
    if (rc < 0) {
        ERROR("can't create client connection: %d %s", -rc, strerror(-rc));
        close(fd);
        return;
    }
    server->client_count++;

    /* check if full of clients */
    if (server->client_count == MAX_CLIENT_COUNT) {
        /* if full avoid accepting new clients */
        rc = pollitem_mod(&server->pollitem, 0, server->pollfd);
        if (rc < 0) {
            ERROR("unexpected server socket error");
            sec_lsm_manager_server_stop(server, rc);
        }
        server->deaf = true;
    }

    DEBUG("starting new client connection");
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see sec-lsm-manager-server.h */
void sec_lsm_manager_server_destroy(sec_lsm_manager_server_t *server)
{
    pollitem_del(&server->pollitem, server->pollfd);
    while (server->client_count > 0)
        client_destroy(server->clients[--server->client_count]);
    close(server->pollitem.fd);
    close(server->pollfd);
    free(server);
}

/* see sec-lsm-manager-server.h */
__wur __nonnull((1))
int sec_lsm_manager_server_create(sec_lsm_manager_server_t **pserver, const char *socket_spec)
{
    mode_t um;
    int rc = 0;
    sec_lsm_manager_server_t *server;

    /* get real effective socket spec */
    socket_spec = sec_lsm_manager_get_socket(socket_spec);
    DEBUG("sec_lsm_manager_server_create %s", socket_spec);

    /* allocate the structure */
    server = (sec_lsm_manager_server_t*)calloc(1, sizeof *server);
    if (server == NULL) {
        ERROR("malloc failed");
        rc = -ENOMEM;
        goto ret;
    }

    /* create the polling fd */
    server->pollfd = epoll_create1(EPOLL_CLOEXEC);
    if (server->pollfd < 0) {
        rc = -errno;
        ERROR("create polling: %d %s", -rc, strerror(-rc));
        goto error;
    }

    /* create the admin server socket */
    um = umask(017);
    server->pollitem.fd = socket_open(socket_spec, MAX_CLIENT_COUNT);
    umask(um);
    if (server->pollitem.fd < 0) {
        rc = -errno;
        ERROR("create server socket %s: %d %s", socket_spec, -rc, strerror(-rc));
        goto error2;
    }

    /* add the socket server to pollfd */
    server->pollitem.handler = on_server_event;
    server->pollitem.closure = server;
    rc = pollitem_add(&server->pollitem, EPOLLIN, server->pollfd);
    if (rc >= 0) {
        *pserver = server;
        return 0;
    }

    rc = -errno;
    ERROR("pollitem_add socket: %d %s", -rc, strerror(-rc));
    close(server->pollitem.fd);
error2:
    close(server->pollfd);
error:
    free(server);
ret:
    *pserver = NULL;
    return rc;
}

/* see sec-lsm-manager-server.h */
void sec_lsm_manager_server_stop(sec_lsm_manager_server_t *server, int status) {
    if (!server->stopped) {
        server->stopped = true;
        server->stoprc = status;
    }
}

/* see sec-lsm-manager-server.h */
__wur int sec_lsm_manager_server_serve(sec_lsm_manager_server_t *server, int shutofftime) {
    int tempo = shutofftime < 0 ? -1 : shutofftime > INT_MAX / 1000 ? INT_MAX : shutofftime * 1000;
    /* process inputs */
    server->stoprc = 0;
    server->stopped = false;
    while (!server->stopped) {
        int rc = pollitem_wait_dispatch(server->pollfd, tempo);
        if (rc < 0 && errno != EINTR) {
            ERROR("when dispatching %d: %s", errno, strerror(errno));
            sec_lsm_manager_server_stop(server, rc);
        }
        else {
            time_t trig = shutofftime < 0 ? 0 : time(NULL) - shutofftime;
            int idxcli = 0;
            while (idxcli < server->client_count) {
                client_t *client = server->clients[idxcli];
                client_disconnect_older(client, trig);
                if (client_is_connected(client))
                    idxcli++;
                else {
                    server->client_count--;
                    server->clients[idxcli] = server->clients[server->client_count];
                    client_destroy(client);
                }
            }
            if (rc == 0 && server->client_count == 0 && shutofftime >= 0)
                sec_lsm_manager_server_stop(server, 0);
            else if (server->deaf && server->client_count < MAX_CLIENT_COUNT) {
                rc = pollitem_mod(&server->pollitem, EPOLLIN, server->pollfd);
                if (rc < 0) {
                    ERROR("unexpected server socket error");
                    sec_lsm_manager_server_stop(server, rc);
                }
            }
        }
    }
    return server->stoprc;
}

