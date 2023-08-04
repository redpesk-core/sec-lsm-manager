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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "utils.h"
#include "plugs.h"

__nonnull()
void plugset_init(plugset_t *plugset)
{
    *plugset = NULL;
}

__nonnull()
void plugset_deinit(plugset_t *plugset)
{
    for (;;) {
        plug_t *plug = *plugset;
        if (plug == NULL)
            return;
        *plugset = plug->next;
        free(plug);
    }
}

__wur __nonnull()
int plugset_add(plugset_t *plugset, const char *expdir, const char *impid, const char *impdir)
{
    char *ptr;
    plug_t *plug;

    /* comute length of strings */
    size_t len_expdir = 1 + strlen(expdir);
    size_t len_impid  = 1 + strlen(impid);
    size_t len_impdir = 1 + strlen(impdir);

    /* allocate the new plug structure */
    plug = malloc(len_expdir + len_impid + len_impdir + sizeof *plug);
    if (plug == NULL)
        return -ENOMEM;

    /* initialize strings of the new plug structure */
    ptr = (char*)(&plug[1]);
    plug->expdir = memcpy(ptr, expdir, len_expdir);
    ptr += len_expdir;
    plug->impdir = memcpy(ptr, impdir, len_impdir);
    ptr += len_impdir;
    plug->impid = memcpy(ptr, impid, len_impid);

    /* link the new plug structure in the set */
    plug->next = *plugset;
    *plugset = plug;
    return 0;
}