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

#include <errno.h>
#include <fcntl.h>
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
#include "sec-lsm-manager-protocol.h"
#include "action/action.h"
#include "context/context.h"
#include "socket.h"
#include "utf8-utils.h"

typedef struct client client_t;

#define VERSION_BITS              3
#define _STR_(x)                  #x
#define PROTOCOL_STRING(x)        _STR_(x)
#define PROTOCOL_VERSION_1        1
#define PROTOCOL_VERSION_1_STRING PROTOCOL_STRING(PROTOCOL_VERSION_1)
#define DEFAULT_PROTOCOL_VERSION  PROTOCOL_VERSION_1

#define MAX_PUTX_ITEMS            15

/** should log? */
int sec_lsm_manager_server_log = 0;

/** structure that represents a client */
struct client {
    /** a protocol structure */
    prot_t *prot;

    /** context used by the client */
    context_t *context;

    /** the version of the protocol used */
    unsigned version: VERSION_BITS;

    /** is relaxed version of the protocol */
    unsigned relax: 1;

    /** is the actual link invalid or valid */
    unsigned invalid: 1;

    /** polling callback */
    pollitem_t pollitem;

    /** server of the client */
    sec_lsm_manager_server_t *sec_lsm_manager_server;
};

/** structure for servers */
struct sec_lsm_manager_server {
    /** the pollfd to use */
    int pollfd;

    /** number of client */
    int count;

    /** is stopped ? */
    int stopped;

    /** the server socket */
    pollitem_t socket;
};

static int display_id(void *cli, const char *id);
static int display_path(void *cli, const char *path, const char *type);
static int display_permission(void *cli, const char *permission);
static int display_plug(void *cli, const char *expdir, const char *impid, const char *impdir);

/** visitor interface for displaying context */
static const context_visitor_itf_t display_visitor_itf = {
    .id = display_id,
    .path = display_path,
    .permission = display_permission,
    .plug = display_plug
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
__nonnull((1))
static void dolog_protocol(client_t *cli, int c2s, unsigned count, const char *fields[])
{
    static const char dir[2] = {'>', '<'};
    unsigned i;

    if (sec_lsm_manager_server_log) {
        fprintf(stderr, "%p%c%c%s", (void*)cli, dir[!c2s], dir[!c2s], "server");
        for (i = 0; i < count; i++)
            fprintf(stderr, " %s", fields[i]);
        fprintf(stderr, "\n");
    }
}

/**
 * @brief Check 'arg' against 'value' beginning at offset accepting it if 'arg' equals 'value'
 *
 * @param[in] arg argument to compare
 * @param[in] value value to compare
 * @param[in] offset offset where begin
 * @return true if matching
 * @return false if not
 */
__nonnull() __wur
static bool ckarg(const char *arg, const char *value, unsigned offset)
{
    while (arg[offset])
        if (arg[offset] == value[offset])
            offset++;
        else
            return false;
    return !value[offset];
}

/**
 * @brief Flush the write buffer
 *
 * @param[in] cli client handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull()
static int flushw(client_t *cli)
{
    int rc;
    struct pollfd pfd;

    for (;;) {
        rc = prot_should_write(cli->prot);
        if (rc < 0)
            ERROR("flushw: should write returned error %s", strerror(-rc));
        else if (rc > 0) {
            rc = prot_write(cli->prot, cli->pollitem.fd);
            if (rc >= 0)
                continue;
            if (rc == -ENODATA)
                rc = 0;
            else if (rc != -EAGAIN)
                ERROR("flushw: write returned error %s", strerror(-rc));
            else {
                pfd.fd = cli->pollitem.fd;
                pfd.events = POLLOUT;
                do {
                    rc = poll(&pfd, 1, 0);
                } while (rc < 0 && errno == EINTR);
                if (rc >= 0)
                    continue;
                rc = -errno;
                ERROR("flushw: poll returned error %s", strerror(-rc));
            }
        }
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
__nonnull((1)) static int putx(client_t *cli, ...) {
    const char *p, *fields[MAX_PUTX_ITEMS];
    unsigned n;
    va_list l;
    int rc;

    /* store temporary in fields */
    n = 0;
    va_start(l, cli);
    p = va_arg(l, const char *);
    while (p) {
        if (n == MAX_PUTX_ITEMS) {
            ERROR("putx: Unexpected big count of args");
            return -EINVAL;
        }
        fields[n++] = p;
        p = va_arg(l, const char *);
    }
    va_end(l);

    dolog_protocol(cli, 0, n, fields);

    /* send now */
    rc = prot_put(cli->prot, n, fields);
    if (rc == -ECANCELED) {
        rc = flushw(cli);
        if (rc == 0) {
            rc = prot_put(cli->prot, n, fields);
            if (rc < 0)
                goto put_error;
        }
    }
    else if (rc < 0) {
put_error:
        ERROR("putx: prot_put returned the error %s", strerror(-rc));
    }
    return rc;
}

