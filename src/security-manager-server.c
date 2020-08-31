/*
 * Copyright (C) 2018-2020 IoT.bzh Company
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

#include "security-manager-server.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "log.h"
#include "pollitem.h"
#include "prot.h"
#include "security-manager-operation.h"
#include "security-manager-protocol.h"
#include "socket.h"
#include "utils.h"

typedef struct client client_t;

#define MAX_PUTX_ITEMS 15

/** should log? */
int security_manager_server_log = 0;

/** structure that represents a client */
struct client {
    /** a protocol structure */
    prot_t *prot;

    security_manager_handle_t sm_handle;

    /** the version of the protocol used */
    unsigned version : 1;

    /** is relaxed version of the protocol */
    unsigned relax : 1;

    /** is the actual link invalid or valid */
    unsigned invalid : 1;

    /** polling callback */
    pollitem_t pollitem;
};

/** structure for servers */
struct security_manager_server {
    /** the pollfd to use */
    int pollfd;

    /** is stopped ? */
    int stopped;

    /** the server socket */
    pollitem_t socket;
};

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Log the protocol
 * @param[in] cli the client handle
 * @param[in] c2s direction: if not 0: client to server, if 0: server to client
 * @param[in] count count of fields
 * @param[in] fields the fields
 */
__nonnull((1)) static void dolog(client_t *cli, int c2s, unsigned count, const char *fields[]) {
    CHECK_NO_NULL_NO_RETURN(cli, "cli");

    static const char dir[2] = {'>', '<'};
    unsigned i;

    fprintf(stderr, "%p%c%c%s", cli, dir[!c2s], dir[!c2s], "server");
    for (i = 0; i < count; i++) fprintf(stderr, " %s", fields[i]);
    fprintf(stderr, "\n");
}

/**
 * @brief Check 'arg' against 'value' beginning at offset accepting it if 'arg' prefixes 'value'
 *
 * @param[in] arg argument to compare
 * @param[in] value value to compare
 * @param[in] offset offset where begin
 * @return true if matching
 * @return false if not
 */
__nonnull() static bool ckarg(const char *arg, const char *value, unsigned offset) __wur {
    CHECK_NO_NULL(arg, "arg");
    CHECK_NO_NULL(value, "value");

    while (arg[offset])
        if (arg[offset] == value[offset])
            offset++;
        else
            return false;
    return true;
}

/**
 * @brief Flush the write buffer
 *
 * @param[in] cli client handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() static int flushw(client_t *cli) __wur {
    CHECK_NO_NULL(cli, "cli");

    int rc;
    struct pollfd pfd;

    for (;;) {
        rc = prot_should_write(cli->prot);
        if (!rc)
            break;
        rc = prot_write(cli->prot, cli->pollitem.fd);
        if (rc == -EAGAIN) {
            pfd.fd = cli->pollitem.fd;
            pfd.events = POLLOUT;
            do {
                rc = poll(&pfd, 1, 0);
            } while (rc < 0 && errno == EINTR);
            if (rc < 0)
                rc = -errno;
        }
        if (rc < 0)
            break;
    }
    return rc;
}

/**
 * @brief Send a reply to client
 *
 * @param[in] cli client handler
 * @param[in] ... strings to send or NULL
 * @return 0 in case of success or a negative -errno value
 */
__nonnull((1)) static int putx(client_t *cli, ...) __wur {
    CHECK_NO_NULL(cli, "cli");

    const char *p, *fields[MAX_PUTX_ITEMS];
    unsigned n;
    va_list l;
    int rc;

    /* store temporary in fields */
    n = 0;
    va_start(l, cli);
    p = va_arg(l, const char *);
    while (p) {
        if (n == MAX_PUTX_ITEMS)
            return -EINVAL;
        fields[n++] = p;
        p = va_arg(l, const char *);
    }
    va_end(l);

    /* emit the log */
    if (security_manager_server_log)
        dolog(cli, 0, n, fields);

    /* send now */
    rc = prot_put(cli->prot, n, fields);
    if (rc == -ECANCELED) {
        rc = flushw(cli);
        if (rc == 0)
            rc = prot_put(cli->prot, n, fields);
    }
    return rc;
}

