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

#include "log.h"
#include "selinux-compile.h"
#include "selinux-label.h"
#include "utils.h"

#define MAX_LINE_SIZE_MODULE 2048

#define REPLACE_APP "~APP~"
#define REPLACE_ID "~ID~"

#if !defined(DEFAULT_TEMPLATE_DIR)
#define DEFAULT_TEMPLATE_DIR "/usr/share/sec-lsm-manager/"
#endif

#define SIZE_EXTENSION 3
#define TE_EXTENSION ".te"
#define FC_EXTENSION ".fc"
#define IF_EXTENSION ".if"
#define PP_EXTENSION ".pp"

#if !defined(DEFAULT_TE_TEMPLATE_FILE)
#define DEFAULT_TE_TEMPLATE_FILE "app-template.te"
#endif

#if !defined(DEFAULT_IF_TEMPLATE_FILE)
#define DEFAULT_IF_TEMPLATE_FILE "app-template.if"
#endif

#if !defined(DEFAULT_SELINUX_TE_TEMPLATE_FILE)
#define DEFAULT_SELINUX_TE_TEMPLATE_FILE DEFAULT_TEMPLATE_DIR DEFAULT_TE_TEMPLATE_FILE
#endif

#if !defined(DEFAULT_SELINUX_IF_TEMPLATE_FILE)
#define DEFAULT_SELINUX_IF_TEMPLATE_FILE DEFAULT_TEMPLATE_DIR DEFAULT_IF_TEMPLATE_FILE
#endif

#if !defined(DEFAULT_SELINUX_RULES_DIR)
#define DEFAULT_SELINUX_RULES_DIR "/usr/share/sec-lsm-manager/selinux-policy/"
#endif

const char default_selinux_rules_dir[] = DEFAULT_SELINUX_RULES_DIR;
const char default_selinux_te_template_file[] = DEFAULT_SELINUX_TE_TEMPLATE_FILE;
const char default_selinux_if_template_file[] = DEFAULT_SELINUX_IF_TEMPLATE_FILE;

typedef struct selinux_module {
    char *id;                              // my-id
    char *selinux_id;                      // my_id
    char *selinux_te_file;                 ///////////////////
    char *selinux_if_file;                 //   PATH MODULE //
    char *selinux_fc_file;                 //      FILE     //
    char *selinux_pp_file;                 ///////////////////
    const char *selinux_rules_dir;         // Store te, if, fc, pp files
    const char *selinux_te_template_file;  // te base template
    const char *selinux_if_template_file;  // if base template
} selinux_module_t;

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Free selinux_pp_file, selinux_if_file, selinux_fc_file, selinux_te_file
 *
 * @param[in] selinux_module selinux_module handler
 */
__nonnull() static void free_path_module_files(selinux_module_t *selinux_module) {
    if (selinux_module) {
        free(selinux_module->selinux_pp_file);
        selinux_module->selinux_pp_file = NULL;
        free(selinux_module->selinux_if_file);
        selinux_module->selinux_if_file = NULL;
        free(selinux_module->selinux_fc_file);
        selinux_module->selinux_fc_file = NULL;
        free(selinux_module->selinux_te_file);
        selinux_module->selinux_te_file = NULL;
    }
}

/**
 * @brief Free all fields of selinux modules or set to NULL
 * The pointer is not free
 * @param[in] selinux_module selinux_module handler
 */
__nonnull() static void free_selinux_module(selinux_module_t *selinux_module) {
    if (selinux_module) {
        free(selinux_module->id);
        selinux_module->id = NULL;
        free(selinux_module->selinux_id);
        selinux_module->selinux_id = NULL;
        free_path_module_files(selinux_module);
        selinux_module->selinux_rules_dir = NULL;
        selinux_module->selinux_te_template_file = NULL;
        selinux_module->selinux_if_template_file = NULL;
    }
}

