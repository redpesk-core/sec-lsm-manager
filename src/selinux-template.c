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

#include "selinux-template.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "limits.h"
#include "log.h"
#include "selinux-compile.h"
#include "template.h"
#include "utils.h"

#if !defined(SEC_LSM_MANAGER_DATADIR)
#define SEC_LSM_MANAGER_DATADIR "/usr/share/sec-lsm-manager"
#endif

#define TE_EXTENSION "te"
#define FC_EXTENSION "fc"
#define IF_EXTENSION "if"
#define PP_EXTENSION "pp"

#if !defined(TE_TEMPLATE_FILE)
#define TE_TEMPLATE_FILE "app-template.te"
#endif

#if !defined(IF_TEMPLATE_FILE)
#define IF_TEMPLATE_FILE "app-template.if"
#endif

#if !defined(SELINUX_TE_TEMPLATE_FILE)
#define SELINUX_TE_TEMPLATE_FILE SEC_LSM_MANAGER_DATADIR "/" TE_TEMPLATE_FILE
#endif

#if !defined(SELINUX_IF_TEMPLATE_FILE)
#define SELINUX_IF_TEMPLATE_FILE SEC_LSM_MANAGER_DATADIR "/" IF_TEMPLATE_FILE
#endif

#if !defined(SELINUX_RULES_DIR)
#define SELINUX_RULES_DIR SEC_LSM_MANAGER_DATADIR "/selinux-rules"
#endif

const char default_selinux_rules_dir[] = SELINUX_RULES_DIR;
const char default_selinux_te_template_file[] = SELINUX_TE_TEMPLATE_FILE;
const char default_selinux_if_template_file[] = SELINUX_IF_TEMPLATE_FILE;

typedef struct selinux_module {
    char selinux_te_file[SEC_LSM_MANAGER_MAX_SIZE_PATH];           ///////////////////
    char selinux_if_file[SEC_LSM_MANAGER_MAX_SIZE_PATH];           //   PATH MODULE //
    char selinux_fc_file[SEC_LSM_MANAGER_MAX_SIZE_PATH];           //      FILE     //
    char selinux_pp_file[SEC_LSM_MANAGER_MAX_SIZE_PATH];           ///////////////////
    char selinux_rules_dir[SEC_LSM_MANAGER_MAX_SIZE_DIR];          // Store te, if, fc, pp files
    char selinux_te_template_file[SEC_LSM_MANAGER_MAX_SIZE_PATH];  // te base template
    char selinux_if_template_file[SEC_LSM_MANAGER_MAX_SIZE_PATH];  // if base template
} selinux_module_t;

char suffix_id[] = "_t";
char suffix_lib[] = "_lib_t";
char suffix_conf[] = "_conf_t";
char suffix_exec[] = "_exec_t";
char suffix_icon[] = "_icon_t";
char suffix_data[] = "_data_t";
char suffix_http[] = "_http_t";
char public_app[] = "redpesk_public_t";

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Init selinux module
 *
 * @param[in] selinux_module selinux module handler to init
 * @param[in] secure_app The secure_app of application
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static void init_selinux_module(selinux_module_t *selinux_module, const secure_app_t *secure_app) {
    memset(selinux_module, 0, sizeof(*selinux_module));

    strncpy(selinux_module->selinux_rules_dir, get_selinux_rules_dir(NULL), SEC_LSM_MANAGER_MAX_SIZE_DIR);

    strncpy(selinux_module->selinux_te_template_file, get_selinux_te_template_file(NULL),
            SEC_LSM_MANAGER_MAX_SIZE_PATH);

    strncpy(selinux_module->selinux_if_template_file, get_selinux_if_template_file(NULL),
            SEC_LSM_MANAGER_MAX_SIZE_PATH);

    snprintf(selinux_module->selinux_te_file, SEC_LSM_MANAGER_MAX_SIZE_PATH, "%s/%s.%s",
             selinux_module->selinux_rules_dir, secure_app->id, TE_EXTENSION);

    snprintf(selinux_module->selinux_fc_file, SEC_LSM_MANAGER_MAX_SIZE_PATH, "%s/%s.%s",
             selinux_module->selinux_rules_dir, secure_app->id, FC_EXTENSION);

    snprintf(selinux_module->selinux_if_file, SEC_LSM_MANAGER_MAX_SIZE_PATH, "%s/%s.%s",
             selinux_module->selinux_rules_dir, secure_app->id, IF_EXTENSION);

    snprintf(selinux_module->selinux_pp_file, SEC_LSM_MANAGER_MAX_SIZE_PATH, "%s/%s.%s",
             selinux_module->selinux_rules_dir, secure_app->id, PP_EXTENSION);
}

