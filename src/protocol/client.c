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

#include "client.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <poll.h>
#include <sys/epoll.h>

#include "action/action.h"
#include "context/context.h"
#include "log.h"
#include "pollitem.h"
#include "prot.h"
#include "sec-lsm-manager-protocol.h"
#include "utf8-utils.h"

#define VERSION_BITS              3
#define _STR_(x)                  #x
#define PROTOCOL_STRING(x)        _STR_(x)
#define PROTOCOL_VERSION_1        1
#define PROTOCOL_VERSION_1_STRING PROTOCOL_STRING(PROTOCOL_VERSION_1)
#define DEFAULT_PROTOCOL_VERSION  PROTOCOL_VERSION_1

#define MAX_PUTX_ITEMS            15

extern bool sec_lsm_manager_server_log;

/** structure that represents a client */
struct client
{
    /** a protocol structure */
    prot_t *prot;

    /** context used by the client */
    context_t *context;

    /** the version of the protocol used */
    unsigned version: VERSION_BITS;

    /** is the actual link invalid or valid */
    unsigned invalid: 1;

    /** pollfd */
    int pollfd;

    /** polling callback */
    pollitem_t pollitem;

    /** last query time */
    time_t lasttime;
};

static int display_id(void *client, const char *id);
static int display_path(void *client, const char *path, const char *type);
static int display_permission(void *client, const char *permission);
static int display_plug(void *client, const char *expdir, const char *impid, const char *impdir);

/** should log? */
bool client_log = 0;

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
 * @param[in] client the client handle
 * @param[in] c2s direction: if not 0: client to server, if 0: server to client
 * @param[in] count count of fields
 * @param[in] fields the fields
 */
