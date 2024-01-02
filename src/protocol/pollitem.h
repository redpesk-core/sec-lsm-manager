/*
 * Copyright (C) 2018-2024 IoT.bzh Company
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

#ifndef SEC_LSM_MANAGER_POLLITEM_H
#define SEC_LSM_MANAGER_POLLITEM_H

/******************************************************************************/
/******************************************************************************/
/* IMPLEMENTATION OF EPOLL HELPER                                             */
/******************************************************************************/
/******************************************************************************/

#include <stdint.h>
/** structure for using epoll easily */
typedef struct pollitem pollitem_t;

/**
 * Structure for using epoll easily
 */
struct pollitem {
    /** callback on event */
    void (*handler)(pollitem_t *pollitem, uint32_t events, int pollfd);

    /** data of any kind free to use */
    void *closure;

    /** file descriptor */
    int fd;
};

/**
 * @brief Add a pollitem to epoll
 *
 * @param pollitem the pollitem to add
 * @param events expected events
 * @param pollfd file descriptor of the epoll
 * @return 0 on success or -1 with errno set accordingly to epoll_ctl
 */
extern int pollitem_add(pollitem_t *pollitem, uint32_t events, int pollfd);

/**
 * @brief Modify a pollitem of epoll
 *
 * @param pollitem the pollitem to modify
 * @param events expected events
 * @param pollfd file descriptor of the epoll
 * @return 0 on success or -1 with errno set accordingly to epoll_ctl
 */
extern int pollitem_mod(pollitem_t *pollitem, uint32_t events, int pollfd);

/**
 * @brief Delete a pollitem from epoll
 *
 * @param pollitem the pollitem to delete
 * @param pollfd file descriptor of the epoll
 * @return 0 on success or -1 with errno set accordingly to epoll_ctl
 */
extern int pollitem_del(pollitem_t *pollitem, int pollfd);

/**
 * @brief Wait one event on epoll and dispatch it to its pollitem callback
 *
 * @param pollfd file descriptor of the epoll
 * @param timeout time to wait
 * @return 0 on timeout
 *         1 if a callback was called
 *         -1 with errno set accordingly to epoll_wait
 */
extern int pollitem_wait_dispatch(int pollfd, int timeout);

#endif
