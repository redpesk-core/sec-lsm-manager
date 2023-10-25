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
#include <errno.h>

#include "protocol/client.h"
#include "protocol/pollitem.h"

int main(int ac, const char **av)
{
    const char *prog = *av++;

    if (ac == 1 || (ac == 2 && 0 == strcmp(av[0], "-"))) {
        static const char * def[2] = { "/dev/stdin", NULL };
        av = (const char **)def;
        ac = 2;
    }

    while(*av) {
        client_t *client;
        int fd = open(*av, O_RDONLY);
        int rc = client_create(&client, fd, dup(1));
        rc = rc == 0 ? 1 : rc;
        while ((rc > 0 || rc == -EAGAIN) && client_is_connected(client))
            rc = client_process_input(client);
        if (client)
            client_destroy(client);
        av++;
    }

    (void)prog;
    return 0;
}

