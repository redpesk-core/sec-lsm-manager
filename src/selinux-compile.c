/*
 * Copyright (C) 2020 IoT.bzh Company
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

#include "selinux-compile.h"

#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "log.h"

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see selinux-compile.h */
int launch_compile() {
    int rc;

    siginfo_t si;
    char *argv[] = {NULL};
    char *envp[] = {NULL};

    rc = fork();  // Generate two process

    if (rc == 0) {  // FIRST PROCESS ==> rc = 0
        rc = execve(COMPILE_SCRIPT, argv, envp);
        ERROR("can't execute %s %m", COMPILE_SCRIPT);
        _exit(1);
        return rc;
    }

    // OTHER PROCESS ==> rc != 0
    if (rc < 0) {
        rc = -errno;
        ERROR("fork %m");
        return rc;
    }

    rc = waitid(P_PID, (id_t)rc, &si, WEXITED);  // wait end exec

    if (rc < 0) {
        rc = -errno;
        ERROR("waitid");
        return rc;
    }

    if (si.si_code != CLD_EXITED) {
        ERROR("unexpected termination status");
        return -1;
    }

    if (si.si_status != 0) {
        ERROR("child terminated with error code %d", si.si_status);
        return -1;
    }

    return 0;
}