/**
 * @brief Generate the fc file
 *
 * @param[in] selinux_fc_file The selinux destination fc file
 * @param[in] secure_app secure_app handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int generate_app_module_fc(const char *selinux_fc_file, const secure_app_t *secure_app,
                                                    path_type_definitions_t path_type_definitions[number_path_type]) {
    int rc = 0;

    FILE *f_module_fc = fopen(selinux_fc_file, "w");

    if (f_module_fc == NULL) {
        rc = -errno;
        ERROR("fopen %s %m", selinux_fc_file);
        goto ret;
    }

    path_t *path;
    char line[SEC_LSM_MANAGER_MAX_SIZE_LINE_MODULE];
    for (size_t i = 0; i < secure_app->path_set.size; i++) {
        path = secure_app->path_set.paths + i;
        snprintf(line, SEC_LSM_MANAGER_MAX_SIZE_LINE_MODULE, "%s(/.*)? gen_context(%s,s0)\n", path->path,
                 path_type_definitions[path->path_type].label);

        rc = fputs(line, f_module_fc);
        if (rc < 0) {
            rc = -errno;
            ERROR("fputs %m");
            goto error;
        }
    }

error:
    rc = fclose(f_module_fc);
    if (rc < 0) {
        rc = -errno;
        ERROR("Fail fclose %m");
    }
ret:
    return rc;
}

/**
 * @brief Generate te, if, fc files
 *
 * @param[in] selinux_module selinux module handler
 * @param[in] secure_app secure app handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur
    static int generate_app_module_files(const selinux_module_t *selinux_module, const secure_app_t *secure_app,
                                         path_type_definitions_t path_type_definitions[number_path_type]) {
    int rc = process_template(selinux_module->selinux_te_template_file, selinux_module->selinux_te_file, secure_app);
    if (rc < 0) {
        ERROR("process_template : %s -> %s", selinux_module->selinux_te_template_file, selinux_module->selinux_te_file);
        goto ret;
    }

    rc = process_template(selinux_module->selinux_if_template_file, selinux_module->selinux_if_file, secure_app);
    if (rc < 0) {
        ERROR("process_template : %s -> %s", selinux_module->selinux_if_template_file, selinux_module->selinux_if_file);
        goto remove_te;
    }

    rc = generate_app_module_fc(selinux_module->selinux_fc_file, secure_app, path_type_definitions);
    if (rc < 0) {
        ERROR("generate_app_module_fc : %s", selinux_module->selinux_fc_file);
        goto remove_if;
    }

    DEBUG("success generate selinux module files");

    goto ret;

remove_if:
    if (remove_file(selinux_module->selinux_if_file) < 0) {
        ERROR("remove file : %s", selinux_module->selinux_if_file);
    }
remove_te:
    if (remove_file(selinux_module->selinux_te_file) < 0) {
        ERROR("remove file : %s", selinux_module->selinux_te_file);
    }
ret:
    return rc;
}

/**
 * @brief Check te, fc, if file exists
 *
 * @param[in] selinux_module selinux module handler
 * @return true if all exist
 * @return false if not
 */
