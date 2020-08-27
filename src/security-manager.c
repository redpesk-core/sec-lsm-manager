/*
 * Copyright (C) 2020 IoT.bzh Company
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

#include "security-manager.h"

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
#include "security-manager-operation.h"
#include "security-manager-protocol.h"
#include "socket.h"

/**
 * structure recording a client
 */
struct security_manager {
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
 * @param[in] security_manager  the handler of the client
 *
 * @return  0 in case of success or a negative -errno value
 */
static int flushw(security_manager_t *security_manager) {
    int rc;
    struct pollfd pfd;

    for (;;) {
        rc = prot_should_write(security_manager->prot);
        if (!rc)
            break;
        rc = prot_write(security_manager->prot, security_manager->fd);
        if (rc == -EAGAIN) {
            pfd.fd = security_manager->fd;
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
 * @param[in] security_manager the client
 * @param[in] fields the fields to send
 * @param[in] count the count of fields
 * @return 0 on success or a negative error code
 */
static int send_reply(security_manager_t *security_manager, const char **fields, int count) {
    int rc, trial, i;
    prot_t *prot;

    /* retrieves the protocol handler */
    prot = security_manager->prot;
    trial = 0;
    for (;;) {
        /* fill the fields */
        for (i = rc = 0; i < count && rc == 0; i++) rc = prot_put_field(prot, fields[i]);

        /* send if done */
        if (rc == 0) {
            rc = prot_put_end(prot);
            if (rc == 0) {
                rc = flushw(security_manager);
                break;
            }
        }

        /* failed to fill protocol, cancel current composition  */
        prot_put_cancel(prot);

        /* fail if was last trial */
        if (trial)
            break;

        /* try to flush the output buffer */
        rc = flushw(security_manager);
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
 * @param[in] security_manager  the handler of the client
 * @param[in] command   the command to send
 * @param[in] optarg    an optional argument or NULL
 * @param[in] ...       optional arguments or NULL
 *
 * @return  0 in case of success or a negative -errno value
 */
static int putxkv(security_manager_t *security_manager, const char *command, const char *optarg, ...) {
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
    rc = send_reply(security_manager, fields, nf - 1);
    return rc;
}

/**
 * @brief Wait some input event
 *
 * @param[in] security_manager  the handler of the client
 *
 * @return  0 in case of success or a negative -errno value
 */
static int wait_input(security_manager_t *security_manager) {
    int rc;
    struct pollfd pfd;

    pfd.fd = security_manager->fd;
    pfd.events = POLLIN;
    do {
        rc = poll(&pfd, 1, -1);
    } while (rc < 0 && errno == EINTR);
    return rc < 0 ? -errno : 0;
}

/**
 * @brief Get the next reply if any
 *
 * @param[in] security_manager  the handler of the client
 *
 * @return  the count of field of the reply (can be 0)
 *          or -EAGAIN if there is no reply
 */
static int get_reply(security_manager_t *security_manager) {
    int rc;

    prot_next(security_manager->prot);
    rc = prot_get(security_manager->prot, &security_manager->reply.fields);

    security_manager->reply.count = rc;
    return rc;
}

/**
 * @brief Wait for a reply
 *
 * @param[in] security_manager  the handler of the client
 * @param[in] block
 *
 * @return  the count of fields greater than 0 or a negative -errno value
 *          or -EAGAIN if nothing and block == false
 *          or -EPIPE if broken link
 */
static int wait_reply(security_manager_t *security_manager, bool block) {
    int rc;

    for (;;) {
        /* get the next reply if any */
        rc = get_reply(security_manager);
        if (rc > 0)
            return rc;

        if (rc < 0) {
            /* wait for an answer */
            rc = prot_read(security_manager->prot, security_manager->fd);
            while (rc <= 0) {
                if (rc == 0)
                    return -(errno = EPIPE);
                if (rc == -EAGAIN && block)
                    rc = wait_input(security_manager);
                if (rc < 0)
                    return rc;
                rc = prot_read(security_manager->prot, security_manager->fd);
            }
        }
    }
}

/**
 * @brief Wait for a reply
 *
 * @param[in] security_manager  the handler of the client
 *
 * @return  0 in case of success or a negative -errno value
 */
static int wait_any_reply(security_manager_t *security_manager) {
    int rc;
    for (;;) {
        rc = wait_reply(security_manager, true);
        if (rc < 0)
            return rc;
        if (rc > 0)
            return rc;
    }
}

/**
 * @brief Wait the reply "done" or "error"
 *
 * @param[in] security_manager  the handler of the client
 *
 * @return  0 in case of success or a negative -errno value
 *          -ECANCELED when received an error status
 */
static int wait_done_or_error(security_manager_t *security_manager) {
    int rc = wait_any_reply(security_manager);
    if (rc > 0) {
        if (!strcmp(security_manager->reply.fields[0], _done_)) {
            return rc;
        } else if (!strcmp(security_manager->reply.fields[0], _error_)) {
            ERROR("%s", security_manager->reply.fields[1]);
            return -1;
        } else {
            return -ECANCELED;
        }
    }
    return rc;
}

/**
 * @brief Disconnect the client
 *
 * @param[in] security_manager  the handler of the client
 */
static void disconnection(security_manager_t *security_manager) {
    if (security_manager->fd >= 0) {
        close(security_manager->fd);
        security_manager->fd = -1;
    }
}

/**
 * @brief Connect the client
 *
 * @param[in] security_manager  the handler of the client
 *
 * @return  0 in case of success or a negative -errno value
 */
static int connection(security_manager_t *security_manager) {
    int rc;

    /* init the client */
    security_manager->reply.count = -1;
    prot_reset(security_manager->prot);
    security_manager->fd = socket_open(security_manager->socketspec, 0);
    if (security_manager->fd < 0)
        return -errno;

    /* negociate the protocol */
    rc = putxkv(security_manager, _security_manager_, "1", NULL);
    if (rc >= 0) {
        rc = wait_any_reply(security_manager);
        if (rc >= 0) {
            rc = -EPROTO;
            if (security_manager->reply.count >= 2 && 0 == strcmp(security_manager->reply.fields[0], _done_) &&
                0 == strcmp(security_manager->reply.fields[1], "1")) {
                return 0;
            }
        }
    }
    disconnection(security_manager);
    return rc;
}

/**
 * @brief Ensure the connection is opened
 *
 * @param[in] security_manager  the handler of the client
 *
 * @return  0 in case of success or a negative -errno value
 */
static int ensure_opened(security_manager_t *security_manager) {
    if (security_manager->fd >= 0 && write(security_manager->fd, NULL, 0) < 0)
        disconnection(security_manager);
    return security_manager->fd < 0 ? connection(security_manager) : 0;
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see security-manager.h */
int security_manager_create(security_manager_t **security_manager, const char *socketspec) {
    socketspec = security_manager_get_socket(socketspec);

    /* allocate the structure */
    *security_manager = (security_manager_t *)malloc(sizeof(security_manager_t));
    if (*security_manager == NULL) {
        return -ENOMEM;
    }
    memset(*security_manager, 0, sizeof(security_manager_t));

    /* socket spec */
    (*security_manager)->socketspec = strdup(socketspec);
    if ((*security_manager)->socketspec == NULL) {
        free(*security_manager);
        *security_manager = NULL;
        return -ENOMEM;
    }

    /* create a protocol object */
    int rc = prot_create(&(*security_manager)->prot);
    if (rc < 0) {
        free(*security_manager);
        *security_manager = NULL;
        free((*security_manager)->socketspec);
        (*security_manager)->socketspec = NULL;
        return rc;
    }

    /* record type and weakly create cache */
    (*security_manager)->synclock = false;

    /* lazy connection */
    (*security_manager)->fd = -1;

    /* done */
    return 0;
}

/* see security-manager.h */
void security_manager_destroy(security_manager_t *security_manager) {
    if (security_manager) {
        disconnection(security_manager);
        if (security_manager->prot)
            prot_destroy(security_manager->prot);
        free(security_manager->socketspec);
        security_manager->socketspec = NULL;
        free(security_manager);
        security_manager = NULL;
    }
}

/* see security-manager.h */
void security_manager_disconnect(security_manager_t *security_manager) {
    if (!security_manager) {
        ERROR("security_manager undefined");
        return;
    }
    disconnection(security_manager);
}

/* see security-manager.h */
int security_manager_set_id(security_manager_t *security_manager, const char *id) {
    if (!security_manager) {
        ERROR("security_manager undefined");
        return -EINVAL;
    } else if (!id) {
        ERROR("id undefined");
        return -EINVAL;
    }

    if (security_manager->synclock)
        return -EBUSY;

    security_manager->synclock = true;

    int rc = ensure_opened(security_manager);
    if (rc < 0) {
        goto ret;
    }

    rc = putxkv(security_manager, _id_, id, NULL);
    if (rc < 0) {
        goto ret;
    }

    rc = wait_done_or_error(security_manager);

ret:
    security_manager->synclock = false;
    return rc;
}

/* see security-manager.h */
int security_manager_add_path(security_manager_t *security_manager, const char *path, const char *path_type) {
    if (!security_manager) {
        ERROR("security_manager undefined");
        return -EINVAL;
    } else if (!path) {
        ERROR("path undefined");
        return -EINVAL;
    } else if (!path_type) {
        ERROR("path_type invalid");
        return -EINVAL;
    }

    if (security_manager->synclock)
        return -EBUSY;

    security_manager->synclock = true;
    int rc = ensure_opened(security_manager);
    if (rc < 0) {
        goto ret;
    }
    rc = putxkv(security_manager, _path_, path, path_type, NULL);
    if (rc < 0) {
        goto ret;
    }

    rc = wait_done_or_error(security_manager);

ret:
    security_manager->synclock = false;
    return rc;
}

/* see security-manager.h */
int security_manager_add_permission(security_manager_t *security_manager, const char *permission) {
    if (!security_manager) {
        ERROR("security_manager undefined");
        return -EINVAL;
    } else if (!permission) {
        ERROR("permission undefined");
        return -EINVAL;
    }

    if (security_manager->synclock)
        return -EBUSY;

    security_manager->synclock = true;

    int rc = ensure_opened(security_manager);
    if (rc < 0) {
        goto ret;
    }
    rc = putxkv(security_manager, _permission_, permission, NULL);
    if (rc < 0) {
        goto ret;
    }

    rc = wait_done_or_error(security_manager);

ret:
    security_manager->synclock = false;
    return rc;
}

/* see security-manager.h */
int security_manager_clean(security_manager_t *security_manager) {
    if (!security_manager) {
        ERROR("security_manager undefined");
        return -EINVAL;
    }

    if (security_manager->synclock)
        return -EBUSY;

    security_manager->synclock = true;

    int rc = ensure_opened(security_manager);
    if (rc < 0) {
        goto ret;
    }
    rc = putxkv(security_manager, _clean_, NULL);
    if (rc < 0) {
        goto ret;
    }

    rc = wait_done_or_error(security_manager);

ret:
    security_manager->synclock = false;
    return rc;
}

/* see security-manager.h */
int security_manager_install(security_manager_t *security_manager) {
    if (!security_manager) {
        ERROR("security_manager undefined");
        return -EINVAL;
    }

    if (security_manager->synclock)
        return -EBUSY;

    security_manager->synclock = true;

    int rc = ensure_opened(security_manager);
    if (rc < 0) {
        goto ret;
    }
    rc = putxkv(security_manager, _install_, NULL);
    if (rc < 0) {
        goto ret;
    }

    rc = wait_done_or_error(security_manager);

ret:
    security_manager->synclock = false;
    return rc;
}

/* see security-manager.h */
int security_manager_uninstall(security_manager_t *security_manager) {
    if (!security_manager) {
        ERROR("security_manager undefined");
        return -EINVAL;
    }

    if (security_manager->synclock)
        return -EBUSY;

    security_manager->synclock = true;

    int rc = ensure_opened(security_manager);
    if (rc < 0) {
        goto ret;
    }
    rc = putxkv(security_manager, _uninstall_, NULL);
    if (rc < 0) {
        goto ret;
    }

    rc = wait_done_or_error(security_manager);

ret:
    security_manager->synclock = false;
    return rc;
}

/* see security-manager.h */
int security_manager_log(security_manager_t *security_manager, int on, int off) {
    if (!security_manager) {
        ERROR("security_manager undefined");
        return -EINVAL;
    }

    if (security_manager->synclock)
        return -EBUSY;

    security_manager->synclock = true;
    int rc = ensure_opened(security_manager);
    if (rc >= 0) {
        rc = putxkv(security_manager, _log_, off ? _off_ : on ? _on_ : 0, NULL);
        if (rc >= 0) {
            rc = wait_done_or_error(security_manager);
            if (rc > 0)
                rc = security_manager->reply.count >= 2 && !strcmp(security_manager->reply.fields[1], _on_);
        }
    }
    security_manager->synclock = false;

    return rc < 0 ? rc : security_manager->reply.count < 2 ? 0 : !strcmp(security_manager->reply.fields[1], _on_);
}

/* see security-manager.h */
int security_manager_display(security_manager_t *security_manager) {
    if (!security_manager) {
        ERROR("security_manager undefined");
        return -EINVAL;
    }

    if (security_manager->synclock)
        return -EBUSY;

    security_manager->synclock = true;

    int rc = ensure_opened(security_manager);
    if (rc < 0) {
        goto ret;
    }

    rc = putxkv(security_manager, _display_, NULL);

    if (rc < 0) {
        goto ret;
    }

    rc = wait_reply(security_manager, true);
    printf("################## SECURE APP ##################\n");

    while (rc > 2 && !strcmp(security_manager->reply.fields[0], _string_)) {
        if (!strcmp(security_manager->reply.fields[1], _id_)) {
            printf("id : %s\n", security_manager->reply.fields[2]);
        }

        if (!strcmp(security_manager->reply.fields[1], _permission_)) {
            printf("permission : %s\n", security_manager->reply.fields[2]);
        }

        if (!strcmp(security_manager->reply.fields[1], _path_) && rc > 3) {
            printf("path : %s %s\n", security_manager->reply.fields[2], security_manager->reply.fields[3]);
        }

        rc = wait_reply(security_manager, true);
    }

    printf("################################################\n");

    if (rc < 0 && strcmp(security_manager->reply.fields[0], _done_)) {
        ERROR("display not done");
        goto ret;
    }

ret:
    security_manager->synclock = false;
    return rc;
}
