/*
 * Copyright (C) 2020-2021 IoT.bzh Company
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

#include "log.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see log.h */
void log_function(const char *msg, ...) {
    va_list va;
    va_start(va, msg);
    fprintf(stdout, ">> ");
    vfprintf(stdout, msg, va);
    fprintf(stdout, "\n");
    va_end(va);
}

/* see log.h */
void debug_function(const char *msg, ...) {
    (void)msg;
#if defined(DEBUG_MODE)
    va_list va;
    va_start(va, msg);
    fprintf(stdout, "[DEBUG] ");
    vfprintf(stdout, msg, va);
    fprintf(stdout, "\n");
    va_end(va);
#endif
}

/* see log.h */
void error_function(const char *file, const int line, const char *msg, ...) {
    va_list va;
    va_start(va, msg);
    fprintf(stderr, "[%s:%d] error : ", file, line);
    vfprintf(stderr, msg, va);
    fprintf(stderr, "\n");
    va_end(va);
}