__nonnull((2, 3, 4)) static int generate_path_module_file(char **dest, const char *selinux_rules_dir, const char *id,
                                                          const char *extension) __wur {
    size_t len = strlen(selinux_rules_dir) + strlen(id) + strlen(extension) + 1;

    *dest = (char *)malloc(len);
    if (!(*dest)) {
        ERROR("malloc selinux_te_file");
        return -ENOMEM;
    }
    memset(*dest, 0, len);
    strcpy(*dest, selinux_rules_dir);
    strcat(*dest, id);
    strcat(*dest, extension);

    return 0;
}

/**
 * @brief Allocate selinux_pp_file, selinux_if_file, selinux_fc_file, selinux_te_file
 *
 * @param[in] selinux_module selinux_module handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() static int generate_path_module_files(selinux_module_t *selinux_module) __wur {
    int rc = generate_path_module_file(&selinux_module->selinux_te_file, selinux_module->selinux_rules_dir,
                                       selinux_module->id, TE_EXTENSION);
    if (rc < 0) {
        ERROR("generate_path_module_file");
        goto error;
    }

    rc = generate_path_module_file(&selinux_module->selinux_fc_file, selinux_module->selinux_rules_dir,
                                   selinux_module->id, FC_EXTENSION);
    if (rc < 0) {
        ERROR("generate_path_module_file");
        goto error;
    }

    rc = generate_path_module_file(&selinux_module->selinux_if_file, selinux_module->selinux_rules_dir,
                                   selinux_module->id, IF_EXTENSION);
    if (rc < 0) {
        ERROR("generate_path_module_file");
        goto error;
    }

    rc = generate_path_module_file(&selinux_module->selinux_pp_file, selinux_module->selinux_rules_dir,
                                   selinux_module->id, PP_EXTENSION);
    if (rc < 0) {
        ERROR("generate_path_module_file");
        goto error;
    }

    goto ret;
error:
    free_path_module_files(selinux_module);
ret:
    return rc;
}

/**
 * @brief Turns dash into underscore
 *
 * @param[in] s String to parse
 */
__nonnull() static void dash_to_underscore(char *s) {
    while (*s) {
        if (*s == '-') {
            *s = '_';
        }
        s++;
    }
}

/**
 * @brief Init selinux module
 *
 * @param[in] selinux_module selinux module handler to init
 * @param[in] id The id of application
 * @param[in] selinux_te_template_file some value or NULL for getting default
 * @param[in] selinux_if_template_file some value or NULL for getting default
 * @param[in] selinux_rules_dir some value or NULL for getting default
 * @return 0 in case of success or a negative -errno value
 */
__nonnull((1, 2)) static int init_selinux_module(selinux_module_t *selinux_module, const char *id,
                                                 const char *selinux_te_template_file,
                                                 const char *selinux_if_template_file,
                                                 const char *selinux_rules_dir) __wur {
    int rc = 0;
    memset(selinux_module, 0, sizeof(*selinux_module));

    // defined paths
    selinux_module->selinux_rules_dir = get_selinux_rules_dir(selinux_rules_dir);
    selinux_module->selinux_te_template_file = get_selinux_te_template_file(selinux_te_template_file);
    selinux_module->selinux_if_template_file = get_selinux_if_template_file(selinux_if_template_file);

    // id
    selinux_module->id = strdup(id);
    if (selinux_module->id == NULL) {
        ERROR("strdup id");
        rc = -ENOMEM;
        goto error;
    }

    // id with underscore
    selinux_module->selinux_id = strdup(id);
    if (selinux_module->selinux_id == NULL) {
        ERROR("strdup selinux id");
        rc = -ENOMEM;
        goto error;
    }
    dash_to_underscore(selinux_module->selinux_id);

    // path of the module that will be created
    rc = generate_path_module_files(selinux_module);
    if (rc < 0) {
        ERROR("generate_path_module_files");
        goto error;
    }

    goto end;

error:
    free_selinux_module(selinux_module);
end:
    return rc;
}

