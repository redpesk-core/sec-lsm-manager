/*
 * Copyright (C) 2020-2025 IoT.bzh Company
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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "log.h"

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see selinux-compile.h */
int launch_compile(const char *id) {
    int rc = 0;
    pid_t pid = 0;
    int status = 0;

    pid = vfork();
    if (pid == 0) {
        DEBUG("Launch : %s %s", COMPILE_SCRIPT_NAME, id);
        execl(COMPILE_SCRIPT, COMPILE_SCRIPT_NAME, id, NULL);
        _exit(EXIT_FAILURE);
    }

    if (pid < 0) {
        rc = -errno;
        ERROR("vfork failed : %d %s", -rc, strerror(-rc));
        return rc;
    }

    rc = waitpid(pid, &status, 0);

    if (rc == -1) {
        rc = -errno;
        ERROR("error wait pid : %d %s", -rc, strerror(-rc));
        return rc;
    }

    if (!WIFEXITED(status)) {
        ERROR("error during compile : child return %d", WEXITSTATUS(status));
        return status;
    }

    return 0;
}
