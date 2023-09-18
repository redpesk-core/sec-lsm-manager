/*
 * Copyright (C) 2020-2023 IoT.bzh Company
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
 *
 *
 */

#include "sec-lsm-manager.h"

#include <errno.h>
#include <poll.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "prot.h"
#include "sec-lsm-manager-protocol.h"
#include "socket.h"

#define CHECK_NO_NULL(param, param_name)   \
    if (!param) {                          \
        ERROR("%s undefined", param_name); \
        return -EINVAL;                    \
    }

#define CHECK_NO_NULL_NO_RETURN(param, param_name) \
    if (!param) {                                  \
        ERROR("%s undefined", param_name);         \
        return;                                    \
    }

/**
 * structure recording a client
 */
struct sec_lsm_manager {
    /** file descriptor of the socket */
    int fd;

    /** synchronous lock */
    bool synclock;

    /** protocol manager object */
    prot_t *prot;

    /** copy of the reply */
    struct {
        /** count of fields of the reply */
        int count;

        /** fields (or fields) of the reply */
        const char **fields;
    } reply;

    /** spec of the socket */
    char *socketspec;
};

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Flush the write buffer of the client
 *
 * @param[in] sec_lsm_manager  the handler of the client
 *
 * @return  0 in case of success or a negative -errno value
 */
__nonnull() __wur static int flushw(sec_lsm_manager_t *sec_lsm_manager) {
    int rc;
    struct pollfd pfd;

    for (;;) {
        rc = prot_should_write(sec_lsm_manager->prot);
        if (!rc)
            break;
        rc = prot_write(sec_lsm_manager->prot, sec_lsm_manager->fd);
        if (rc == -EAGAIN) {
            pfd.fd = sec_lsm_manager->fd;
            pfd.events = POLLOUT;
            do {
                rc = poll(&pfd, 1, -1);
            } while (rc < 0 && errno == EINTR);
            if (rc < 0)
                rc = -errno;
        }
        if (rc < 0) {
            break;
        }
    }
    return rc;
}

/**
 * @brief Send a reply
 *
 * @param[in] sec_lsm_manager the client
 * @param[in] fields the fields to send
 * @param[in] count the count of fields
 * @return 0 on success or a negative error code
 */
__nonnull() __wur static int send_reply(sec_lsm_manager_t *sec_lsm_manager, const char **fields, int count) {
    int rc, trial, i;
    prot_t *prot;

    /* retrieves the protocol handler */
    prot = sec_lsm_manager->prot;
    trial = 0;
    for (;;) {
        /* fill the fields */
        for (i = rc = 0; i < count && rc == 0; i++)
	    rc = prot_put_field(prot, fields[i]);

        /* send if done */
        if (rc == 0) {
            rc = prot_put_end(prot);
            if (rc == 0) {
                rc = flushw(sec_lsm_manager);
                break;
            }
        }

        /* failed to fill protocol, cancel current composition  */
        prot_put_cancel(prot);

        /* fail if was last trial */
        if (trial)
            break;

        /* try to flush the output buffer */
        rc = flushw(sec_lsm_manager);
        if (rc)
            break;

        trial = 1;
    }
    return rc;
}

/**
 * @brief Put the command made of arguments ...
 * Increment the count of pending requests.
 *
 * @param[in] sec_lsm_manager  the handler of the client
 * @param[in] command   the command to send
 * @param[in] optarg    an optional argument or NULL
 * @param[in] ...       optional arguments or NULL
 *
 * @return  0 in case of success or a negative -errno value
 */
__nonnull((1, 2)) __wur
static int putxkv(sec_lsm_manager_t *sec_lsm_manager, const char *command, const char *optarg, ...) {
    int nf, rc;
    const char *fields[8] = {0};

    /* prepare fields */
    fields[0] = command;
    nf = 1;

    va_list va;
    va_start(va, optarg);

    fields[nf++] = optarg;

    while (fields[nf - 1] != NULL && nf < 7) {
        fields[nf++] = va_arg(va, const char *);
    }

    va_end(va);

    /* send now */
    rc = send_reply(sec_lsm_manager, fields, nf - 1);
    return rc;
}

/**
 * @brief Wait some input event
 *
 * @param[in] sec_lsm_manager  the handler of the client
 *
 * @return  0 in case of success or a negative -errno value
 */
__nonnull() __wur static int wait_input(sec_lsm_manager_t *sec_lsm_manager) {
    int rc;
    struct pollfd pfd;

    pfd.fd = sec_lsm_manager->fd;
    pfd.events = POLLIN;
    do {
        rc = poll(&pfd, 1, -1);
    } while (rc < 0 && errno == EINTR);
    return rc < 0 ? -errno : 0;
}

