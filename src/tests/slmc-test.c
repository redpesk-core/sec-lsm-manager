/*
 * Copyright (C) 2018-2023 IoT.bzh Company
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
 */

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "protocol/client.h"
#include "protocol/pollitem.h"

int main(int ac, const char **av)
{
    const char *prog = *av;
    const char * def[] = { prog, "-", NULL };

    if (ac == 1) {
        av = (const char **)def;
    }
    signal(SIGPIPE, SIG_IGN); /* avoid SIGPIPE! */
    while(*++av) {
        char byte;
        client_t *client;
        int rc, pfds[2], df1, st;
        const char *name = strcmp(*av, "-") ? *av : "/dev/stdin";
        int fd = open(name, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "error %s: %s\n", *av, strerror(errno));
            continue;
        }
        df1 = dup(1);
        if (df1 < 0) {
            fprintf(stderr, "error can't dup: %s\n", strerror(errno));
            close(fd);
            continue;
        }
        rc = pipe(pfds);
        if (rc < 0) {
            fprintf(stderr, "error can't pipe: %s\n", strerror(errno));
            close(fd);
            close(df1);
            continue;
        }
        fcntl(pfds[0], F_SETFL, O_RDONLY|O_NONBLOCK);
        fcntl(pfds[1], F_SETFL, O_WRONLY|O_NONBLOCK);
        rc = client_create(&client, pfds[0], df1);
        if (rc < 0) {
            fprintf(stderr, "error can't create client: %s\n", strerror(-rc));
            close(fd);
            close(df1);
            close(pfds[0]);
            close(pfds[1]);
            continue;
        }
        for(rc = st = 1 ; (rc > 0 || rc == -EAGAIN) && client_is_connected(client) ; ) {
            /* get the input */
            while (st != 0) {
                rc = (int)read(fd, &byte, 1);
                if (rc <= 0) {
                    st = 0;
                    close(pfds[1]);
                    pfds[1] = -1;
                    break;
                }
                write(df1, &byte, 1);
                if (st == 1)
                    st = byte == '#' ? 2 : 3;
                if (st == 3) {
                    rc = (int)write(pfds[1], &byte, 1);
                    if (rc <= 0) {
                        st = 0;
                        close(pfds[1]);
                        pfds[1] = -1;
                        break;
                    }
                }
                if (byte == '\n') {
                    st = 1;
                    break;
                }
            }
            rc = client_process_input(client);
        }
        client_destroy(client);
        close(fd);
        close(pfds[1]);
    }

    (void)prog;
    return 0;
}