__nonnull() __wur static bool check_app_module_files_exists(const selinux_module_t *selinux_module) {
    if (!check_file_exists(selinux_module->selinux_te_file))
        return false;
    if (!check_file_exists(selinux_module->selinux_fc_file))
        return false;
    if (!check_file_exists(selinux_module->selinux_if_file))
        return false;

    return true;
}

/**
 * @brief Remove app module files (te, fc, if)
 *
 * @param[in] selinux_module selinux module handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int remove_app_module_files(const selinux_module_t *selinux_module) {
    int rc = remove_file(selinux_module->selinux_te_file);
    if (rc < 0) {
        ERROR("remove_file : %s", selinux_module->selinux_te_file);
        goto end;
    }

    rc = remove_file(selinux_module->selinux_if_file);
    if (rc < 0) {
        ERROR("remove_file : %s", selinux_module->selinux_if_file);
        goto end;
    }

    rc = remove_file(selinux_module->selinux_fc_file);
    if (rc < 0) {
        ERROR("remove_file : %s", selinux_module->selinux_fc_file);
        goto end;
    }

    goto end;

end:
    return rc;
}

/**
 * @brief Remove pp file
 *
 * @param[in] selinux_module selinux_module handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int remove_pp_file(const selinux_module_t *selinux_module) {
    int rc = remove_file(selinux_module->selinux_pp_file);
    if (rc < 0) {
        ERROR("remove_file : %s", selinux_module->selinux_pp_file);
    }
    return rc;
}

/**
 * @brief Destroy semanage handle and content
 *
 * @param[in] semanage_handle semanage_handle handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int destroy_semanage_handle(semanage_handle_t *semanage_handle) {
    int rc = 0;

    if (semanage_is_connected(semanage_handle)) {
        rc = semanage_disconnect(semanage_handle);
        if (rc < 0) {
            ERROR("semanage_disconnect");
        }
    }
    semanage_handle_destroy(semanage_handle);

    return rc;
}

/**
 * @brief Create a semanage handle
 *
 * @param[out] semanage_handle pointer semanage_handle handler
 * @return 0 in case of success or a negative -errno value
 */
__wur static int create_semanage_handle(semanage_handle_t **semanage_handle) {
    int rc = 0;
    *semanage_handle = semanage_handle_create();

    if (semanage_handle == NULL) {
        ERROR("semanage_handle_create");
        rc = -1;
        goto ret;
    }

    semanage_set_create_store(*semanage_handle, 1);

    rc = semanage_connect(*semanage_handle);
    if (rc < 0) {
        ERROR("semanage_connect");

        goto error;
    }

    rc = semanage_set_default_priority(*semanage_handle, 400);
    if (rc != 0) {
        ERROR("semanage_set_default_priority");
        goto error;
    }

    goto ret;

error:
    if (destroy_semanage_handle(*semanage_handle) < 0) {
        ERROR("destroy_semanage_handle");
    }
ret:
    return rc;
}

/**
 * @brief Install selinux module
 *
 * @param[in] semanage_handle semanage_handle handler
 * @param[in] selinux_pp_file Path of the pp file
 * @return 0 in case of success or a negative -errno value
 */
__nonnull((1)) __wur static int install_module(semanage_handle_t *semanage_handle, const char *selinux_pp_file) {
    int rc = semanage_module_install_file(semanage_handle, selinux_pp_file);
    if (rc < 0) {
        ERROR("semanage_module_install_file : %s", selinux_pp_file);
        goto end;
    }

    rc = semanage_commit(semanage_handle);
    if (rc < 0) {
        ERROR("semanage_commit : install_module %s", selinux_pp_file);
        goto end;
    }

end:
    return rc;
}