/**
 * @brief emit a simple done reply and flush
 *
 * @param[in] cli client handler
 */
__nonnull((1)) static void send_done(client_t *cli, const char *arg) {
    putx(cli, _done_, arg, NULL);
    flushw(cli);
}

/**
 * @brief emit a simple error reply and flush
 *
 * @param[in] cli client handler
 * @param[in] errorstr string error to send
 */
__nonnull((1)) static void send_error(client_t *cli, const char *errorstr) {
    context_raise_error(cli->context);
    putx(cli, _error_, errorstr, NULL);
    flushw(cli);
}

/* visitor's id callback for displaying context */
static int display_id(void *cli, const char *id)
{
    return putx((client_t*)cli, _string_, _id_, id, NULL);
}

/* visitor's path callback for displaying context */
static int display_path(void *cli, const char *path, const char *type)
{
    return putx((client_t*)cli, _string_, _path_, path, type, NULL);
}

/* visitor's permission callback for displaying context */
static int display_permission(void *cli, const char *permission)
{
    return putx((client_t*)cli, _string_, _permission_, permission, NULL);
}

/* visitor's plug callback for displaying context */
static int display_plug(void *cli, const char *expdir, const char *impid, const char *impdir)
{
    return putx((client_t*)cli, _string_, _plug_, expdir, impid, impdir, NULL);
}

/**
 * @brief emit the reply to a display query
 *
 * @param[in] cli client handler
 */
__nonnull() __wur
static int send_display(client_t *cli) {

    int rc = 0;
    
    if (context_has_error(cli->context))
        rc = putx(cli, _string_, _error_, _on_, NULL);

    if (rc == 0)
        rc = context_visit(cli->context, cli, &display_visitor_itf);

    return rc;
}

/**
 * @brief checks utf8 validity of received fields
 *
 * @param[in] count The number or arguments
 * @param[in] args Arguments
 * @return true when all are valid utf8, false otherwise
 */
__nonnull() __wur
static bool check_utf8(unsigned count, const char *args[])
{
    while(count)
        if (!is_utf8(args[--count]))
            return false;
    return true;
}

/**
 * @brief handle a request
 *
 * @param[in] cli client handler
 * @param[in] count The number or arguments
 * @param[in] args Arguments
 */