/**
 * @brief emit a simple done reply and flush
 *
 * @param[in] cli client handler
 */
__nonnull() static void send_done(client_t *cli) {
    CHECK_NO_NULL_NO_RETURN(cli, "cli");

    putx(cli, _done_, NULL);
    flushw(cli);
}

/**
 * @brief emit a simple error reply and flush
 *
 * @param[in] cli client handler
 * @param[in] errorstr string error to send
 */
__nonnull((1)) static void send_error(client_t *cli, const char *errorstr) {
    CHECK_NO_NULL_NO_RETURN(cli, "cli");
    putx(cli, _error_, errorstr, NULL);
    flushw(cli);
}

/**
 * @brief emit the content of secure app to display it
 *
 * @param[in] cli client handler
 */
static void send_display_security_manager_handle(client_t *cli) {
    if (cli->sm_handle.secure_app->id) {
        putx(cli, _string_, _id_, cli->sm_handle.secure_app->id, NULL);
    }

    for (size_t i = 0; i < cli->sm_handle.secure_app->paths.size; i++) {
        putx(cli, _string_, _path_, cli->sm_handle.secure_app->paths.paths[i].path,
             cli->sm_handle.secure_app->paths.paths[i].path_type, NULL);
    }

    for (size_t i = 0; i < cli->sm_handle.secure_app->policies.size; i++) {
        putx(cli, _string_, _permission_, cli->sm_handle.secure_app->policies.policies[i].k.permission, NULL);
    }
}

/**
 * @brief handle a request
 *
 * @param[in] cli client handler
 * @param[in] count The number or arguments
 * @param[in] args Arguments
 */
__nonnull((1)) static void onrequest(client_t *cli, unsigned count, const char *args[]) {
    CHECK_NO_NULL_NO_RETURN(cli, "cli");

    int nextlog, rc;

    /* just ignore empty lines */
    if (count == 0)
        return;

    /* emit the log */
    if (security_manager_server_log)
        dolog(cli, 1, count, args);

    /* version hand-shake */
    if (!cli->version) {
        if (ckarg(args[0], _security_manager_, 0)) {
            if (count < 2 || !ckarg(args[1], "1", 0)) {
                send_error(cli, "invalid");
                if (!cli->relax)
                    cli->invalid = 1;
                return;
            }
            putx(cli, _done_, "1", NULL);
            flushw(cli);
            cli->version = 1;
            return;
        }
        /* switch automatically to version 1 */
        cli->version = 1;
    }

    switch (args[0][0]) {
        case 'c':
            if (ckarg(args[0], _clean_, 1) && count == 1) {
                rc = security_manager_handle_clean(&(cli->sm_handle));
                if (rc == 0) {
                    send_done(cli);
                } else {
                    send_error(cli, "security_manager_handle_clean");
                }
                return;
            }
            break;
        case 'd':
            if (ckarg(args[0], _display_, 1) && count == 1) {
                send_display_security_manager_handle(cli);
                send_done(cli);
                return;
            }
            break;
        case 'i':
            if (ckarg(args[0], _id_, 1) && count == 2) {
                rc = security_manager_handle_set_id(&(cli->sm_handle), args[1]);
                if (rc == 0) {
                    send_done(cli);
                } else if (rc == 1) {
                    send_error(cli, "id already set");
                } else {
                    send_error(cli, "security_manager_handle_set_id");
                }
                return;
            }
            if (ckarg(args[0], _install_, 1) && count == 1) {
                rc = security_manager_handle_install(&(cli->sm_handle));
                if (rc >= 0) {
                    send_done(cli);
                } else {
                    send_error(cli, "security_manager_handle_install");
                }
                return;
            }
            break;
        case 'l':
            if (ckarg(args[0], _log_, 1) && count <= 2) {
                nextlog = security_manager_server_log;
                if (count == 2) {
                    if (!ckarg(args[1], _on_, 0) && !ckarg(args[1], _off_, 0))
                        break;
                    nextlog = ckarg(args[1], _on_, 0);
                }
                putx(cli, _done_, nextlog ? _on_ : _off_, NULL);
                flushw(cli);
                security_manager_server_log = nextlog;
                return;
            }
            break;
        case 'p':
            if (ckarg(args[0], _path_, 1) && count == 3) {
                rc = security_manager_handle_add_path(&(cli->sm_handle), args[1], get_path_type(args[2]));
                if (rc == 0) {
                    putx(cli, _done_, NULL);
                    flushw(cli);
                } else {
                    send_error(cli, "security_manager_handle_add_path");
                }
                return;
            }
            if (ckarg(args[0], _permission_, 1) && count == 2) {
                rc = security_manager_handle_add_permission(&(cli->sm_handle), args[1]);
                if (rc == 0) {
                    putx(cli, _done_, NULL);
                    flushw(cli);
                } else {
                    send_error(cli, "security_manager_handle_add_permission");
                }
                return;
            }
            break;
        case 'u':
            if (ckarg(args[0], _uninstall_, 1) && count == 1) {
                rc = security_manager_handle_uninstall(&(cli->sm_handle));
                if (rc == 0) {
                    send_done(cli);
                } else {
                    send_error(cli, "security_manager_handle_uninstall");
                }
                return;
            }
    }
    return;
}

