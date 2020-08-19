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
void error_function(const char *file, const int line, const char *msg, ...) {
    va_list va;
    va_start(va, msg);
    fprintf(stderr, "[%s:%d] error : ", file, line);
    vfprintf(stderr, msg, va);
    fprintf(stderr, "\n");
    va_end(va);
}