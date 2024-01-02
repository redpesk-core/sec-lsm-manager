/*
 * Copyright (C) 2020-2024 IoT.bzh Company
 * Author: José Bollo <jose.bollo@iot.bzh>
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

#include "path-utils.h"

/* see path-utils.h */
size_t path_std(char *buffer, size_t size, const char *path)
{
    size_t len = 0;
    int ps = 0, sd = 1, fps = *path == '/';

    /* process the rest of the path */
    for(;;) {
        if (path[0] == '/') {
            ps = 1;
            sd = 1;
            path++;
        }
        else if (sd && path[0] == '.' && (path[1] == '/' || path[1] == 0)) {
            path += 1 + (path[1] != 0);
        }
        else if (sd  && path[0] == '.' && path[1] == '.' && (path[2] == '/' || path[2] == 0)) {
            if (len <= size)
                while (len > 0 && buffer[--len] != '/');
            path += 2 + (path[2] != 0);
            ps = len > 0 || fps;
        }
        else {
            if (ps && (*path || len == 0)) {
                if (len < size)
                    buffer[len] = '/';
                len++;
            }
            if (len < size)
                buffer[len] = *path;
            if (*path == 0) {
                // IREV2: an empty path should return "." to indicate it is relative
                if (len == 0) {
                    if (size > 0)
                        buffer[0] = '.';
                    if (size > 1)
                        buffer[1] = 0;
                    len = 1;
                }
                return len;
            }
            len++;
            path++;
            sd = 0;
            ps = 0;
        }
    }
}

#if 0

int main(int ac, char **av)
{
    char buffer[1024];
    while (*++av) {
    	size_t sz = path_std(buffer, sizeof buffer, *av);
    	printf("%s\n  ==> [%d] %s\n\n",*av,(int)sz,buffer);
    }
    return 0;
}


test: gcc -o pstd pstd.c


./pstd . .. /. /.. ./toto /////a///./b/..//////c//.

.
  ==> [1] .

..
  ==> [1] .

/.
  ==> [1] /

/..
  ==> [1] /

./toto
  ==> [4] toto

/////a///./b/..//////c//.
  ==> [4] /a/c

#endif
