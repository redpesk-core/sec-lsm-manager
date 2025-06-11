/*
 * Copyright (C) 2018-2025 IoT.bzh Company
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

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/statfs.h>
#include <unistd.h>

#if !defined(BINARY_DIR)
#define BINARY_DIR "/usr/bin"
#endif

#if WITH_SELINUX

#if !defined(SELINUX_FS_PATH)
#define SELINUX_FS_PATH "/sys/fs/selinux"
#endif

#if !defined(SELINUX_BYNARY_NAME)
#define SELINUX_BYNARY_NAME "sec-lsm-manager-selinuxd"
#endif

#define SELINUX_BYNARY BINARY_DIR "/" SELINUX_BYNARY_NAME

#endif

#if WITH_SMACK

#if !defined(SMACK_FS_PATH)
#define SMACK_FS_PATH "/sys/fs/smackfs"
#endif

#if !defined(SMACK_BYNARY_NAME)
#define SMACK_BYNARY_NAME "sec-lsm-manager-smackd"
#endif

#define SMACK_BYNARY BINARY_DIR "/" SMACK_BYNARY_NAME

#endif

/**/

#if SIMULATE_SELINUX
#undef SELINUX_FS_PATH
#define SELINUX_FS_PATH NULL
#endif

#if SIMULATE_SMACK
#undef SMACK_FS_PATH
#define SMACK_FS_PATH NULL
#endif


static
bool run_if(int argc, char **argv, char **envp, const char *binary, const char *condpath)
{
    (void)(argc); /* avoid unused argument warning */

    if (condpath && access(condpath, F_OK) != 0 && errno == ENOENT)
        return false;

    execve(binary, argv, envp);

    fprintf(stderr, "error executing %s: %s\n", binary, strerror(errno));
    return true;
}

int main(int argc, char **argv, char **envp) {
    bool failed = false;

#if WITH_SELINUX
    failed |= run_if(argc, argv, envp, SELINUX_BYNARY, SELINUX_FS_PATH);
#endif
#if WITH_SMACK
    failed |= run_if(argc, argv, envp, SMACK_BYNARY, SMACK_FS_PATH);
#endif
    if (!failed)
        fprintf(stderr, "no mac found\n");

    fprintf(stderr, "launching sec-lsm-manager implementation failed\n");

    return EXIT_FAILURE;
}