/**
 * @brief Parse a line. ~APP~ turns into selinux_id and ~ID~ into id
 *
 * @param[in,out] line The line to parse
 * @param[in] id The id of the application
 * @param[in] selinux_id The selinux id of the application
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() static int parse_line(const char *line, const char *id, const char *selinux_id) __wur {
    int rc = 0;

    char *pos_str;
    char after[MAX_LINE_SIZE_MODULE];

    // Replace ~ID~
    while ((pos_str = strstr(line, REPLACE_ID))) {
        strcpy(after, pos_str + strlen(REPLACE_ID));  // save overwrite data
        strcpy(pos_str, id);
        strcpy(pos_str + strlen(id), after);
    }

    // Replace ~APP~
    while ((pos_str = strstr(line, REPLACE_APP))) {
        strcpy(after, pos_str + strlen(REPLACE_APP));  // save overwrite data
        strcpy(pos_str, selinux_id);
        strcpy(pos_str + strlen(selinux_id), after);
    }

    return rc;
}

/**
 * @brief Transforms the template into a module
 *
 * @param[in] template The path of a selinux template
 * @param[in] module The path of the module
 * @param[in] id The id of the application
 * @param[in] selinux_id The selinux id of the application
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() static int template_to_module(const char *template, const char *module, const char *id,
                                          const char *selinux_id) __wur {
    int rc = 0;
    char line[MAX_LINE_SIZE_MODULE];

    FILE *f_template = fopen(template, "r");

    if (f_template == NULL) {
        rc = -errno;
        ERROR("fopen %s %m", template);
        goto ret;
    }

    FILE *f_module = fopen(module, "w");

    if (f_module == NULL) {
        rc = -errno;
        ERROR("fopen %s %m", module);
        goto error1;
    }

    while (fgets(line, MAX_LINE_SIZE_MODULE, f_template)) {
        rc = parse_line(line, id, selinux_id);
        if (rc < 0) {
            ERROR("parse_line");
            goto error2;
        }

        rc = fputs(line, f_module);
        if (rc < 0) {
            rc = -errno;
            ERROR("fputs %m");
            goto error2;
        }
    }

error2:
    rc = fclose(f_module);
    if (rc < 0) {
        rc = -errno;
        ERROR("fclose %m");
    }
error1:
    rc = fclose(f_template);
    if (rc < 0) {
        rc = -errno;
        ERROR("fclose %m");
    }
ret:
    return rc;
}

/**
 * @brief Generate the te file
 *
 * @param[in] selinux_te_template_file The selinux te template file
 * @param[in] selinux_te_file The selinux destination te file
 * @param[in] id The id of the application
 * @param[in] selinux_id The selinux id of the application
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() static int generate_app_module_te(const char *selinux_te_template_file, const char *selinux_te_file,
                                              const char *id, const char *selinux_id) __wur {
    int rc = template_to_module(selinux_te_template_file, selinux_te_file, id, selinux_id);

    if (rc < 0) {
        ERROR("template_to_module");
        return rc;
    }

    return 0;
}

/**
 * @brief Generate the if file
 *
 * @param[in] selinux_if_template_file The selinux if template file
 * @param[in] selinux_if_file The selinux destination if file
 * @param[in] id The id of the application
 * @param[in] selinux_id The selinux id of the application
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() static int generate_app_module_if(const char *selinux_if_template_file, const char *selinux_if_file,
                                              const char *id, const char *selinux_id) __wur {
    int rc = template_to_module(selinux_if_template_file, selinux_if_file, id, selinux_id);

    if (rc < 0) {
        ERROR("template_to_module");
        return rc;
    }

    return 0;
}

/**
 * @brief Generate the fc file
 *
 * @param[in] selinux_fc_file The selinux destination if file
 * @param[in] paths paths handler
 * @param[in] selinux_id The selinux id of the application
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() static int generate_app_module_fc(const char *selinux_fc_file, const path_set_t *paths,
                                              const char *selinux_id) __wur {
    int rc = 0;
    char line[MAX_LINE_SIZE_MODULE];

    FILE *f_module_fc = fopen(selinux_fc_file, "w");

    if (f_module_fc == NULL) {
        rc = -errno;
        ERROR("fopen %s %m", selinux_fc_file);
        goto ret;
    }

    path_type_definitions_t path_type_definitions[number_path_type];

    memset(path_type_definitions, 0, number_path_type * sizeof(path_type_definitions_t));
    rc = init_path_type_definitions(path_type_definitions, selinux_id);  // init labels
    if (rc < 0) {
        ERROR("init_path_type_definitions");
        goto error;
    }

    char *label = NULL;
    const char *gen_context = " gen_context(system_u:object_r:";
    const char *s0 = ",s0)\n";

    for (size_t i = 0; i < paths->size; i++) {
        LOG("Add path %s with type %s", paths->paths[i].path, get_path_type_string(paths->paths[i].path_type));
        label = path_type_definitions[paths->paths[i].path_type].label;

        // Size path + size label + size begin gen_context + size end gen_context
        if (strlen(paths->paths[i].path) + strlen(label) + strlen(gen_context) + strlen(s0) >= MAX_LINE_SIZE_MODULE) {
            ERROR("too long");
            goto error2;
        }

        memset(line, 0, MAX_LINE_SIZE_MODULE);
        strcpy(line, paths->paths[i].path);
        strcat(line, gen_context);
        strcat(line, label);
        strcat(line, s0);

        rc = fputs(line, f_module_fc);
        if (rc < 0) {
            rc = -errno;
            ERROR("fputs %m");
            goto error2;
        }
    }

error2:
    free_path_type_definitions(path_type_definitions);
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
__nonnull() static int generate_app_module_files(const selinux_module_t *selinux_module,
                                                 const secure_app_t *secure_app) __wur {
    int rc = generate_app_module_te(selinux_module->selinux_te_template_file, selinux_module->selinux_te_file,
                                    selinux_module->id, selinux_module->selinux_id);
    if (rc < 0) {
        ERROR("generate_app_module_te");
        goto ret;
    }

    rc = generate_app_module_if(selinux_module->selinux_if_template_file, selinux_module->selinux_if_file,
                                selinux_module->id, selinux_module->selinux_id);
    if (rc < 0) {
        ERROR("generate_app_module_if");
        goto remove_te;
    }

    rc = generate_app_module_fc(selinux_module->selinux_fc_file, &(secure_app->path_set), selinux_module->selinux_id);
    if (rc < 0) {
        ERROR("generate_app_module_fc");
        goto remove_if;
    }

    goto ret;

remove_if:
    if (remove_file(selinux_module->selinux_if_file) < 0) {
        ERROR("remove if file");
    }
remove_te:
    if (remove_file(selinux_module->selinux_te_file) < 0) {
        ERROR("remove te file");
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
__nonnull() static bool check_app_module_files_exists(const selinux_module_t *selinux_module) __wur {
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
__nonnull() static int remove_app_module_files(const selinux_module_t *selinux_module) __wur {
    int rc = remove_file(selinux_module->selinux_te_file);
    if (rc < 0) {
        goto error;
    }

    rc = remove_file(selinux_module->selinux_if_file);
    if (rc < 0) {
        goto error;
    }

    rc = remove_file(selinux_module->selinux_fc_file);
    if (rc < 0) {
        goto error;
    }

    goto end;

error:
    ERROR("remove_file");
end:
    return rc;
}

/**
 * @brief Remove pp file
 *
 * @param[in] selinux_module selinux module handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() static int remove_pp_files(const selinux_module_t *selinux_module) __wur {
    int rc = remove_file(selinux_module->selinux_pp_file);
    if (rc < 0) {
        goto error;
    }

    goto end;

error:
    ERROR("remove_file");
    goto end;
end:
    return rc;
}

/**
 * @brief Destroy semanage handle and content
 *
 * @param[in] semanage_handle semanage_handle handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() static int destroy_semanage_handle(semanage_handle_t *semanage_handle) __wur {
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
static int create_semanage_handle(semanage_handle_t **semanage_handle) __wur {
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
__nonnull((1)) static int install_module(semanage_handle_t *semanage_handle, const char *selinux_pp_file) __wur {
    int rc = semanage_module_install_file(semanage_handle, selinux_pp_file);
    if (rc < 0) {
        ERROR("semanage_module_install_file");
        goto end;
    }

    rc = semanage_commit(semanage_handle);
    if (rc < 0) {
        ERROR("semanage_commit");
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
__nonnull() static int remove_module(semanage_handle_t *semanage_handle, const char *module_name) __wur {
    char *module_name_ = strdupa(module_name);  // semanage_module_remove take no const module name
    int rc = 0;
    if (module_name_ == NULL) {
        ERROR("strdupa");
        rc = -ENOMEM;
        goto end;
    }

    rc = semanage_module_remove(semanage_handle, module_name_);
    if (rc < 0) {
        ERROR("semanage_module_remove");
        goto end;
    }

    rc = semanage_commit(semanage_handle);
    if (rc < 0) {
        ERROR("semanage_commit");
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
__nonnull() static bool check_module(semanage_handle_t *semanage_handle, const char *id) __wur {
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
    return value ?: secure_getenv("SELINUX_RULES_DIR") ?: DEFAULT_SELINUX_RULES_DIR;
}

/* see selinux-template.h */
int create_selinux_rules(const secure_app_t *secure_app, const char *selinux_te_template_file,
                         const char *selinux_if_template_file, const char *selinux_rules_dir) {
    selinux_module_t selinux_module;
    int rc = init_selinux_module(&selinux_module, secure_app->id, selinux_te_template_file, selinux_if_template_file,
                                 selinux_rules_dir);
    if (rc < 0) {
        ERROR("init_selinux_module");
        goto ret;
    }

    semanage_handle_t *semanage_handle;
    rc = create_semanage_handle(&semanage_handle);
    if (rc < 0) {
        ERROR("create_semanage_handle");
        goto end;
    }

    // Generate files
    rc = generate_app_module_files(&selinux_module, secure_app);
    if (rc < 0) {
        ERROR("generate_app_module_files");
        goto end2;
    }

    // fc, if, te generated
    rc = launch_compile();
    if (rc < 0) {
        ERROR("launch_compile");
        goto error3;
    }

    // pp generated

    rc = install_module(semanage_handle, selinux_module.selinux_pp_file);

    if (rc < 0) {
        ERROR("install_module");
        goto error4;
    }

    goto end2;

error4:
    remove_pp_files(&selinux_module);
error3:
    remove_app_module_files(&selinux_module);
end2:
    rc = destroy_semanage_handle(semanage_handle);
    if (rc < 0) {
        ERROR("destroy_semanage_handle");
    }
end:
    free_selinux_module(&selinux_module);
ret:
    return rc;
}

