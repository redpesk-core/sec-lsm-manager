/*
 * Copyright (C) 2018-2023 IoT.bzh Company
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

#if !defined(BINARY_DIR)
#define BINARY_DIR "/usr/bin"
#endif

#if defined(WITH_SELINUX)

#if !defined(SELINUX_FS_PATH)
#define SELINUX_FS_PATH "/sys/fs/selinux"
#endif

#if !defined(SELINUX_BYNARY_NAME)
#define SELINUX_BYNARY_NAME "sec-lsm-manager-selinuxd"
#endif

#define SELINUX_BYNARY BINARY_DIR "/" SELINUX_BYNARY_NAME

#endif

#if defined(WITH_SMACK)

#if !defined(SMACK_FS_PATH)
#define SMACK_FS_PATH "/sys/fs/smackfs"
#endif

#if !defined(SMACK_BYNARY_NAME)
#define SMACK_BYNARY_NAME "sec-lsm-manager-smackd"
#endif

#define SMACK_BYNARY BINARY_DIR "/" SMACK_BYNARY_NAME

#endif

bool mac_enable(const char *path) {
#if !defined(SIMULATE_SMACK) && !defined(SIMULATE_SELINUX)
    return 0 == access(path, F_OK);
#else
    (void)path;
    return true;
#endif
}

int main(int argc, char **argv, char **envp) {
    int rc = 0;
    (void)(argc);
#ifdef WITH_SELINUX
    if (mac_enable(SELINUX_FS_PATH)) {
        fprintf(stdout, ">> Launch %s\n", SELINUX_BYNARY);
        argv[0] = SELINUX_BYNARY;
        rc = execve(argv[0], argv, envp);
        fprintf(stderr, "error execute %s : %m\n", SELINUX_BYNARY);
    }
#endif
#ifdef WITH_SMACK
    if (mac_enable(SMACK_FS_PATH)) {
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
