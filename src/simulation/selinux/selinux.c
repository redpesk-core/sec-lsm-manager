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

#include "selinux.h"

static int ptr = 0;

#if !defined(SEC_LSM_MANAGER_DATADIR)
#define SEC_LSM_MANAGER_DATADIR "/usr/share/sec-lsm-manager"
#endif

#if !defined(SELINUX_RULES_DIR)
#define SELINUX_RULES_DIR SEC_LSM_MANAGER_DATADIR "selinux-rules"
#endif

#if !defined(SELINUX_POLICY_DIR)
#define SELINUX_POLICY_DIR SEC_LSM_MANAGER_DATADIR "selinux-simulation"
#endif

#include <dirent.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

struct semanage_module_info {
    char name[256];
};

int create_file(const char *path) {
    printf("create_file(%s)\n", path);
    int rc = -1;
    FILE *f = fopen(path, "w");
    if (f) {
        fclose(f);
        rc = 0;
    }
    return rc;
}

////////////////////////////////////////////

int is_selinux_enabled(void) {
    printf("cynagora_async_process()\n");
    return 1;
}

int selinux_restorecon(const char *pathname, unsigned int restorecon_flags) {
    printf("selinux_restorecon(%s, %u)\n", pathname, restorecon_flags);
    return 0;
}

int semanage_is_connected(semanage_handle_t *sh) {
    printf("semanage_is_connected(%p)\n", sh);
    return 0;
}

int semanage_disconnect(semanage_handle_t *sh) {
    printf("semanage_disconnect(%p)\n", sh);
    return 0;
}

semanage_handle_t *semanage_handle_create(void) {
    printf("semanage_handle_create()\n");
    semanage_handle_t *ret = (void *)(intptr_t)(++ptr);
    return ret;
}

void semanage_set_create_store(semanage_handle_t *handle, int create_store) {
    printf("semanage_set_create_store(%p, %d)\n", handle, create_store);
}

int semanage_set_default_priority(semanage_handle_t *sh, uint16_t priority) {
    printf("semanage_set_default_priority(%p, %u)\n", sh, priority);
    return 0;
}

int semanage_connect(semanage_handle_t *sh) {
    printf("semanage_connect(%p)\n", sh);
    return 0;
}

int semanage_commit(semanage_handle_t *sh) {
    printf("semanage_commit(%p)\n", sh);
    return 0;
}

void semanage_handle_destroy(semanage_handle_t *sh) { printf("semanage_handle_destroy(%p)\n", sh); }

int semanage_module_install_file(semanage_handle_t *sh, const char *file_path) {
    printf("semanage_module_install_file(%p, %s)\n", sh, file_path);
    mkdir(SELINUX_POLICY_DIR, 0755);
    char path[200];
    char *f_tmp = strdup(file_path);
    char *b = basename(f_tmp);
    b[strlen(b) - 3] = 0;
    snprintf(path, 200, "%s/%s", SELINUX_POLICY_DIR, b);
    int rc = create_file(path);
    return rc;
}

int semanage_module_remove(semanage_handle_t *sh, char *module_name) {
    printf("semanage_module_remove(%p, %s)\n", sh, module_name);
    char path[200];
    snprintf(path, 200, "%s/%s", SELINUX_POLICY_DIR, module_name);
    remove(path);
    return 0;
}

int semanage_module_list(semanage_handle_t *sh, semanage_module_info_t **semanage_module_info, int *num_modules) {
    printf("semanage_module_list(%p)\n", sh);
    *semanage_module_info = NULL;
    *num_modules = 0;

    DIR *dir = NULL;
    struct dirent *file = NULL;
    int i = 0;

    dir = opendir(SELINUX_POLICY_DIR);
    if (dir != NULL) {
        while (readdir(dir) != NULL) {
            (*num_modules)++;
        }
        closedir(dir);
        dir = NULL;
    }

    *semanage_module_info = (semanage_module_info_t *)malloc(sizeof(semanage_module_info_t) * (size_t)(*num_modules));

    dir = opendir(SELINUX_POLICY_DIR);
    if (dir != NULL) {
        while (i < *num_modules) {
            file = readdir(dir);
            secure_strncpy((*semanage_module_info)[i].name, file->d_name, 256);
            i++;
        }
        closedir(dir);
    }

    return 0;
}

semanage_module_info_t *semanage_module_list_nth(semanage_module_info_t *list, int n) {
    printf("semanage_module_list_nth(%p, %d)\n", list, n);
    return list + n;
}

int semanage_module_info_get_name(semanage_handle_t *sh, semanage_module_info_t *modinfo, const char **name) {
    printf("semanage_module_info_get_name(%p, %p)\n", sh, modinfo);
    *name = modinfo->name;
    return 0;
}

int semanage_module_info_destroy(semanage_handle_t *handle, semanage_module_info_t *modinfo) {
    printf("semanage_module_info_destroy(%p, %p)\n", handle, modinfo);
    return 0;
}

int launch_compile(const char *id) {
    printf("launch_compile(%s)\n", id);
    char path[200];
    snprintf(path, 200, "%s/%s.pp", SELINUX_RULES_DIR, id);
    int rc = create_file(path);
    return rc;
}