__nonnull((1)) static void onrequest(client_t *cli, unsigned count, const char *args[]) {
    int nextlog, rc;
    const char *errtxt;

    /* just ignore empty lines */
    if (count == 0)
        return;

    /* emit the log */
    dolog_protocol(cli, 1, count, args);

    /* check utf8 validity */
    if (!check_utf8(count, args))
        goto invalid_protocol;

    /* version hand-shake */
    if (cli->version == 0) {
        if (ckarg(args[0], _sec_lsm_manager_, 0)) {
            unsigned version = 0, idx = count;
            while (version == 0) {
                if (--idx == 0) {
                    cli->relax = 0; /* not relax on protocol mismatch */
                    goto invalid_protocol;
                }
                if (ckarg(args[idx], PROTOCOL_VERSION_1_STRING, 0))
                    version = PROTOCOL_VERSION_1;
            }
            cli->version = version & ((1 << VERSION_BITS) - 1);
            send_done(cli, args[idx]);
            return;
        }
        /* switch automatically to version 1 */
        cli->version = DEFAULT_PROTOCOL_VERSION;
    }

    switch (args[0][0]) {
        case 'c':
            /* clear */
            if (ckarg(args[0], _clear_, 1) && count == 1) {
                clear_context(cli->context);
                send_done(cli, NULL);
                return;
            }
            break;
        case 'd':
            /* display */
            if (ckarg(args[0], _display_, 1) && count == 1) {
                rc = send_display(cli);
                if (rc >= 0) {
                    send_done(cli, NULL);
                } else {
                    send_error(cli, "internal");
                    ERROR("send_display_context: %d %s", -rc, strerror(-rc));
                }
                return;
            }
            break;
        case 'i':
            /* id */
            if (ckarg(args[0], _id_, 1) && count == 2) {
                rc = context_set_id(cli->context, args[1]);
                if (rc >= 0) {
                    send_done(cli, NULL);
                } else {
                    switch (-rc) {
                    case ENOTRECOVERABLE: errtxt = "not-recoverable"; break;
                    case EINVAL:       errtxt = "invalid"; break;
                    case EEXIST:       errtxt = "already-set"; break;
                    default:           errtxt = "internal"; break;
                    }
                    send_error(cli, errtxt);
                    ERROR("sec_lsm_manager_handle_set_id: %s", errtxt);
                }
                return;
            }
            /* install */
            if (ckarg(args[0], _install_, 1) && count == 1) {
                rc = action_install(cli->context);
                if (rc >= 0) {
                    send_done(cli, NULL);
                } else {
                    switch (-rc) {
                    case ENOTRECOVERABLE: errtxt = "not-recoverable"; break;
                    case EINVAL:       errtxt = "invalid"; break;
                    case EPERM:        errtxt = "forbidden"; break;
                    default:           errtxt = "internal"; break;
                    }
                    send_error(cli, errtxt);
                    ERROR("sec_lsm_manager_handle_install: %s", errtxt);
                }
                return;
            }
            break;
        case 'l':
            /* log */
            if (ckarg(args[0], _log_, 1) && count <= 2) {
                nextlog = sec_lsm_manager_server_log;
                if (count == 2) {
                    if (!ckarg(args[1], _on_, 0) && !ckarg(args[1], _off_, 0))
                        goto invalid_protocol;
                    nextlog = ckarg(args[1], _on_, 0);
                }
                send_done(cli, nextlog ? _on_ : _off_);
                sec_lsm_manager_server_log = nextlog;
                return;
            }
            break;
        case 'p':
            /* path */
            if (ckarg(args[0], _path_, 1) && count == 3) {
                rc = context_add_path(cli->context, args[1], args[2]);
                if (rc >= 0) {
                    send_done(cli, NULL);
                } else {
                    switch (-rc) {
                    case ENOTRECOVERABLE: errtxt = "not-recoverable"; break;
                    case EINVAL:       errtxt = "invalid"; break;
                    case EEXIST:       errtxt = "already-set"; break;
                    case ENOENT:       errtxt = "not-found"; break;
                    case EACCES:       errtxt = "no-access"; break;
                    default:           errtxt = "internal"; break;
                    }
                    send_error(cli, errtxt);
                    ERROR("error when adding path: %s", errtxt);
                }
                return;
            }
            /* permission */
            if (ckarg(args[0], _permission_, 1) && count == 2) {
                rc = context_add_permission(cli->context, args[1]);
                if (rc >= 0) {
                    send_done(cli, NULL);
                } else {
                    switch (-rc) {
                    case ENOTRECOVERABLE: errtxt = "not-recoverable"; break;
                    case EINVAL:       errtxt = "invalid"; break;
                    case EEXIST:       errtxt = "already-set"; break;
                    default:           errtxt = "internal"; break;
                    }
                    send_error(cli, errtxt);
                    ERROR("error when adding permission: %s", errtxt);
                }
                return;
            }
            /* plug */
            if (ckarg(args[0], _plug_, 1) && count == 4) {
                rc = context_add_plug(cli->context, args[1], args[2], args[3]);
                if (rc >= 0) {
                    send_done(cli, NULL);
                } else {
                    switch (-rc) {
                    case ENOTRECOVERABLE: errtxt = "not-recoverable"; break;
                    case EINVAL:       errtxt = "invalid"; break;
                    case EEXIST:       errtxt = "already-set"; break;
                    case ENOENT:       errtxt = "not-found"; break;
                    case EACCES:       errtxt = "no-access"; break;
                    case ENOTDIR:      errtxt = "not-dir"; break;
                    default:           errtxt = "internal"; break;
                    }
                    send_error(cli, errtxt);
                    ERROR("error when adding plug: %s", errtxt);
                }
                return;
            }
            break;
        case 'u':
            /* uninstall */
            if (ckarg(args[0], _uninstall_, 1) && count == 1) {
                rc = action_uninstall(cli->context);
                if (rc >= 0) {
                    send_done(cli, NULL);
                } else {
                    switch (-rc) {
                    case ENOTRECOVERABLE: errtxt = "not-recoverable"; break;
                    case EINVAL:       errtxt = "invalid"; break;
                    default:           errtxt = "internal"; break;
                    }
                    send_error(cli, errtxt);
                    ERROR("sec_lsm_manager_handle_uninstall: %s", errtxt);
                }
                return;
            }
            break;
        default:
            break;
    }
invalid_protocol:
    send_error(cli, "protocol");
    if (!cli->relax)
        cli->invalid = 1;
}

/**
 * @brief destroy a client
 *
 * @param[in] cli client handler
 * @param[in] closefds if true close pollitem fd
 */