/**
 * @brief Get the next reply if any
 *
 * @param[in] sec_lsm_manager  the handler of the client
 *
 * @return  the count of field of the reply (can be 0)
 *          or -EAGAIN if there is no reply
 */
__nonnull() __wur static int get_reply(sec_lsm_manager_t *sec_lsm_manager) {
    prot_next(sec_lsm_manager->prot);
    int rc = prot_get(sec_lsm_manager->prot, &sec_lsm_manager->reply.fields);

    sec_lsm_manager->reply.count = rc;
    return rc;
}

/**
 * @brief Wait for a reply
 *
 * @param[in] sec_lsm_manager  the handler of the client
 * @param[in] block
 *
 * @return  the count of fields greater than 0 or a negative -errno value
 *          or -EAGAIN if nothing and block == false
 *          or -EPIPE if broken link
 */
__nonnull() __wur static int wait_reply(sec_lsm_manager_t *sec_lsm_manager, bool block) {
    for (;;) {
        /* get the next reply if any */
        int rc = get_reply(sec_lsm_manager);
        if (rc > 0)
            return rc;

        if (rc < 0) {
            /* wait for an answer */
            rc = prot_read(sec_lsm_manager->prot, sec_lsm_manager->fd);
            while (rc <= 0) {
                if (rc == 0)
                    return -(errno = EPIPE);
                if (rc == -EAGAIN && block)
                    rc = wait_input(sec_lsm_manager);
                if (rc < 0)
                    return rc;
                rc = prot_read(sec_lsm_manager->prot, sec_lsm_manager->fd);
            }
        }
    }
    return -1;
}

/**
 * @brief Wait for a reply
 *
 * @param[in] sec_lsm_manager  the handler of the client
 *
 * @return  0 in case of success or a negative -errno value
 */
__nonnull() __wur static int wait_any_reply(sec_lsm_manager_t *sec_lsm_manager) {
    for (;;) {
        int rc = wait_reply(sec_lsm_manager, true);
        if (rc < 0)
            return rc;
        if (rc > 0)
            return rc;
    }
}

/**
 * @brief Wait the reply "done" or "error"
 *
 * @param[in] sec_lsm_manager  the handler of the client
 *
 * @return  the count of fields in case of success or a negative -errno value
 *          -ECANCELED when received an error status
 */
__nonnull() __wur static int raw_wait_done_or_error(sec_lsm_manager_t *sec_lsm_manager) {
    int rc = wait_any_reply(sec_lsm_manager);

    if (rc > 0) {
        if (!strcmp(sec_lsm_manager->reply.fields[0], _done_)) {
            return rc;
        } else if (!strcmp(sec_lsm_manager->reply.fields[0], _error_)) {
            return -EPROTO;
        } else {
            return -ECANCELED;
        }
    }
    return rc;
}

/**
 * @brief Wait the reply "done" or "error"
 *
 * @param[in] sec_lsm_manager  the handler of the client
 *
 * @return  0 in case of success or a negative -errno value
 *          -ECANCELED when received an error status
 */
__nonnull() __wur static int wait_done_or_error(sec_lsm_manager_t *sec_lsm_manager) {
    int rc = raw_wait_done_or_error(sec_lsm_manager);
    return rc < 0 ? rc : 0;
}

/**
 * @brief Disconnect the client
 *
 * @param[in] sec_lsm_manager  the handler of the client
 */
__nonnull() static void disconnection(sec_lsm_manager_t *sec_lsm_manager) {
    if (sec_lsm_manager->fd >= 0) {
        close(sec_lsm_manager->fd);
        sec_lsm_manager->fd = -1;
    }
}

/**
 * @brief Connect the client
 *
 * @param[in] sec_lsm_manager  the handler of the client
 *
 * @return  0 in case of success or a negative -errno value
 */
__nonnull() __wur static int connection(sec_lsm_manager_t *sec_lsm_manager) {
    int rc;

    /* init the client */
    sec_lsm_manager->reply.count = -1;
    prot_reset(sec_lsm_manager->prot);
    sec_lsm_manager->fd = socket_open(sec_lsm_manager->socketspec, 0);
    if (sec_lsm_manager->fd < 0)
        return -errno;

    /* negociate the protocol */
    rc = putxkv(sec_lsm_manager, _sec_lsm_manager_, "1", NULL);
    if (rc >= 0) {
        rc = wait_any_reply(sec_lsm_manager);
        if (rc >= 0) {
            rc = -EPROTO;
            if (sec_lsm_manager->reply.count >= 2 && 0 == strcmp(sec_lsm_manager->reply.fields[0], _done_) &&
                0 == strcmp(sec_lsm_manager->reply.fields[1], "1")) {
                return 0;
            }
        }
    }
    disconnection(sec_lsm_manager);
    return rc;
}

