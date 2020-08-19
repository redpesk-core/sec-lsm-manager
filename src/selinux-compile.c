/*
 * Copyright (C) 2020 "IoT.bzh"
 * Author Arthur Guyader <arthur.guyader@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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