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

static int put(void *closure, const char *name, int escape, FILE *file) {
    (void)escape;
    const secure_app_t *secure_app = (const secure_app_t *)closure;
    // DEBUG("name : %s", name);

    if (!strcmp(name, "id")) {
        fputs(secure_app->id, file);
    } else if (!strcmp(name, "id_underscore")) {
        fputs(secure_app->id_underscore, file);
    }

    return 0;
}

static int enter(void *closure, const char *name) {
    const secure_app_t *secure_app = (const secure_app_t *)closure;
    for (size_t i = 0; i < secure_app->permission_set.size; i++) {
        if (!strcasecmp(name, secure_app->permission_set.permissions[i])) {
            return 1;
        }
    }
    return 0;
}

static int leave(void *closure) {
    (void)closure;
    DEBUG("leave");
    return 0;
}

static int next(void *closure) {
    (void)closure;
    DEBUG("next");
    return 0;
}

static struct mustach_itf itf = {.enter = enter, .put = put, .next = next, .leave = leave};

int process_template(const char *template_path, const char *dest, const secure_app_t *secure_app) {
    int rc = 0;
    int rc2 = 0;
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

    rc = fmustach(template, &itf, (void*)secure_app, f_dest);
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