/**
 * @brief destroy a client
 *
 * @param[in] cli client handler
 * @param[in] closefds if true close pollitem fd
 */
__nonnull((1)) static void destroy_client(client_t *cli, bool closefds) {
    CHECK_NO_NULL_NO_RETURN(cli, "cli");
    /* close protocol */
    if (closefds)
        close(cli->pollitem.fd);

    prot_destroy(cli->prot);
    free_security_manager_handle(&(cli->sm_handle));
    free(cli);
}

/**
 * @brief handle client requests
 *
 * @param[in] pollitem pollitem of requests
 * @param[in] events events receive
 * @param[in] pollfd pollfd of the client
 */
static void on_client_event(pollitem_t *pollitem, uint32_t events, int pollfd) {
    int nargs, nr;
    const char **args;
    client_t *cli = pollitem->closure;

    /* is it a hangup? */
    if (events & EPOLLHUP) {
        goto terminate;
    }

    /* possible input */
    if (events & EPOLLIN) {
        nr = prot_read(cli->prot, cli->pollitem.fd);
        if (nr <= 0) {
            goto terminate;
        }

        nargs = prot_get(cli->prot, &args);
        while (nargs >= 0) {
            onrequest(cli, (unsigned)nargs, args);
            if (cli->invalid && !cli->relax) {
                goto terminate;
            }
            prot_next(cli->prot);
            nargs = prot_get(cli->prot, &args);
        }
    }
    return;

    /* terminate the client session */
terminate:
    pollitem_del(&cli->pollitem, pollfd);
    destroy_client(cli, true);
}

/**
 * @brief Create a client object
 *
 * @param[out] pcli pointer to the handle of a client
 * @param[in] fd file descriptor of client
 * @return 0 in case of success or a negative -errno value
 */
static int create_client(client_t **pcli, int fd) {
    int rc = 0;

    /* allocate the object */
    *pcli = calloc(1, sizeof(**pcli));
    if (*pcli == NULL) {
        rc = -ENOMEM;
        goto ret;
    }

    /* create protocol object */
    rc = prot_create(&((*pcli)->prot));
    if (rc < 0) {
        goto error1;
    }

    rc = init_security_manager_handle(&((*pcli)->sm_handle));
    if (rc < 0) {
        goto error2;
    }

    /* records the file descriptor */
    (*pcli)->version = 0; /* version not set */
    (*pcli)->relax = 0;   /* relax on error */
    (*pcli)->invalid = 0; /* not invalid */
    (*pcli)->pollitem.handler = on_client_event;
    (*pcli)->pollitem.closure = (*pcli);
    (*pcli)->pollitem.fd = fd;

    goto ret;

error2:
    prot_destroy((*pcli)->prot);
error1:
    free(*pcli);
    *pcli = NULL;
ret:
    return rc;
}

