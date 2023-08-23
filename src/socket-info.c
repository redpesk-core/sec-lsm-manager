/*
 * Copyright (C) 2020-2023 IoT.bzh Company
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

#include "socket-info.h"

int get_socket_info(int fd, socket_info_t *socket_info) {
    int rc = 0;
    struct ucred uc;
    socklen_t len = sizeof(uc);

    if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &uc, &len)) {
        rc = -errno;
        goto ret;
    }

#if WITH_SMACK
    rc = (int)smack_new_label_from_socket(fd, &(socket_info->label));
    if (rc < 0) {
        rc = -errno;
        ERROR("smack_new_label_from_socket");
        goto ret;
    }
#elif WITH_SELINUX
    socket_info->label = strdup("LABELSOCKETSELINUX");
#else
    socket_info->label = strdup("LABELSOCKET");
#endif
    socket_info->uid = uc.uid;
    socket_info->pid = uc.pid;

    socket_info->uid_str = get_uid_str(socket_info->uid);
    if (socket_info->uid_str == NULL) {
        ERROR("get_uid_str %d", socket_info->uid);
        rc = -EINVAL;
        goto ret;
    }

ret:
    return rc;
}
