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

#include "smack-template.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "template.h"
#include "utils.h"

#define SMACK_EXTENSION "smack"

#if !defined(SEC_LSM_MANAGER_DATADIR)
#define SEC_LSM_MANAGER_DATADIR "/usr/share/sec-lsm-manager"
#endif

#if !defined(TEMPLATE_FILE)
#define TEMPLATE_FILE "app-template.smack"
#endif

#if !defined(SMACK_TEMPLATE_FILE)
#define SMACK_TEMPLATE_FILE SEC_LSM_MANAGER_DATADIR "/" TEMPLATE_FILE
#endif

#if !defined(SMACK_POLICY_DIR)
#define SMACK_POLICY_DIR "/etc/smack/accesses.d"
#endif

const char default_smack_template_file[] = SMACK_TEMPLATE_FILE;
const char default_smack_policy_dir[] = SMACK_POLICY_DIR;

char prefix_app[] = "App:";
char suffix_lib[] = ":Lib";
char suffix_conf[] = ":Conf";
char suffix_exec[] = ":Exec";  // see label_exec before remove this line
char suffix_icon[] = ":Icon";
char suffix_data[] = ":Data";
char suffix_http[] = ":Http";
char user_home[] = "User:Home";
char public_app[] = "System:Shared";

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Remove file and loaded rules
 *
 * @param[in] file The path of the file to remove
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int remove_load_rules(const char *file) {
    int rc = 0;
    int rc2 = 0;
    struct smack_accesses *smack_access = NULL;

    int fd = open(file, O_RDONLY);
    if (fd < 0) {
        rc = -errno;
        ERROR("open file %s : %d %s", file, -rc, strerror(-rc));
        goto ret;
    }

    rc = smack_accesses_new(&smack_access);
    if (rc < 0) {
        ERROR("smack_accesses_new");
        goto end;
    }

    rc = smack_accesses_add_from_file(smack_access, fd);
    if (rc < 0) {
        ERROR("smack_accesses_add_from_file");
        goto end;
    }

    rc = smack_accesses_clear(smack_access);
    if (rc < 0) {
        ERROR("smack_accesses_clear");
        goto end;
    }

end:
    rc2 = close(fd);
    if (rc2 < 0) {
        ERROR("close : %d %s", errno, strerror(errno));
    }
ret:
    smack_accesses_free(smack_access);
    return rc;
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see smack-template.h */
const char *get_smack_template_file(const char *value) {
    return value ?: secure_getenv("SMACK_TEMPLATE_FILE") ?: default_smack_template_file;
}

/* see smack-template.h */
const char *get_smack_policy_dir(const char *value) {
    value = value ?: secure_getenv("SMACK_POLICY_DIR") ?: default_smack_policy_dir;
    if (strlen(value) >= SEC_LSM_MANAGER_MAX_SIZE_DIR) {
        value = NULL;
        ERROR("smack_policy_dir too long");
    }
    return value;
}

/* see smack-label.h */
bool smack_enabled() {
    if (smack_smackfs_path() == NULL) {
        return false;
    }
    return true;
}

/* see smack-label.h */
void init_path_type_definitions(path_type_definitions_t path_type_definitions[number_path_type], const char *id) {
    memset(path_type_definitions, 0, sizeof(path_type_definitions_t) * 9);
    snprintf(path_type_definitions[type_conf].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "%s%s%s", prefix_app, id,
             suffix_conf);
    snprintf(path_type_definitions[type_data].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "%s%s%s", prefix_app, id,
             suffix_data);
    snprintf(path_type_definitions[type_exec].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "%s%s%s", prefix_app, id,
             suffix_exec);
    snprintf(path_type_definitions[type_http].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "%s%s%s", prefix_app, id,
             suffix_http);
    snprintf(path_type_definitions[type_icon].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "%s%s%s", prefix_app, id,
             suffix_icon);
    snprintf(path_type_definitions[type_id].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "%s%s", prefix_app, id);
    snprintf(path_type_definitions[type_lib].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "%s%s%s", prefix_app, id,
             suffix_lib);
    secure_strncpy(path_type_definitions[type_public].label, public_app, SEC_LSM_MANAGER_MAX_SIZE_LABEL);

    // executable
    path_type_definitions[type_exec].is_executable = true;

    // transmute
    path_type_definitions[type_data].is_transmute = true;
    path_type_definitions[type_http].is_transmute = true;
    path_type_definitions[type_id].is_transmute = true;
    path_type_definitions[type_lib].is_transmute = true;
    path_type_definitions[type_public].is_transmute = true;
}

/* see smack-template.h */
int create_smack_rules(const secure_app_t *secure_app) {
    int rc = 0;
    int rc2 = 0;
    struct smack_accesses *smack_accesses = NULL;
    char smack_policy_dir[SEC_LSM_MANAGER_MAX_SIZE_DIR];
    char smack_rules_file[SEC_LSM_MANAGER_MAX_SIZE_PATH];
    char smack_template_file[SEC_LSM_MANAGER_MAX_SIZE_PATH];

    secure_strncpy(smack_policy_dir, get_smack_policy_dir(NULL), SEC_LSM_MANAGER_MAX_SIZE_DIR);
    secure_strncpy(smack_template_file, get_smack_template_file(NULL), SEC_LSM_MANAGER_MAX_SIZE_PATH);

    snprintf(smack_rules_file, SEC_LSM_MANAGER_MAX_SIZE_PATH, "%s/%s.%s", smack_policy_dir, secure_app->id,
             SMACK_EXTENSION);

    rc = process_template(smack_template_file, smack_rules_file, secure_app);
    if (rc < 0) {
        ERROR("process_template : %d %s", -rc, strerror(-rc));
        goto end;
    }

    int fd = open(smack_rules_file, O_RDONLY);
    if (fd < 0) {
        rc = -errno;
        ERROR("open file %s : %d %s", smack_rules_file, -rc, strerror(-rc));
        goto error;
    }

    rc = smack_accesses_new(&smack_accesses);
    if (rc < 0) {
        ERROR("smack_accesses_new");
        goto error;
    }

    rc = smack_accesses_add_from_file(smack_accesses, fd);
    if (rc < 0) {
        ERROR("smack_accesses_add_from_file");
        goto error;
    }

    if (smack_enabled()) {
        rc = smack_accesses_apply(smack_accesses);
        if (rc < 0) {
            ERROR("smack_accesses_apply");
            goto error;
        }
    }

    DEBUG("create_smack_rules success");
    goto end;

error:
    rc2 = remove(smack_rules_file);
    if (rc2 < 0) {
        ERROR("remove %s : %d %s", smack_rules_file, errno, strerror(errno));
    }
end:
    smack_accesses_free(smack_accesses);
    smack_accesses = NULL;
    return rc;
}

/* see smack-template.h */
int remove_smack_rules(const secure_app_t *secure_app) {
    int rc = 0;
    char smack_policy_dir[SEC_LSM_MANAGER_MAX_SIZE_DIR];
    char smack_rules_file[SEC_LSM_MANAGER_MAX_SIZE_PATH];

    secure_strncpy(smack_policy_dir, get_smack_policy_dir(NULL), SEC_LSM_MANAGER_MAX_SIZE_DIR);
    snprintf(smack_rules_file, SEC_LSM_MANAGER_MAX_SIZE_PATH, "%s/%s.%s", smack_policy_dir, secure_app->id,
             SMACK_EXTENSION);

    if (smack_enabled()) {
        rc = remove_load_rules(smack_rules_file);
        if (rc < 0) {
            ERROR("remove_load_rules : %d %s", -rc, strerror(-rc));
        }
    }

    rc = remove_file(smack_rules_file);
    if (rc < 0) {
        ERROR("remove_file %s : %d %s", smack_rules_file, -rc, strerror(-rc));
    }

    return rc;
}