/* see selinux-template.h */
bool check_module_files_exist(const secure_app_t *secure_app, const char *selinux_rules_dir) {
    selinux_module_t selinux_module;
    bool ret = false;
    int rc = init_selinux_module(&selinux_module, secure_app->id, NULL, NULL, selinux_rules_dir);
    if (rc < 0) {
        ERROR("init_selinux_module");
        goto end;
    }

    ret = check_app_module_files_exists(&selinux_module);

    free_selinux_module(&selinux_module);

end:
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
int remove_selinux_rules(const secure_app_t *secure_app, const char *selinux_rules_dir) {
    // remove files
    selinux_module_t selinux_module;
    int rc = init_selinux_module(&selinux_module, secure_app->id, NULL, NULL, selinux_rules_dir);
    if (rc < 0) {
        ERROR("init_selinux_module");
        goto ret;
    }

    rc = remove_app_module_files(&selinux_module) + remove_pp_files(&selinux_module);
    if (rc < 0) {
        ERROR("remove files modules");
        goto end;
    }

    // remove module in policy
    semanage_handle_t *semanage_handle;
    rc = create_semanage_handle(&semanage_handle);
    if (rc < 0) {
        ERROR("create_semanage_handle");
        goto end;
    }
    rc = remove_module(semanage_handle, secure_app->id);
    if (rc < 0) {
        ERROR("remove_module");
        goto end2;
    }

    goto end2;

end2:
    rc = destroy_semanage_handle(semanage_handle);
    if (rc < 0) {
        ERROR("destroy_semanage_handle");
    }
end:
    free_selinux_module(&selinux_module);
ret:
    return rc;
}
