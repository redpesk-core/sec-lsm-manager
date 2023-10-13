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

#include "selinux-template.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sizes.h"
#include "log.h"
#include "selinux-compile.h"
#include "templating/template.h"
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

const char suffix_id[] = "_t";
const char suffix_lib[] = "_lib_t";
const char suffix_conf[] = "_conf_t";
const char suffix_exec[] = "_exec_t";
const char suffix_icon[] = "_icon_t";
const char suffix_data[] = "_data_t";
const char suffix_http[] = "_http_t";
const char public_app[] = "redpesk_public_t";

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Init selinux module
 *
 * @param[in] selinux_module selinux module handler to init
 * @param[in] context The context of application
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() static void init_selinux_module(selinux_module_t *selinux_module, const context_t *context) {
    memset(selinux_module, 0, sizeof(*selinux_module));

    /* TODO: treat the case type_default or treat when label is "" */

    secure_strncpy(selinux_module->selinux_rules_dir, get_selinux_rules_dir(NULL), SEC_LSM_MANAGER_MAX_SIZE_DIR);

    secure_strncpy(selinux_module->selinux_te_template_file, get_selinux_te_template_file(NULL),
                   SEC_LSM_MANAGER_MAX_SIZE_PATH);

    secure_strncpy(selinux_module->selinux_if_template_file, get_selinux_if_template_file(NULL),
                   SEC_LSM_MANAGER_MAX_SIZE_PATH);

    snprintf(selinux_module->selinux_te_file, SEMOD_MAX_SIZE_PATH, "%s/%s.%s",
             selinux_module->selinux_rules_dir, context->id, TE_EXTENSION);

    snprintf(selinux_module->selinux_fc_file, SEMOD_MAX_SIZE_PATH, "%s/%s.%s",
             selinux_module->selinux_rules_dir, context->id, FC_EXTENSION);

    snprintf(selinux_module->selinux_if_file, SEMOD_MAX_SIZE_PATH, "%s/%s.%s",
             selinux_module->selinux_rules_dir, context->id, IF_EXTENSION);

    snprintf(selinux_module->selinux_pp_file, SEMOD_MAX_SIZE_PATH, "%s/%s.%s",
             selinux_module->selinux_rules_dir, context->id, PP_EXTENSION);
}

