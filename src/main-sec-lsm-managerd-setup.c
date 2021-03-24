/*
 * Copyright (C) 2018-2021 IoT.bzh Company
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

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/statfs.h>
#include <unistd.h>

#ifndef BYNARY_PATH
#define BYNARY_PATH "/usr/bin/"
#endif

#ifdef WITH_SELINUX
#ifndef SELINUX_BYNARY_NAME
#define SELINUX_BYNARY_NAME "sec-lsm-manager-selinuxd"
#endif
#define SELINUX_BYNARY BYNARY_PATH SELINUX_BYNARY_NAME
#define SELINUX_PATH "/sys/fs/selinux/"
#endif

#ifdef WITH_SMACK
#ifndef SMACK_BYNARY_NAME
#define SMACK_BYNARY_NAME "sec-lsm-manager-smackd"
#endif
#define SMACK_BYNARY BYNARY_PATH SMACK_BYNARY_NAME
#define SMACK_PATH "/sys/fs/smackfs/"
#endif

bool mac_enable(const char *path) {
    struct statfs sf;

    if (!statfs(path, &sf)) {
        return true;
    }

    return false;
}

int main(int argc, char **argv, char **envp) {
    int rc = 0;

#ifdef WITH_SELINUX
    if (mac_enable(SELINUX_PATH)) {
        fprintf(stdout, ">> Launch %s\n", SELINUX_BYNARY);
        argv[0] = SELINUX_BYNARY;
        rc = execve(argv[0], argv, envp);
        fprintf(stderr, "error execute %s : %m\n", SELINUX_BYNARY);
    }
#endif
#ifdef WITH_SMACK
    if (mac_enable(SMACK_PATH)) {
        fprintf(stdout, ">> Launch %s\n", SMACK_BYNARY);
        argv[0] = SMACK_BYNARY;
        rc = execve(argv[0], argv, envp);
        fprintf(stderr, "error execute %s : %m\n", SMACK_BYNARY);
    }
#endif

    if (rc == 0) {
        fprintf(stderr, "no mac found\n");
    }

    fprintf(stderr, "launch sec-lsm-manager failed\n");

    return -1;
}
