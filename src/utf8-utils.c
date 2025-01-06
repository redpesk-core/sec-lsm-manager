/*
 * Copyright (C) 2020-2025 IoT.bzh Company
 * Author: Arthur Guyader <arthur.guyader@iot.bzh>
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

#include "utf8-utils.h"

/* see utf8-utils.h */
bool is_utf8(const char *text)
{
    unsigned len;
    const unsigned char *iter = (const unsigned char *)text;
    while (*iter) {
        if (*iter <= 0x7f)
            len = 0;
        else if (*iter <= 0xbf)
            return false;
        else if (*iter <= 0xdf)
            len = 1;
        else if (*iter <= 0xef)
            len = 2;
        else if (*iter <= 0xf7)
            len = 3;
        else
            return false;
        for (iter++; len ; len--, iter++)
            if (*iter < 0x80 || *iter > 0xbf)
                return false;
    }
    return true;
}