__nonnull((1))
static void dolog_protocol(client_t *client, int c2s, unsigned count, const char *fields[])
{
    static const char dir[2] = {'>', '<'};
    unsigned i;

    if (sec_lsm_manager_server_log) {
        fprintf(stderr, "%p%c%c%s", (void*)client, dir[!c2s], dir[!c2s], "server");
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
 * @param[in] client client handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull()
static int flushw(client_t *client)
{
    int rc;
    struct pollfd pfd;

    for (;;) {
        rc = prot_should_write(client->prot);
        if (rc < 0)
            ERROR("flushw: should write returned error %s", strerror(-rc));
        else if (rc > 0) {
            rc = prot_write(client->prot, client->pollitem.fd);
            if (rc >= 0)
                continue;
            if (rc == -ENODATA)
                rc = 0;
            else if (rc != -EAGAIN)
                ERROR("flushw: write returned error %s", strerror(-rc));
            else {
                pfd.fd = client->pollitem.fd;
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
 * @param[in] client client handler
 * @param[in] ... strings to send or NULL
 * @return 0 in case of success or a negative -errno value
 */
__nonnull((1))
static int putx(client_t *client, ...) {
    const char *p, *fields[MAX_PUTX_ITEMS];
    unsigned n;
    va_list l;
    int rc;

    /* store temporary in fields */
    n = 0;
    va_start(l, client);
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

    dolog_protocol(client, 0, n, fields);

    /* send now */
    rc = prot_put(client->prot, n, fields);
    if (rc == -ECANCELED) {
        rc = flushw(client);
        if (rc == 0) {
            rc = prot_put(client->prot, n, fields);
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
 * @param[in] client client handler
 */
__nonnull((1))
static void send_done(client_t *client, const char *arg) {
    putx(client, _done_, arg, NULL);
    flushw(client);
}

/**
 * @brief emit a simple error reply and flush
 *
 * @param[in] client client handler
 * @param[in] errorstr string error to send
 */
__nonnull((1))
static void send_error(client_t *client, const char *errorstr) {
    context_raise_error(client->context);
    putx(client, _error_, errorstr, NULL);
    flushw(client);
}

/* visitor's id callback for displaying context */
static int display_id(void *client, const char *id)
{
    return putx((client_t*)client, _string_, _id_, id, NULL);
}

/* visitor's path callback for displaying context */
static int display_path(void *client, const char *path, const char *type)
{
    return putx((client_t*)client, _string_, _path_, path, type, NULL);
}

/* visitor's permission callback for displaying context */
static int display_permission(void *client, const char *permission)
{
    return putx((client_t*)client, _string_, _permission_, permission, NULL);
}

/* visitor's plug callback for displaying context */
static int display_plug(void *client, const char *expdir, const char *impid, const char *impdir)
{
    return putx((client_t*)client, _string_, _plug_, expdir, impid, impdir, NULL);
}

/**
 * @brief emit the reply to a display query
 *
 * @param[in] client client handler
 */
__nonnull() __wur
static int send_display(client_t *client) {

    int rc = 0;

    if (context_has_error(client->context))
        rc = putx(client, _string_, _error_, _on_, NULL);

    if (rc == 0)
        rc = context_visit(client->context, client, &display_visitor_itf);

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
 * @param[in] client client handler
 * @param[in] count The number or arguments
 * @param[in] args Arguments
 */
__nonnull((1)) static void onrequest(client_t *client, unsigned count, const char *args[]) {
    int nextlog, rc;
    const char *errtxt;

    /* just ignore empty lines */
    if (count == 0)
        return;

    /* emit the log */
    dolog_protocol(client, 1, count, args);

    /* check utf8 validity */
    if (!check_utf8(count, args))
        goto invalid_protocol;

    /* version hand-shake */
    if (client->version == 0) {
        if (ckarg(args[0], _sec_lsm_manager_, 0)) {
            unsigned version = 0, idx = count;
            while (version == 0) {
                if (--idx == 0)
                    goto invalid_protocol;
                if (ckarg(args[idx], PROTOCOL_VERSION_1_STRING, 0))
                    version = PROTOCOL_VERSION_1;
            }
            client->version = version & ((1 << VERSION_BITS) - 1);
            send_done(client, args[idx]);
            return;
        }
        /* switch automatically to version 1 */
        client->version = DEFAULT_PROTOCOL_VERSION;
    }

    switch (args[0][0]) {
        case 'c':
            /* clear */
            if (ckarg(args[0], _clear_, 1) && count == 1) {
                context_clear(client->context);
                send_done(client, NULL);
                return;
            }
            break;
        case 'd':
            /* display */
            if (ckarg(args[0], _display_, 1) && count == 1) {
                rc = send_display(client);
                if (rc >= 0) {
                    send_done(client, NULL);
                } else {
                    send_error(client, "internal");
                    ERROR("send_display_context: %d %s", -rc, strerror(-rc));
                }
                return;
            }
            break;
        case 'i':
            /* id */
            if (ckarg(args[0], _id_, 1) && count == 2) {
                rc = context_set_id(client->context, args[1]);
                if (rc >= 0) {
                    send_done(client, NULL);
                } else {
                    switch (-rc) {
                    case ENOTRECOVERABLE: errtxt = "not-recoverable"; break;
                    case EINVAL:       errtxt = "invalid"; break;
                    case EEXIST:       errtxt = "already-set"; break;
                    default:           errtxt = "internal"; break;
                    }
                    send_error(client, errtxt);
                    ERROR("sec_lsm_manager_handle_set_id: %s", errtxt);
                }
                return;
            }
            /* install */
            if (ckarg(args[0], _install_, 1) && count == 1) {
                rc = action_install(client->context);
                if (rc >= 0) {
                    send_done(client, NULL);
                } else {
                    switch (-rc) {
                    case ENOTRECOVERABLE: errtxt = "not-recoverable"; break;
                    case EINVAL:       errtxt = "invalid"; break;
                    case EPERM:        errtxt = "forbidden"; break;
                    default:           errtxt = "internal"; break;
                    }
                    send_error(client, errtxt);
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
                send_done(client, nextlog ? _on_ : _off_);
                sec_lsm_manager_server_log = nextlog;
                return;
            }
            break;
        case 'p':
            /* path */
            if (ckarg(args[0], _path_, 1) && count == 3) {
                rc = context_add_path(client->context, args[1], args[2]);
                if (rc >= 0) {
                    send_done(client, NULL);
                } else {
                    switch (-rc) {
                    case ENOTRECOVERABLE: errtxt = "not-recoverable"; break;
                    case EINVAL:       errtxt = "invalid"; break;
                    case EEXIST:       errtxt = "already-set"; break;
                    case ENOENT:       errtxt = "not-found"; break;
                    case EACCES:       errtxt = "no-access"; break;
                    default:           errtxt = "internal"; break;
                    }
                    send_error(client, errtxt);
                    ERROR("error when adding path: %s", errtxt);
                }
                return;
            }
            /* permission */
            if (ckarg(args[0], _permission_, 1) && count == 2) {
                rc = context_add_permission(client->context, args[1]);
                if (rc >= 0) {
                    send_done(client, NULL);
                } else {
                    switch (-rc) {
                    case ENOTRECOVERABLE: errtxt = "not-recoverable"; break;
                    case EINVAL:       errtxt = "invalid"; break;
                    case EEXIST:       errtxt = "already-set"; break;
                    default:           errtxt = "internal"; break;
                    }
                    send_error(client, errtxt);
                    ERROR("error when adding permission: %s", errtxt);
                }
                return;
            }
            /* plug */
            if (ckarg(args[0], _plug_, 1) && count == 4) {
                rc = context_add_plug(client->context, args[1], args[2], args[3]);
                if (rc >= 0) {
                    send_done(client, NULL);
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
                    send_error(client, errtxt);
                    ERROR("error when adding plug: %s", errtxt);
                }
                return;
            }
            break;
        case 'u':
            /* uninstall */
            if (ckarg(args[0], _uninstall_, 1) && count == 1) {
                rc = action_uninstall(client->context);
                if (rc >= 0) {
                    send_done(client, NULL);
                } else {
                    switch (-rc) {
                    case ENOTRECOVERABLE: errtxt = "not-recoverable"; break;
                    case EINVAL:       errtxt = "invalid"; break;
                    default:           errtxt = "internal"; break;
                    }
                    send_error(client, errtxt);
                    ERROR("sec_lsm_manager_handle_uninstall: %s", errtxt);
                }
                return;
            }
            break;
        default:
            break;
    }
invalid_protocol:
    send_error(client, "protocol");
    client->invalid = 1;
}

/**
 * @brief disconnect the client (that shall be connected)
 * @param[in] client client handler
 */
__nonnull()
static void disconnect(client_t *client)
{
    pollitem_del(&client->pollitem, client->pollfd);
    close(client->pollitem.fd);
    client->pollfd = client->pollitem.fd = -1;
}


/**
 * @brief handle client requests
 *
 * @param[in] pollitem pollitem of requests
 * @param[in] events events receive
 * @param[in] pollfd pollfd of the client
 */
static void on_client_event(pollitem_t *pollitem, uint32_t events, int pollfd)
{
    (void) pollfd; /* make compiler happy even when -Wunused-parameter */

    int nargs, nr;
    const char **args;
    client_t *client = pollitem->closure;

    /* is it incoming data ? */
    if ((events & EPOLLHUP) == 0 && (events & EPOLLIN) != 0) {

        /* read the incoming data */
        nr = prot_read(client->prot, client->pollitem.fd);
        if (nr > 0) {
            /* yes, something to process */
            client->lasttime = time(NULL);
            for (;;) {
                nargs = prot_get(client->prot, &args);
                if (nargs <= 0) {
                    if (nargs != -EMSGSIZE)
                        return;
                    break;
                }
                onrequest(client, (unsigned)nargs, args);
                if (client->invalid)
                    break;
                prot_next(client->prot);
            }
        }
    }
    disconnect(client);
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see client.h */
__wur __nonnull()
int client_create(client_t **pclient, int fd, int pollfd)
{
    client_t *client;
    int rc;

    /* check parameters */
    if (fd < 0 || pollfd < 0) {
        rc = -EINVAL;
        goto error;
    }

    /* allocate the object */
    client = calloc(1, sizeof(*client));
    if (client == NULL) {
        rc = -ENOMEM;
        goto error;
    }

    /* create protocol object */
    rc = prot_create(&(client->prot));
    if (rc < 0) {
        ERROR("prot_create: %d %s", -rc, strerror(-rc));
        goto error1;
    }

    /* create the context object */
    rc = context_create(&(client->context));
    if (rc < 0) {
        ERROR("context_create %d %s", -rc, strerror(-rc));
        goto error2;
    }

    /* init other fields */
    client->version = 0; /* version not set */
    client->invalid = 0; /* not invalid */
    client->pollfd = pollfd;
    client->pollitem.handler = on_client_event;
    client->pollitem.closure = client;
    client->pollitem.fd = fd;
    client->lasttime = time(NULL);

    /* connect the client to polling */
    rc = pollitem_add(&client->pollitem, EPOLLIN, pollfd);
    if (rc < 0) {
        ERROR("can't poll client connection: %d %s", -rc, strerror(-rc));
        goto error3;
    }

    /* link the ready client */
    *pclient = client;
    return 0;

error3:
    context_destroy(client->context);
error2:
    prot_destroy(client->prot);
error1:
    free(client);
error:
    *pclient = NULL;
    return rc;
}

/* see client.h */
__wur __nonnull()
bool client_is_connected(client_t *client)
{
    return client->pollitem.fd >= 0;
}

/* see client.h */
__nonnull()
void client_disconnect(client_t *client)
{
    if (client_is_connected(client))
        disconnect(client);
}

/* see client.h */
__nonnull()
void client_disconnect_older(client_t *client, time_t trigger)
{
    if (client_is_connected(client) && client->lasttime <= trigger)
        disconnect(client);
}

/* see client.h */
__nonnull()
void client_destroy(client_t *client)
{
    client_disconnect(client);
    prot_destroy(client->prot);
    context_destroy(client->context);
    free(client);
}