/**
 * @brief Generate the fc file
 *
 * @param[in] selinux_fc_file The selinux destination fc file
 * @param[in] context context handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur
int generate_app_module_fc(const char *selinux_fc_file, const context_t *context,
                                                    path_type_definitions_t path_type_definitions[number_path_type]) {
    int rc = 0;
    int rc2 = 0;

    FILE *f_module_fc = fopen(selinux_fc_file, "w");

    if (f_module_fc == NULL) {
        rc = -errno;
        ERROR("fopen %s : %d %s", selinux_fc_file, -rc, strerror(-rc));
        goto ret;
    }

    path_t *path;
    char line[SEC_LSM_MANAGER_MAX_SIZE_LINE_MODULE];
    for (size_t i = 0; i < context->path_set.size; i++) {
        path = context->path_set.paths[i];
        snprintf(line, SEC_LSM_MANAGER_MAX_SIZE_LINE_MODULE, "%s(/.*)? gen_context(%s,s0)\n", path->path,
                 path_type_definitions[path->path_type].label);

        rc = fputs(line, f_module_fc);
        if (rc < 0) {
            rc = -errno;
            ERROR("fputs : %d %s", -rc, strerror(-rc));
            goto error;
        }
    }

    rc = 0;

error:
    rc2 = fclose(f_module_fc);
    if (rc2 < 0) {
        ERROR("fclose : %d %s", errno, strerror(errno));
    }
ret:
    return rc;
}

/**
 * @brief Generate te, if, fc files
 *
 * @param[in] selinux_module selinux module handler
 * @param[in] context context handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur
int generate_app_module_files(const selinux_module_t *selinux_module, const context_t *context,
                                         path_type_definitions_t path_type_definitions[number_path_type]) {
    int rc = 0;
    int rc2 = 0;
    rc = process_template(selinux_module->selinux_te_template_file, selinux_module->selinux_te_file, context);
    if (rc < 0) {
        ERROR("process_template %s -> %s : %d %s", selinux_module->selinux_te_template_file,
              selinux_module->selinux_te_file, -rc, strerror(-rc));
        goto ret;
    }

    rc = process_template(selinux_module->selinux_if_template_file, selinux_module->selinux_if_file, context);
    if (rc < 0) {
        ERROR("process_template %s -> %s : %d %s", selinux_module->selinux_if_template_file,
              selinux_module->selinux_if_file, -rc, strerror(-rc));
        goto remove_te;
    }

    rc = generate_app_module_fc(selinux_module->selinux_fc_file, context, path_type_definitions);
    if (rc < 0) {
        ERROR("generate_app_module_fc %s : %d %s", selinux_module->selinux_fc_file, -rc, strerror(-rc));
        goto remove_if;
    }

    DEBUG("success generate selinux module files");

    goto ret;

remove_if:
    rc2 = remove_file(selinux_module->selinux_if_file);
    if (rc2 < 0) {
        ERROR("remove file %s : %d %s", selinux_module->selinux_if_file, -rc2, strerror(-rc2));
    }
remove_te:
    rc2 = remove_file(selinux_module->selinux_te_file);
    if (rc < 0) {
        ERROR("remove file %s : %d %s", selinux_module->selinux_te_file, -rc2, strerror(-rc2));
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
    bool exists;
    int sum = 0;
    get_file_informations(selinux_module->selinux_te_file, &exists, NULL, NULL);
    sum += exists;
    get_file_informations(selinux_module->selinux_fc_file, &exists, NULL, NULL);
    sum += exists;
    get_file_informations(selinux_module->selinux_if_file, &exists, NULL, NULL);
    sum += exists;

    if (sum != 3) {
        return false;
    }

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
        ERROR("remove_file %s : %d %s", selinux_module->selinux_te_file, -rc, strerror(-rc));
        goto error;
    }

    rc = remove_file(selinux_module->selinux_if_file);
    if (rc < 0) {
        ERROR("remove_file %s : %d %s", selinux_module->selinux_if_file, -rc, strerror(-rc));
        goto error;
    }

    rc = remove_file(selinux_module->selinux_fc_file);
    if (rc < 0) {
        ERROR("remove_file %s : %d %s", selinux_module->selinux_fc_file, -rc, strerror(-rc));
    }

error:
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
        ERROR("remove_file %s : %d %s", selinux_module->selinux_pp_file, -rc, strerror(-rc));
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
            rc = -errno;
            ERROR("semanage_disconnect : %d %s", -rc, strerror(-rc));
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
    int rc2 = 0;
    *semanage_handle = semanage_handle_create();

    if (semanage_handle == NULL) {
        rc = -errno;
        ERROR("semanage_handle_create : %d %s", -rc, strerror(-rc));
        goto ret;
    }

    semanage_set_create_store(*semanage_handle, 1);

    rc = semanage_connect(*semanage_handle);
    if (rc < 0) {
        rc = -errno;
        ERROR("semanage_connect : %d %s", -rc, strerror(-rc));
        goto error;
    }

    rc = semanage_set_default_priority(*semanage_handle, 400);
    if (rc != 0) {
        rc = -errno;
        ERROR("semanage_set_default_priority : %d %s", -rc, strerror(-rc));
        goto error;
    }

    goto ret;

error:
    if (destroy_semanage_handle(*semanage_handle) < 0) {
        rc2 = -errno;
        ERROR("destroy_semanage_handle : %d %s", -rc2, strerror(-rc2));
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
        rc = -errno;
        ERROR("semanage_module_install_file %s : %d %s", selinux_pp_file, -rc, strerror(-rc));
        goto end;
    }

    rc = semanage_commit(semanage_handle);
    if (rc < 0) {
        rc = -errno;
        ERROR("semanage_commit (install_module %s) : %d %s", selinux_pp_file, -rc, strerror(-rc));
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
    int rc = 0;
    char *module_name_ = strdupa(module_name);  // semanage_module_remove take no const module name
    if (module_name_ == NULL) {
        rc = -ENOMEM;
        ERROR("strdupa %s : %d %s", module_name, -rc, strerror(-rc));
        goto end;
    }

    rc = semanage_module_remove(semanage_handle, module_name_);
    if (rc < 0) {
        rc = -errno;
        ERROR("semanage_module_remove %s : %d %s", module_name_, -rc, strerror(-rc));
        goto end;
    }

    rc = semanage_commit(semanage_handle);
    if (rc < 0) {
        rc = -errno;
        ERROR("semanage_commit (remove module %s) : %d %s", module_name_, -rc, strerror(-rc));
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
    bool ret = false;
    int semanage_module_info_len = 0;
    semanage_module_info_t *semanage_module_info = NULL;
    semanage_module_info_t *semanage_module_info_list = NULL;

    int rc = semanage_module_list(semanage_handle, &semanage_module_info_list, &semanage_module_info_len);
    if (rc < 0) {
        rc = -errno;
        ERROR("semanage_module_list : %d %s", -rc, strerror(-rc));
        goto end;
    }

    for (int i = 0; i < semanage_module_info_len; i++) {
        semanage_module_info = semanage_module_list_nth(semanage_module_info_list, i);
        const char *module_name = NULL;
        rc = semanage_module_info_get_name(semanage_handle, semanage_module_info, &module_name);
        if (rc < 0) {
            rc = -errno;
            ERROR("semanage_module_info_get_name : %d %s", -rc, strerror(-rc));
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
    if (value == NULL) {
        value = secure_getenv("SELINUX_TE_TEMPLATE_FILE");
        if (value == NULL)
            value = default_selinux_te_template_file;
    }
    return value;
}

/* see selinux-template.h */
const char *get_selinux_if_template_file(const char *value) {
    if (value == NULL) {
        value = secure_getenv("SELINUX_IF_TEMPLATE_FILE");
        if (value == NULL)
            value = default_selinux_if_template_file;
    }
    return value;
}

