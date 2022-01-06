/*
 * Copyright (C) 2020-2022 IoT.bzh Company
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

#ifndef SEC_LSM_MANAGER_SIMULATION_SELINUX_H
#define SEC_LSM_MANAGER_SIMULATION_SELINUX_H

#include <stdint.h>

#define SELINUX_RESTORECON_SET_SPECFILE_CTX 1
#define SELINUX_RESTORECON_IGNORE_DIGEST 2

typedef struct semanage_handle semanage_handle_t;
typedef struct semanage_module_info semanage_module_info_t;

extern int is_selinux_enabled(void);

extern int selinux_restorecon(const char *pathname, unsigned int restorecon_flags);

extern int semanage_is_connected(semanage_handle_t *sh);

extern int semanage_disconnect(semanage_handle_t *);

extern void semanage_handle_destroy(semanage_handle_t *);

extern semanage_handle_t *semanage_handle_create(void);

extern void semanage_set_create_store(semanage_handle_t *handle, int create_store);

extern int semanage_connect(semanage_handle_t *);

extern int semanage_set_default_priority(semanage_handle_t *sh, uint16_t priority);

extern int semanage_module_install_file(semanage_handle_t *, const char *module_name);

extern int semanage_commit(semanage_handle_t *);

extern int semanage_module_remove(semanage_handle_t *, char *module_name);

extern int semanage_module_list(semanage_handle_t *sh, semanage_module_info_t **smi, int *num_modules);

extern semanage_module_info_t *semanage_module_list_nth(semanage_module_info_t *list, int n);

extern int semanage_module_info_get_name(semanage_handle_t *sh, semanage_module_info_t *modinfo, const char **name);

extern int semanage_module_info_destroy(semanage_handle_t *handle, semanage_module_info_t *modinfo);

extern int launch_compile(const char *id);

#endif
