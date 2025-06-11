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

#include "offline.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "protocol/client.h"

/* see cynagora-interface.h */
__wur __nonnull((1))
static int add_permissions(const char *label, const permission_set_t *permission_set, int drop_before)
{
    (void)drop_before;
    /* add permissions */
    if (permission_set != NULL) {
        unsigned i;
        for (i = 0 ; i < permission_set->size ; i++)
            fprintf(stdout, "%s * * %s yes forever\n",
                label, permission_set->permissions[i]);
    }
    return 0;
}

static int drop_permissions(const char *label)
{
    (void)label;
    return 0; /* do nothing */
}

__nonnull() __wur
static int check_permission(const char *label, const char *permission)
{
    (void)label;
    (void)permission;
    return 1; /* always accept !!! */
}

static void fail(const char *message)
{
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
}

void offline(void)
{
    perm_mgr_itf_t offlineitf =
    {
        .add_permissions  = add_permissions,
        .drop_permissions = drop_permissions,
        .check_permission = check_permission
    };
    client_t *client = NULL;
    int rc, fdout;

    fdout = open("/dev/null", O_WRONLY);
    if (fdout < 0)
        fail("can't open /dev/null");

    rc = client_create(&client, STDIN_FILENO, fdout);
    if (rc < 0)
        fail("can't create client");

    client_set_permission_manager(client, &offlineitf);
    for(;;) {
        rc = client_process_input(client);
        if (rc < 0)
            fail("error while processing");
        if (rc == 0)
            exit(EXIT_SUCCESS);
    }
}