/**
 * @brief Remove module in the policy
 *
 * @param[in] semanage_handle semanage handle handler
 * @param[in] module_name Module name in the selinux policy
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static int remove_module(semanage_handle_t *semanage_handle, const char *module_name) {
    char *module_name_ = strdupa(module_name);  // semanage_module_remove take no const module name
    int rc = 0;
    if (module_name_ == NULL) {
        ERROR("strdupa : %s", module_name);
        rc = -ENOMEM;
        goto end;
    }

    rc = semanage_module_remove(semanage_handle, module_name_);
    if (rc < 0) {
        ERROR("semanage_module_remove : %s", module_name_);
        goto end;
    }

    rc = semanage_commit(semanage_handle);
    if (rc < 0) {
        ERROR("semanage_commit : remove module %s", module_name_);
        goto end;
    }

end:
    return rc;
}

/**
 * @brief Free semanage_module_info_list
 *
 * @param[in] semanage_handle semanage_handle handler
 * @param[in] semanage_module_info_list semanage_module_info_list handler
 * @param[in] semanage_module_info_len semanage_module_info_len handler
 */
__nonnull() static void free_module_info_list(semanage_handle_t *semanage_handle,
                                              semanage_module_info_t *semanage_module_info_list,
                                              int semanage_module_info_len) {
    semanage_module_info_t *semanage_module_info = NULL;
    for (int i = 0; i < semanage_module_info_len; i++) {
        semanage_module_info = semanage_module_list_nth(semanage_module_info_list, i);
        semanage_module_info_destroy(semanage_handle, semanage_module_info);
    }
    free(semanage_module_info_list);
}

/**
 * @brief Check module in selinux policy
 *
 * @param[in] semanage_handle semanage_handle handler
 * @param[in] id name of the module
 * @return true if exists, false else
 */
