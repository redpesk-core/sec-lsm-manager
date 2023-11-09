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
/* IMPLEMENTATION OF EPOLL HELPER                                             */
/******************************************************************************/
/******************************************************************************/
#include "pollitem.h"

#include <sys/epoll.h>

/**
 * @brief Wraps the call to epoll_ctl for operation 'op'
 *
 * @param pollitem the pollitem to process
 * @param events the expected events
 * @param pollfd the file descriptor for epoll
 * @param op the operation to perform
 * @return 0 on success or -1 with errno set accordingly to epoll_ctl
 */
static int pollitem_do(pollitem_t *pollitem, uint32_t events, int pollfd, int op) {
    struct epoll_event ev = {.events = events, .data.ptr = pollitem};
    return epoll_ctl(pollfd, op, pollitem->fd, &ev);
}

/* see pollitem.h */
int pollitem_add(pollitem_t *pollitem, uint32_t events, int pollfd) {
    return pollitem_do(pollitem, events, pollfd, EPOLL_CTL_ADD);
}

/* see pollitem.h */
int pollitem_mod(pollitem_t *pollitem, uint32_t events, int pollfd) {
    return pollitem_do(pollitem, events, pollfd, EPOLL_CTL_MOD);
}

/* see pollitem.h */
int pollitem_del(pollitem_t *pollitem, int pollfd) {
    return pollitem_do(pollitem, 0, pollfd, EPOLL_CTL_DEL);
}

/* see pollitem.h */
int pollitem_wait_dispatch(int pollfd, int timeout) {
    int rc;
    struct epoll_event ev;
    pollitem_t *pi;

    rc = epoll_wait(pollfd, &ev, 1, timeout);
    if (rc == 1) {
        pi = ev.data.ptr;
        pi->handler(pi, ev.events, pollfd);
    }
    return rc;
}