/**
 * @brief Ensure the connection is opened
 *
 * @param[in] sec_lsm_manager  the handler of the client
 *
 * @return  0 in case of success or a negative -errno value
 */
__nonnull() __wur static int ensure_opened(sec_lsm_manager_t *sec_lsm_manager) {
    if (sec_lsm_manager->fd >= 0 && write(sec_lsm_manager->fd, NULL, 0) < 0)
        disconnection(sec_lsm_manager);
    return sec_lsm_manager->fd < 0 ? connection(sec_lsm_manager) : 0;
}

/**
 * @brief lock, open and send
 *
 */
static int sync_send_do(sec_lsm_manager_t *sec_lsm_manager, int (*aftersend)(sec_lsm_manager_t *), ...)
{
    va_list va;
    int nf, rc;
    const char *fields[8];

    CHECK_NO_NULL(sec_lsm_manager, "sec_lsm_manager");

    /* check lock */
    if (sec_lsm_manager->synclock)
        return -EBUSY;

    /* lock and open */
    sec_lsm_manager->synclock = true;
    rc = ensure_opened(sec_lsm_manager);
    if (rc >= 0) {

        /* fill the arguments */
        va_start(va, aftersend);
        for (nf = 0 ; nf < (int)(sizeof fields / sizeof *fields) ; nf++) {
            fields[nf] = va_arg(va, const char *);
            if (fields[nf] == NULL)
                break;
        }
        va_end(va);

        /* send the reply */
        rc = send_reply(sec_lsm_manager, fields, nf);
        if (rc >= 0) {
            rc = aftersend(sec_lsm_manager);
        }
    }
    sec_lsm_manager->synclock = false;

    return rc;
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see sec-lsm-manager.h */
int sec_lsm_manager_create(sec_lsm_manager_t **sec_lsm_manager, const char *socketspec) {
    socketspec = sec_lsm_manager_get_socket(socketspec);

    /* allocate the structure */
    *sec_lsm_manager = (sec_lsm_manager_t *)malloc(sizeof(sec_lsm_manager_t));
    if (*sec_lsm_manager == NULL) {
        return -ENOMEM;
    }
    memset(*sec_lsm_manager, 0, sizeof(sec_lsm_manager_t));

    /* socket spec */
    (*sec_lsm_manager)->socketspec = strdup(socketspec);
    if ((*sec_lsm_manager)->socketspec == NULL) {
        free(*sec_lsm_manager);
        *sec_lsm_manager = NULL;
        return -ENOMEM;
    }

    /* create a protocol object */
    int rc = prot_create(&(*sec_lsm_manager)->prot);
    if (rc < 0) {
        free(*sec_lsm_manager);
        *sec_lsm_manager = NULL;
        free((*sec_lsm_manager)->socketspec);
        (*sec_lsm_manager)->socketspec = NULL;
        return rc;
    }

    /* record type and weakly create cache */
    (*sec_lsm_manager)->synclock = false;

    /* lazy connection */
    (*sec_lsm_manager)->fd = -1;

    /* done */
    return 0;
}

/* see sec-lsm-manager.h */
void sec_lsm_manager_destroy(sec_lsm_manager_t *sec_lsm_manager) {
    CHECK_NO_NULL_NO_RETURN(sec_lsm_manager, "sec_lsm_manager");

    disconnection(sec_lsm_manager);
    if (sec_lsm_manager->prot)
        prot_destroy(sec_lsm_manager->prot);
    free(sec_lsm_manager->socketspec);
    sec_lsm_manager->socketspec = NULL;
    free(sec_lsm_manager);
    sec_lsm_manager = NULL;
}

/* see sec-lsm-manager.h */
void sec_lsm_manager_disconnect(sec_lsm_manager_t *sec_lsm_manager) {
    CHECK_NO_NULL_NO_RETURN(sec_lsm_manager, "sec_lsm_manager");

    disconnection(sec_lsm_manager);
}

/* see sec-lsm-manager.h */
int sec_lsm_manager_set_id(sec_lsm_manager_t *sec_lsm_manager, const char *id) {
    CHECK_NO_NULL(id, "id");

    return sync_send_do(sec_lsm_manager, wait_done_or_error, _id_, id, NULL);
}

/* see sec-lsm-manager.h */
int sec_lsm_manager_add_path(sec_lsm_manager_t *sec_lsm_manager, const char *path, const char *path_type) {
    CHECK_NO_NULL(path, "path");
    CHECK_NO_NULL(path_type, "path_type");

    return sync_send_do(sec_lsm_manager, wait_done_or_error, _path_, path, path_type, NULL);
}

/* see sec-lsm-manager.h */
__nonnull() __wur
int sec_lsm_manager_add_plug(sec_lsm_manager_t *sec_lsm_manager, const char *expdir, const char *impid, const char *impdir)
{
    CHECK_NO_NULL(expdir, "expdir");
    CHECK_NO_NULL(impid, "impid");
    CHECK_NO_NULL(impdir, "impdir");

    return sync_send_do(sec_lsm_manager, wait_done_or_error, _plug_, expdir, impid, impdir, NULL);
}

/* see sec-lsm-manager.h */
int sec_lsm_manager_add_permission(sec_lsm_manager_t *sec_lsm_manager, const char *permission) {
    CHECK_NO_NULL(permission, "permission");

    return sync_send_do(sec_lsm_manager, wait_done_or_error, _permission_, permission, NULL);
}

/* see sec-lsm-manager.h */
int sec_lsm_manager_clear(sec_lsm_manager_t *sec_lsm_manager) {
    return sync_send_do(sec_lsm_manager, wait_done_or_error, _clear_, NULL);
}

/* see sec-lsm-manager.h */
int sec_lsm_manager_install(sec_lsm_manager_t *sec_lsm_manager) {
    return sync_send_do(sec_lsm_manager, wait_done_or_error, _install_, NULL);
}

/* see sec-lsm-manager.h */
int sec_lsm_manager_uninstall(sec_lsm_manager_t *sec_lsm_manager) {
    return sync_send_do(sec_lsm_manager, wait_done_or_error, _uninstall_, NULL);
}

/**
 * @brief callback for processing log replies
 * @param sec_lsm_manager sec_lsm_manager client handler
 * @return 0 if replied "done off", 1 if replied "done on" or a negative -errno value
 */
static int wait_log_reply(sec_lsm_manager_t *sec_lsm_manager)
{
    int rc = raw_wait_done_or_error(sec_lsm_manager);
    if (rc > 0)
        rc = sec_lsm_manager->reply.count >= 2 && !strcmp(sec_lsm_manager->reply.fields[1], _on_);
    return rc;
}

/* see sec-lsm-manager.h */
int sec_lsm_manager_log(sec_lsm_manager_t *sec_lsm_manager, int on, int off) {
    return sync_send_do(sec_lsm_manager, wait_log_reply, _log_, off ? _off_ : on ? _on_ : NULL, NULL);
}

/**
 * @brief callback for processing display replies
 * @param sec_lsm_manager sec_lsm_manager client handler
 * @return 0 on success or a negative -errno value
 */
static int wait_display_replies(sec_lsm_manager_t *sec_lsm_manager)
{
    int rc = wait_reply(sec_lsm_manager, true);
    if (rc > 2 && !strcmp(sec_lsm_manager->reply.fields[0], _string_)) {
        puts("################## SECURE APP ##################");
        do {
            printf(&"%s %s %s %s\n"[15 - 3 * (rc > 5 ? 5 : rc)],
              sec_lsm_manager->reply.fields[1],
              sec_lsm_manager->reply.fields[2],
              rc > 3 ? sec_lsm_manager->reply.fields[3] : NULL,
              rc > 4 ? sec_lsm_manager->reply.fields[4] : NULL);
            rc = wait_reply(sec_lsm_manager, true);
        }
        while (rc > 2 && !strcmp(sec_lsm_manager->reply.fields[0], _string_));

        puts("################################################");
    }
    if (rc > 0 && !strcmp(sec_lsm_manager->reply.fields[0], _error_)) {
        ERROR("%s", sec_lsm_manager->reply.fields[1]);
	rc = -1;
    }
    return rc;
}

/* see sec-lsm-manager.h */
int sec_lsm_manager_display(sec_lsm_manager_t *sec_lsm_manager) {
    return sync_send_do(sec_lsm_manager, wait_display_replies, _display_, NULL);
}

/* see sec-lsm-manager.h */
int sec_lsm_manager_error_message(sec_lsm_manager_t *sec_lsm_manager, char **message)
{
    int idx;
    size_t size;
    char *text;

    /* check state */
    if (sec_lsm_manager->reply.count < 1
     || strcmp(sec_lsm_manager->reply.fields[0], _error_)) {
        *message = NULL;
        return -EINVAL;
    }

    /* compute size */
    size = 0;
    for (idx = 1 ; idx < sec_lsm_manager->reply.count ; idx++)
        size = strlen(sec_lsm_manager->reply.fields[idx]) + (idx > 1);

    /* allocate */
    *message = text = malloc(size + 1);
    if (text == NULL)
        return -ENOMEM;

    /* compute size */
    *text = 0;
    for (idx = 1 ; idx < sec_lsm_manager->reply.count ; idx++) {
        if (idx > 1)
            *text++ = ' ';
        strcpy(text, sec_lsm_manager->reply.fields[idx]);
        while (*text)
            text++;
    }
    return 0;
}
