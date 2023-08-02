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

#ifndef SEC_LSM_MANAGER_PLUGS_H
#define SEC_LSM_MANAGER_PLUGS_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/cdefs.h>

#include "limits.h"

/**
 * @brief Structure of plug
 */
typedef struct plug {
    struct plug *next;
    const char *expdir; /* exported directory */
    const char *impid;  /* import appid */
    const char *impdir; /* import directory */
} plug_t;

/**
 * @brief Structure of plugset
 */
typedef plug_t *plugset_t;

/**
 * @brief Initialize the fields 'size' and 'plugs'
 *
 * @param[in] plugset plugset handler
 */
__nonnull()
extern void plugset_init(plugset_t *plugset);

/**
 * @brief Free plugs that have been added
 * The pointer is not free
 *
 * @param[in] plugset plugset handler
 */
__nonnull()
extern void plugset_deinit(plugset_t *plugset);

/**
 * @brief Add a plug to plugs
 *
 * @param[in] plugset plugset handler
 * @param[in] expdir   exported directory
 * @param[in] impid    import appid
 * @param[in] impdir   import directory
 * @return 0 in case of success or a negative -errno value
 */
__wur __nonnull()
extern int plugset_add(plugset_t *plugset, const char *expdir, const char *impid, const char *impdir);

#endif
