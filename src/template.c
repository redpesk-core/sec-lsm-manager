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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "log.h"
#include "mustach/mustach.h"
#include "secure-app.h"
#include "utils.h"
#include "template.h"

typedef struct {
        const secure_app_t *secure_app;
        const plug_t *plug;
    }
        template_data_t;

static int put(void *closure, const char *name, int escape, FILE *file) {
    (void)escape;
    template_data_t *data = closure;
    const char *txt = NULL;

    // DEBUG("name : %s", name);

    if (!strcmp(name, "id"))
        txt = data->secure_app->id;

    else if (!strcmp(name, "id_underscore"))
        txt = data->secure_app->id_underscore;

    else if (!strcmp(name, "_id_"))
        txt = data->secure_app->id;

    else if (!strcmp(name, "impid") && data->plug != NULL)
        txt = data->plug->impid;

    else if (!strcmp(name, "_impid_") && data->plug != NULL)
        txt = data->plug->impid;

    if (txt != NULL) {
        if (*name != '_')
            fputs(txt, file);
        else {
            for ( ; *txt ; txt++)
                fputc(*txt == '-' ? '_' : *txt, file);
        }
    }
    return 0;
}

static int enter(void *closure, const char *name) {

    static const char permission_key[] = "p=";
    const size_t permission_key_length = sizeof permission_key - 1;

    template_data_t *data = closure;

    if (data->plug != NULL) /* avoid stacking in plugs ATM */
        return 0;

    if (!strncmp(name, permission_key, permission_key_length)) {
        return secure_app_has_permission(data->secure_app, &name[permission_key_length]);
    }

    if (!strcmp(name, "plug")) {
        data->plug = data->secure_app->plugset;
        return data->plug != NULL;
    }

    return 0;
}

static int leave(void *closure) {
    template_data_t *data = closure;
    DEBUG("leave");
    data->plug = NULL;
    return 0;
}

static int next(void *closure) {
    template_data_t *data = closure;
    DEBUG("next");
    if (data->plug != NULL)
        data->plug = data->plug->next;
    return data->plug != NULL;
}

static struct mustach_itf itf = {.enter = enter, .put = put, .next = next, .leave = leave};

int process_template(const char *template_path, const char *dest, const secure_app_t *secure_app) {
    int rc = 0;
    int rc2 = 0;
    template_data_t data = { .secure_app = secure_app, .plug = NULL };

    char *template = read_file(template_path);
    if (template == NULL) {
        ERROR("read_file : %s", template_path);
        return -EINVAL;
    }

    FILE *f_dest = fopen(dest, "w");
    if (f_dest == NULL) {
        ERROR("fopen : %s", dest);
        rc = -EINVAL;
        goto end;
    }

    rc = fmustach(template, &itf, &data, f_dest);
    if (rc < 0) {
        ERROR("fmustach : %d %s", errno, strerror(errno));
    }

    rc2 = fclose(f_dest);
    if (rc2 < 0) {
        ERROR("fclose %s : %d %s", dest, errno, strerror(errno));
    }

end:
    free(template);
    return rc;
}