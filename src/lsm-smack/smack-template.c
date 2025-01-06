/*
 * Copyright (C) 2020-2025 IoT.bzh Company
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

#if SIMULATE_SMACK
#include "simulation/smack/smack.h"
#include "simulation/smack/smack.c"
#else
#include <sys/smack.h>
#endif

#include "log.h"
#include "templating/template.h"
#include "file-utils.h"

#define SMACK_EXTENSION "smack"

#if !defined(PREFIX)
#define PREFIX "/usr"
#endif

#if !defined(DATADIR)
#define DATADIR   PREFIX"/share"
#endif

#if !defined(SYSCONFDIR)
#define SYSCONFDIR "/etc"
#endif

#if !defined(SEC_LSM_MANAGER_DATADIR)
#define SEC_LSM_MANAGER_DATADIR  DATADIR"/sec-lsm-manager"
#endif

#if !defined(TEMPLATE_FILE)
#define TEMPLATE_FILE   "app-template.smack"
#endif

#if !defined(SMACK_TEMPLATE_FILE)
#define SMACK_TEMPLATE_FILE SEC_LSM_MANAGER_DATADIR "/" TEMPLATE_FILE
#endif

#if !defined(SMACK_POLICY_DIR)
#define SMACK_POLICY_DIR   SYSCONFDIR"/smack/accesses.d"
#endif



const char default_smack_template_file[] = SMACK_TEMPLATE_FILE;
const char default_smack_policy_dir[] = SMACK_POLICY_DIR;

static const struct {
    const char *label;
    bool exec;
    bool transmute;
} patterns[number_path_type] =
{
    [type_unset]   = { "",              false, false },
    [type_default] = { "",              false, false },
    [type_conf]    = { "App:%s:Conf",   false, false },
    [type_data]    = { "App:%s:Data",   false, true },
    [type_exec]    = { "App:%s:Exec",   true,  false },
    [type_http]    = { "App:%s:Http",   false, true },
    [type_icon]    = { "App:%s:Icon",   false, false },
    [type_id]      = { "App:%s",        false, true },
    [type_lib]     = { "App:%s:Lib",    false, true },
    [type_plug]    = { "App:%s:Plug",   false, false },
    [type_public]  = { "System:Shared", false, true },
};

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Remove file and loaded rules
 *
 * @param[in] file The path of the file to remove
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur
static int remove_load_rules(const char *file) {
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
    if (value == NULL) {
        value = secure_getenv("SMACK_TEMPLATE_FILE");
        if (value == NULL)
            value = default_smack_template_file;
    }
    return value;
}

/* see smack-template.h */
const char *get_smack_policy_dir(const char *value) {
    if (value == NULL) {
        value = secure_getenv("SMACK_POLICY_DIR");
        if (value == NULL)
            value = default_smack_policy_dir;
    }
    return value;
}

int get_smack_rule_path(char rule_path[SEC_LSM_MANAGER_MAX_SIZE_PATH + 1], const char *id)
{
    int len = snprintf(rule_path, SEC_LSM_MANAGER_MAX_SIZE_PATH + 1,
                   "%s/%s.%s", get_smack_policy_dir(NULL), id, SMACK_EXTENSION);
    return len < 0 ? len : len > SEC_LSM_MANAGER_MAX_SIZE_PATH ? -ENAMETOOLONG : len;
}

/* see smack-label.h */
bool smack_enabled() {
    if (smack_smackfs_path() == NULL) {
        return false;
    }
    return true;
}

/* see smack-label.h */
void init_path_type_definitions(path_type_definitions_t path_type_definitions[number_path_type], const char *id)
{
    int idx;
    for (idx = 0 ; idx < number_path_type ; idx++) {
        snprintf(path_type_definitions[idx].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, patterns[idx].label, id);
        path_type_definitions[idx].is_executable = patterns[idx].exec;
        path_type_definitions[idx].is_transmute = patterns[idx].transmute;
    }
}

/* see smack-template.h */
int create_smack_rules(const context_t *context) {
    int rc = 0;
    int rc2 = 0;
    struct smack_accesses *smack_accesses = NULL;
    char smack_rules_file[SEC_LSM_MANAGER_MAX_SIZE_PATH + 1];
    const char *smack_template_file;

    rc = get_smack_rule_path(smack_rules_file, context->id);
    if (rc < 0) {
        ERROR("get_smack_rule_path: %d %s", -rc, strerror(-rc));
        goto end;
    }

    smack_template_file = get_smack_template_file(NULL);
    rc = template_process(smack_template_file, smack_rules_file, context);
    if (rc < 0) {
        ERROR("template_process : %d %s", -rc, strerror(-rc));
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
        goto error2;
    }

    rc = smack_accesses_add_from_file(smack_accesses, fd);
    if (rc < 0) {
        ERROR("smack_accesses_add_from_file");
        goto error2;
    }

    if (smack_enabled()) {
        rc = smack_accesses_apply(smack_accesses);
        if (rc < 0) {
            ERROR("smack_accesses_apply");
            goto error2;
        }
    }

    DEBUG("create_smack_rules success");
    close(fd);
    goto end;

error2:
    close(fd);
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
int remove_smack_rules(const context_t *context) {
    int rc = 0;
    char smack_rules_file[SEC_LSM_MANAGER_MAX_SIZE_PATH + 1];

    rc = get_smack_rule_path(smack_rules_file, context->id);
    if (rc < 0) {
        ERROR("get_smack_rule_path: %d %s", -rc, strerror(-rc));
        return rc;
    }

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