/* see selinux-template.h */
const char *get_selinux_rules_dir(const char *value) {
    if (value == NULL) {
        value = secure_getenv("SELINUX_RULES_DIR");
        if (value == NULL)
            value = default_selinux_rules_dir;
    }
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
    snprintf(path_type_definitions[type_plug].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "system_u:object_r:%s",
             public_app); /* TODO: plug is ATM mapped as public */
    snprintf(path_type_definitions[type_public].label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "system_u:object_r:%s",
             public_app);
}

/* see selinux-template.h */
int create_selinux_rules(const context_t *context,
                         path_type_definitions_t path_type_definitions[number_path_type]) {
    int rc = 0;
    int rc2 = 0;
    selinux_module_t selinux_module;
    init_selinux_module(&selinux_module, context);

    semanage_handle_t *semanage_handle;
    rc = create_semanage_handle(&semanage_handle);
    if (rc < 0) {
        ERROR("create_semanage_handle : %d %s", -rc, strerror(-rc));
        goto ret;
    }

    // Generate files
    rc = generate_app_module_files(&selinux_module, context, path_type_definitions);
    if (rc < 0) {
        ERROR("generate_app_module_files : %d %s", -rc, strerror(-rc));
        goto end2;
    }

    DEBUG("success generate selinux files module");

    // fc, if, te generated
    rc = launch_compile(context->id);
    if (rc < 0) {
        ERROR("launch_compile : %d %s", -rc, strerror(-rc));
        goto error3;
    }

    DEBUG("success compile selinux module");

    // pp generated

    rc = install_module(semanage_handle, selinux_module.selinux_pp_file);
    if (rc < 0) {
        ERROR("install_module : %d %s", -rc, strerror(-rc));
        goto error4;
    }

    DEBUG("success install module");

    goto end2;

error4:
    rc2 = remove_pp_file(&selinux_module);
    if (rc2 < 0) {
        ERROR("remove_pp_file : %d %s", -rc2, strerror(-rc2));
    }
error3:
    rc2 = remove_app_module_files(&selinux_module);
    if (rc2 < 0) {
        ERROR("remove_app_module_files : %d %s", -rc2, strerror(-rc2));
    }
end2:
    rc2 = destroy_semanage_handle(semanage_handle);
    if (rc2 < 0) {
        ERROR("destroy_semanage_handle : %d %s", -rc2, strerror(-rc2));
    }
ret:
    return rc;
}

/* see selinux-template.h */
bool check_module_files_exist(const context_t *context) {
    selinux_module_t selinux_module;
    init_selinux_module(&selinux_module, context);
    return check_app_module_files_exists(&selinux_module);
}

/* see selinux-template.h */
bool check_module_in_policy(const context_t *context) {
    bool ret = false;
    semanage_handle_t *semanage_handle;
    int rc = create_semanage_handle(&semanage_handle);
    if (rc < 0) {
        ERROR("create_semanage_handle : %d %s", -rc, strerror(-rc));
        goto end;
    }

    ret = check_module(semanage_handle, context->id);

    rc = destroy_semanage_handle(semanage_handle);
    if (rc < 0) {
        ERROR("destroy_semanage_handle : %d %s", -rc, strerror(-rc));
    }

end:
    return ret;
}

/* see selinux-template.h */
int remove_selinux_rules(const context_t *context) {
    int rc = 0;
    int rc2 = 0;
    selinux_module_t selinux_module;
    init_selinux_module(&selinux_module, context);

    // remove files
    rc = remove_app_module_files(&selinux_module);
    if (rc < 0) {
        ERROR("remove_app_module_files : %d %s", -rc, strerror(-rc));
        goto ret;
    }

    rc = remove_pp_file(&selinux_module);
    if (rc < 0) {
        ERROR("remove_pp_file : %d %s", -rc, strerror(-rc));
        goto ret;
    }

    DEBUG("success remove selinux files");

    // remove module in policy
    semanage_handle_t *semanage_handle;
    rc = create_semanage_handle(&semanage_handle);
    if (rc < 0) {
        ERROR("create_semanage_handle : %d %s", -rc, strerror(-rc));
        goto ret;
    }
    rc = remove_module(semanage_handle, context->id);
    if (rc < 0) {
        ERROR("remove_module : %d %s", -rc, strerror(-rc));
        goto end;
    }

    DEBUG("success remove selinux module");

    goto end;

end:
    rc2 = destroy_semanage_handle(semanage_handle);
    if (rc2 < 0) {
        ERROR("destroy_semanage_handle : %d %s", -rc2, strerror(-rc2));
    }
ret:
    return rc;
}