__nonnull((1)) static void destroy_client(client_t *cli, bool closefds) {
    cli->sec_lsm_manager_server->count--;

    /* close protocol */
    if (closefds)
        close(cli->pollitem.fd);

    prot_destroy(cli->prot);
    destroy_context(cli->context);
    cli->context = NULL;
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
__wur static int create_client(client_t **pcli, int fd, sec_lsm_manager_server_t *server) {
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
        ERROR("prot_create: %d %s", -rc, strerror(-rc));
        goto error1;
    }

    rc = create_context(&((*pcli)->context));
    if (rc < 0) {
        ERROR("create_context %d %s", -rc, strerror(-rc));
        (*pcli)->context = NULL;
        goto error2;
    }

    /* records the file descriptor */
    (*pcli)->version = 0; /* version not set */
    (*pcli)->relax = 0;   /* not relax on error */
    (*pcli)->invalid = 0; /* not invalid */
    (*pcli)->pollitem.handler = on_client_event;
    (*pcli)->pollitem.closure = (*pcli);
    (*pcli)->pollitem.fd = fd;
    (*pcli)->sec_lsm_manager_server = server;

    server->count++;

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
    sec_lsm_manager_server_t *server = (sec_lsm_manager_server_t *)pollitem->closure;

    /* is it a hangup? it shouldn't! */
    if (events & EPOLLHUP) {
        ERROR("unexpected server socket closing");
        exit(2);
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
    rc = create_client(&cli, fd, server);
    if (rc < 0) {
        ERROR("can't create client connection: %d %s", -rc, strerror(-rc));
        close(fd);
        return;
    }

    /* add the client to the epolling */
    rc = pollitem_add(&cli->pollitem, EPOLLIN, pollfd);
    if (rc < 0) {
        ERROR("can't poll client connection: %d %s", -rc, strerror(-rc));
        destroy_client(cli, 1);
        return;
    }
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see sec-lsm-manager-server.h */
void sec_lsm_manager_server_destroy(sec_lsm_manager_server_t *server) {
    if (server->pollfd >= 0)
        close(server->pollfd);
    if (server->socket.fd >= 0)
        close(server->socket.fd);
    free(server);
    server = NULL;
}

/* see sec-lsm-manager-server.h */
__wur int sec_lsm_manager_server_create(sec_lsm_manager_server_t **server, const char *socket_spec) {
    DEBUG("sec_lsm_manager_server_create");
    mode_t um;
    int rc = 0;
    /* allocate the structure */
    *server = (sec_lsm_manager_server_t *)malloc(sizeof(sec_lsm_manager_server_t));
    if (*server == NULL) {
        ERROR("malloc failed");
        rc = -ENOMEM;
        goto ret;
    }
    memset(*server, 0, sizeof(sec_lsm_manager_server_t));

    /* create the polling fd */
    (*server)->socket.fd = -1;
    (*server)->pollfd = epoll_create1(EPOLL_CLOEXEC);
    if ((*server)->pollfd < 0) {
        rc = -errno;
        ERROR("create polling: %d %s", -rc, strerror(-rc));
        goto error;
    }

    /* create the admin server socket */
    socket_spec = sec_lsm_manager_get_socket(socket_spec);

    DEBUG("socket = %s", socket_spec);

    um = umask(017);
    (*server)->socket.fd = socket_open(socket_spec, 1);
    umask(um);
    if ((*server)->socket.fd < 0) {
        rc = -errno;
        ERROR("create server socket %s: %d %s", socket_spec, -rc, strerror(-rc));
        goto error;
    }

    /* add the socket server to pollfd */
    (*server)->socket.handler = on_server_event;
    (*server)->socket.closure = *server;
    rc = pollitem_add(&(*server)->socket, EPOLLIN, (*server)->pollfd);
    if (rc < 0) {
        rc = -errno;
        ERROR("pollitem_add socket: %d %s", -rc, strerror(-rc));
        goto error;
    }

    goto ret;

error:
    sec_lsm_manager_server_destroy(*server);
ret:
    return rc;
}

/* see sec-lsm-manager-server.h */
void sec_lsm_manager_server_stop(sec_lsm_manager_server_t *server, int status) {
    server->stopped = status != 0 ? status : INT_MIN;
}

/* see sec-lsm-manager-server.h */
__wur int sec_lsm_manager_server_serve(sec_lsm_manager_server_t *server, int shutofftime) {
    int rc, tempo = shutofftime < 0 ? -1 : shutofftime > INT_MAX / 1000 ? INT_MAX : shutofftime * 1000;
    /* process inputs */
    server->stopped = 0;
    while (!server->stopped) {
        rc = pollitem_wait_dispatch(server->pollfd, tempo);
        if ((rc < 0 && errno != EINTR)
         || (rc == 0 && server->count == 0))
            sec_lsm_manager_server_stop(server, rc);
    }
    return server->stopped == INT_MIN ? 0 : server->stopped;
}