/**
 * @brief handle server events
 *
 * @param[in] pollitem pollitem of requests
 * @param[in] events events receive
 * @param[in] pollfd pollfd of the server
 */
static void on_server_event(pollitem_t *pollitem, uint32_t events, int pollfd) {
    int servfd = pollitem->fd;
    int fd, rc;
    struct sockaddr saddr;
    socklen_t slen;
    client_t *cli;

    /* is it a hangup? it shouldn't! */
    if (events & EPOLLHUP) {
        fprintf(stderr, "unexpected server socket closing\n");
        exit(2);
    }

    /* EPOLLIN is the only expected event but asserting makes fear */
    if (!(events & EPOLLIN))
        return;

    /* accept the connection */
    slen = (socklen_t)sizeof(saddr);
    fd = accept(servfd, &saddr, &slen);
    if (fd < 0) {
        fprintf(stderr, "can't accept connection: %m\n");
        return;
    }
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    fcntl(fd, F_SETFL, O_NONBLOCK);

    /* create a client for the connection */
    rc = create_client(&cli, fd);
    if (rc < 0) {
        fprintf(stderr, "can't create client connection: %s\n", strerror(-rc));
        close(fd);
        return;
    }

    /* add the client to the epolling */
    rc = pollitem_add(&cli->pollitem, EPOLLIN, pollfd);
    if (rc < 0) {
        fprintf(stderr, "can't poll client connection: %s\n", strerror(-rc));
        destroy_client(cli, 1);
        return;
    }
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see security-manager-server.h */
void security_manager_server_destroy(security_manager_server_t *server) {
    if (server) {
        if (server->pollfd >= 0)
            close(server->pollfd);
        if (server->socket.fd >= 0)
            close(server->socket.fd);
        free(server);
        server = NULL;
    }
}

/* see security-manager-server.h */
int security_manager_server_create(security_manager_server_t **server, const char *socket_spec) __wur {
    LOG("security_manager_server_create");
    mode_t um;
    int rc = 0;
    /* allocate the structure */
    *server = (security_manager_server_t *)malloc(sizeof(security_manager_server_t));
    if (*server == NULL) {
        ERROR("malloc security_manager_server_t failed");
        rc = -ENOMEM;
        goto ret;
    }
    memset(*server, 0, sizeof(security_manager_server_t));

    /* create the polling fd */
    (*server)->socket.fd = -1;
    (*server)->pollfd = epoll_create1(EPOLL_CLOEXEC);
    if ((*server)->pollfd < 0) {
        rc = -errno;
        ERROR("create polling: %m")
        goto error;
    }

    /* create the admin server socket */
    socket_spec = security_manager_get_socket(socket_spec);

    LOG("socket = %s", socket_spec);

    um = umask(017);
    (*server)->socket.fd = socket_open(socket_spec, 1);
    umask(um);
    if ((*server)->socket.fd < 0) {
        rc = -errno;
        ERROR("create server socket %s: %m", socket_spec);
        goto error;
    }

    /* add the socket server to pollfd */
    (*server)->socket.handler = on_server_event;
    (*server)->socket.closure = *server;
    rc = pollitem_add(&(*server)->socket, EPOLLIN, (*server)->pollfd);
    if (rc < 0) {
        rc = -errno;
        ERROR("pollitem_add socket: %m");
        goto error;
    }
    goto ret;

error:
    security_manager_server_destroy(*server);
ret:
    return rc;
}

/* see security-manager-server.h */
void security_manager_server_stop(security_manager_server_t *server, int status) {
    CHECK_NO_NULL_NO_RETURN(server, "server");

    server->stopped = status ?: INT_MIN;
}

/* see security-manager-server.h */
int security_manager_server_serve(security_manager_server_t *server) __wur {
    CHECK_NO_NULL(server, "server");

    /* process inputs */
    server->stopped = 0;
    while (!server->stopped) {
        pollitem_wait_dispatch(server->pollfd, -1);
    }
    return server->stopped == INT_MIN ? 0 : server->stopped;
}