__nonnull() __wur static bool check_module(semanage_handle_t *semanage_handle, const char *id) {
    semanage_module_info_t *semanage_module_info = NULL;
    semanage_module_info_t *semanage_module_info_list = NULL;
    int semanage_module_info_len = 0;
    bool ret = false;

    int rc = semanage_module_list(semanage_handle, &semanage_module_info_list, &semanage_module_info_len);
    if (rc < 0) {
        ERROR("semanage_module_list");
        goto end;
    }

    for (int i = 0; i < semanage_module_info_len; i++) {
        semanage_module_info = semanage_module_list_nth(semanage_module_info_list, i);
        const char *module_name = NULL;
        rc = semanage_module_info_get_name(semanage_handle, semanage_module_info, &module_name);
        if (rc < 0) {
            ERROR("semanage_module_info_get_name");
            goto end;
        }

        if (!strcmp(module_name, id)) {
            ret = true;
            goto end;
        }
    }
end:
    free_module_info_list(semanage_handle, semanage_module_info_list, semanage_module_info_len);
    return ret;
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see selinux-template.h */
const char *get_selinux_te_template_file(const char *value) {
    return value ?: secure_getenv("SELINUX_TE_TEMPLATE_FILE") ?: default_selinux_te_template_file;
}

/* see selinux-template.h */
const char *get_selinux_if_template_file(const char *value) {
    return value ?: secure_getenv("SELINUX_IF_TEMPLATE_FILE") ?: default_selinux_if_template_file;
}

/* see selinux-template.h */
const char *get_selinux_rules_dir(const char *value) {
    value = value ?: secure_getenv("SELINUX_RULES_DIR") ?: default_selinux_rules_dir;
    if (strlen(value) >= SEC_LSM_MANAGER_MAX_SIZE_DIR) {
        value = NULL;
        ERROR("selinux_rules_dir too long");
    }
    return value;
}

/* see selinux-label.h */
void init_path_type_definitions(path_type_definitions_t path_type_definitions[number_path_type], const char *id) {
    snprintf(path_type_definitions[type_conf].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "system_u:object_r:%s%s", id,
             suffix_conf);
    snprintf(path_type_definitions[type_data].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "system_u:object_r:%s%s", id,
             suffix_data);
    snprintf(path_type_definitions[type_exec].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "system_u:object_r:%s%s", id,
             suffix_exec);
    snprintf(path_type_definitions[type_http].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "system_u:object_r:%s%s", id,
             suffix_http);
    snprintf(path_type_definitions[type_icon].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "system_u:object_r:%s%s", id,
             suffix_icon);
    snprintf(path_type_definitions[type_id].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "system_u:object_r:%s%s", id,
             suffix_id);
    snprintf(path_type_definitions[type_lib].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "system_u:object_r:%s%s", id,
             suffix_lib);
    snprintf(path_type_definitions[type_public].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "system_u:object_r:%s",
             public_app);
}

/* see selinux-template.h */
int create_selinux_rules(const secure_app_t *secure_app,
                         path_type_definitions_t path_type_definitions[number_path_type]) {
    selinux_module_t selinux_module;
    init_selinux_module(&selinux_module, secure_app);

    semanage_handle_t *semanage_handle;
    int rc = create_semanage_handle(&semanage_handle);
    if (rc < 0) {
        ERROR("create_semanage_handle");
        goto ret;
    }

    // Generate files
    rc = generate_app_module_files(&selinux_module, secure_app, path_type_definitions);
    if (rc < 0) {
        ERROR("generate_app_module_files");
        goto end2;
    }

    DEBUG("success generate selinux files module");

    // fc, if, te generated
    rc = launch_compile(secure_app->id);
    if (rc < 0) {
        ERROR("launch_compile");
        goto error3;
    }

    DEBUG("success compile selinux module");

    // pp generated

    rc = install_module(semanage_handle, selinux_module.selinux_pp_file);

    if (rc < 0) {
        ERROR("install_module");
        goto error4;
    }

    DEBUG("success install module");

    goto end2;

error4:
    remove_pp_file(&selinux_module);
error3:
    remove_app_module_files(&selinux_module);
end2:
    rc = destroy_semanage_handle(semanage_handle);
    if (rc < 0) {
        ERROR("destroy_semanage_handle");
    }
ret:
    return rc;
}

/* see selinux-template.h */
bool check_module_files_exist(const secure_app_t *secure_app) {
    bool ret = false;
    selinux_module_t selinux_module;
    init_selinux_module(&selinux_module, secure_app);

    ret = check_app_module_files_exists(&selinux_module);

    return ret;
}

/* see selinux-template.h */
bool check_module_in_policy(const secure_app_t *secure_app) {
    semanage_handle_t *semanage_handle;
    bool ret = false;
    int rc = create_semanage_handle(&semanage_handle);
    if (rc < 0) {
        ERROR("create_semanage_handle");
        goto end;
    }

    ret = check_module(semanage_handle, secure_app->id);

    if (destroy_semanage_handle(semanage_handle) < 0) {
        ERROR("destroy_semanage_handle");
    }

end:
    return ret;
}

/* see selinux-template.h */
int remove_selinux_rules(const secure_app_t *secure_app) {
    // remove files
    selinux_module_t selinux_module;
    init_selinux_module(&selinux_module, secure_app);

    int rc = remove_app_module_files(&selinux_module) + remove_pp_file(&selinux_module);
    if (rc < 0) {
        ERROR("remove files modules");
        goto ret;
    }

    DEBUG("success remove selinux files");

    // remove module in policy
    semanage_handle_t *semanage_handle;
    rc = create_semanage_handle(&semanage_handle);
    if (rc < 0) {
        ERROR("create_semanage_handle");
        goto ret;
    }
    rc = remove_module(semanage_handle, secure_app->id);
    if (rc < 0) {
        ERROR("remove_module");
        goto end2;
    }

    DEBUG("success remove selinux module");

    goto end2;

end2:
    rc = destroy_semanage_handle(semanage_handle);
    if (rc < 0) {
        ERROR("destroy_semanage_handle");
    }
ret:
    return rc;
}
