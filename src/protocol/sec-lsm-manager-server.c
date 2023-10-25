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

struct client
{
    /** the client */
    client_t *client;

    /** pollfd */
    int pollfd;

    /** polling callback */
    pollitem_t pollitem;

    /** last query time */
    time_t lasttime;
};

/** structure for servers */
struct sec_lsm_manager_server {
    /** the pollfd to use */
    int pollfd;

    /** stop status */
    int stoprc;

    /** is stopped ? */
    bool stopped;

    /** is deaf ? */
    bool deaf;

    /** the server socket */
    pollitem_t pollitem;

    /** the clients */
    struct client clients[MAX_CLIENT_COUNT];
};

/**
 * @brief handle client requests
 *
 * @param[in] pollitem pollitem of requests
 * @param[in] events events receive
 * @param[in] pollfd pollfd of the client
 */
static void on_client_event(pollitem_t *pollitem, uint32_t events, int pollfd)
{
    bool keep;
    struct client *client = pollitem->closure;

    if ((events & EPOLLHUP) != 0)
        keep = false;
    else {
        int rc = client_process_input(client->client);
        keep = rc > 0 || rc == -EAGAIN;
    }
    if (keep)
        client->lasttime = time(NULL);
    else {
        pollitem_del(&client->pollitem, pollfd);
        client_destroy(client->client);
        client->client = NULL;
    }
}

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
    int fd, rc, idx;
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

    /* search a slot */
    for(idx = 0 ; idx < MAX_CLIENT_COUNT ; idx++)
        if (server->clients[idx].client == NULL)
            break;

    if (idx < MAX_CLIENT_COUNT) {
        struct client *client = &server->clients[idx];

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
        rc = client_create(&client->client, fd, fd);
        if (rc < 0) {
            ERROR("can't create client connection: %d %s", -rc, strerror(-rc));
            close(fd);
            return;
        }

        /* set pollitem */
        client->lasttime = time(NULL);
        client->pollitem.handler = on_client_event;
        client->pollitem.closure = client;
        client->pollitem.fd = fd;

        /* connect the client to polling */
        rc = pollitem_add(&client->pollitem, EPOLLIN, pollfd);
        if (rc < 0) {
            ERROR("can't poll client connection: %d %s", -rc, strerror(-rc));
            client_destroy(client->client);
            client->client = NULL;
            return;
        }
        DEBUG("starting new client connection");

        while(++idx < MAX_CLIENT_COUNT)
            if (server->clients[idx].client == NULL)
                return;
    }

    /* if full avoid accepting new clients */
    rc = pollitem_mod(&server->pollitem, 0, server->pollfd);
    if (rc < 0) {
        ERROR("unexpected server socket error");
        sec_lsm_manager_server_stop(server, rc);
    }
    server->deaf = true;
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see sec-lsm-manager-server.h */
void sec_lsm_manager_server_destroy(sec_lsm_manager_server_t *server)
{
    int idx;
    pollitem_del(&server->pollitem, server->pollfd);
    for (idx = 0 ; idx < MAX_CLIENT_COUNT ; idx++) {
        if (server->clients[idx].client) {
            pollitem_del(&server->clients[idx].pollitem, server->pollfd);
            client_destroy(server->clients[idx].client);
        }
    }
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
            int idx, ncli;
            for (idx = ncli = 0 ; idx < MAX_CLIENT_COUNT ; idx ++) {
                struct client *client = &server->clients[idx];
                if (client->client != NULL) {
                    if (client->lasttime <= trig)
                        client_disconnect(client->client);
                    if (client_is_connected(client->client))
                        ncli++;
                    else {
                        pollitem_del(&client->pollitem, server->pollfd);
                        client_destroy(client->client);
                        client->client = NULL;
                    }
                }
            }
            if (rc == 0 && ncli == 0 && shutofftime >= 0)
                sec_lsm_manager_server_stop(server, 0);
            else if (server->deaf && ncli < MAX_CLIENT_COUNT) {
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

