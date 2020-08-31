/*
 * Copyright (C) 2020 IoT.bzh Company
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
#include "utils.h"

#define MAX_SMACK_LABEL_SIZE SMACK_LABEL_LEN
#define MAX_ACCESS_SIZE 6
#define MAX_SMACK_RULE_SIZE MAX_SMACK_LABEL_SIZE * 2 + MAX_ACCESS_SIZE + 2

#define SMACK_COMMENT_CHAR '#'

#if !defined(DEFAULT_TEMPLATE_DIR)
#define DEFAULT_TEMPLATE_DIR "/usr/share/security-manager/"
#endif

#if !defined(DEFAULT_TEMPLATE_FILE)
#define DEFAULT_TEMPLATE_FILE "app-template.smack"
#endif

#if !defined(DEFAULT_SMACK_TEMPLATE_FILE)
#define DEFAULT_SMACK_TEMPLATE_FILE DEFAULT_TEMPLATE_DIR DEFAULT_TEMPLATE_FILE
#endif

#if !defined(DEFAULT_SMACK_RULES_DIR)
#define DEFAULT_SMACK_RULES_DIR "/etc/smack/accesses.d/"
#endif

const char default_smack_template_file[] = DEFAULT_SMACK_TEMPLATE_FILE;
const char default_smack_rules_dir[] = DEFAULT_SMACK_RULES_DIR;

const char prefix_app_rules[] = "app-";

#define REPLACE_APP "~APP~"

typedef struct smack_handle {
    const char *id;
    struct smack_accesses *smack_accesses;
} smack_handle_t;

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Free smack handle
 * The pointer is not free
 * @param[in] smack_handle smack_handle handler
 */
__nonnull() static void free_smack_handle(smack_handle_t *smack_handle) {
    CHECK_NO_NULL_NO_RETURN(smack_handle, "smack_handle");

    if (smack_handle->smack_accesses) {
        smack_accesses_free(smack_handle->smack_accesses);
        smack_handle->smack_accesses = NULL;
    }
    free((void *)smack_handle->id);
    smack_handle->id = NULL;
}

/**
 * @brief Init smack handle
 *
 * @param[in] smack_handle smack_handle handler
 * @param[in] id id of application
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() static int init_smack_handle(smack_handle_t *smack_handle, const char *id) __wur {
    CHECK_NO_NULL(smack_handle, "smack_handle");
    CHECK_NO_NULL(id, "id");

    int rc = 0;
    smack_handle->id = strdup(id);

    if (!smack_handle->id) {
        rc = -errno;
        ERROR("strdup id %m");
        return rc;
    }

    rc = smack_accesses_new(&(smack_handle->smack_accesses));
    if (rc < 0) {
        ERROR("smack_accesses_new");
        free_smack_handle(smack_handle);
        return rc;
    }

    return 0;
}

/**
 * @brief Count the number of space in a line
 *
 * @param[in] line The line to parse
 * @return number of space in case of success or a negative -errno value
 */
__nonnull() static int count_space(const char *line) __wur {
    CHECK_NO_NULL(line, "line");

    size_t len = strlen(line);
    int count = 0;
    for (size_t i = 0; i < len; i++) {
        if (line[i] == ' ')
            count++;
    }
    return count;
}

/**
 * @brief Parse a line and add the rule in smack handle
 *
 * @param[in] line The smack rule line to parse
 * @param[in] smack_handle smack_handle handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() static int parse_line(char *line, const smack_handle_t *smack_handle,
                                  path_type_definitions_t path_type_definitions[number_path_type]) __wur {
    CHECK_NO_NULL(line, "line");
    CHECK_NO_NULL(smack_handle, "smack_handle");

    if (line[0] == SMACK_COMMENT_CHAR) {  // comment
        return 0;
    } else if (line[0] == '\n') {  // new line
        return 0;
    }

    char subject[MAX_SMACK_LABEL_SIZE];
    char object[MAX_SMACK_LABEL_SIZE];
    char access[MAX_SMACK_LABEL_SIZE];

    // replace id
    char *pos_str = NULL;
    char after[MAX_SMACK_RULE_SIZE] = {0};

    line[strcspn(line, "\n")] = 0;
    while ((pos_str = strstr(line, REPLACE_APP))) {
        strcpy(after, pos_str + strlen(REPLACE_APP));  // save overwrite data
        strcpy(pos_str, path_type_definitions[type_id].label);
        strcpy(pos_str + strlen(path_type_definitions[type_id].label), after);
    }

    // check valid rule
    int c = count_space(line);
    if (c != 2) {
        printf("Invalid rules");
        return -EINVAL;
    }

    // subject
    char *ptr = strtok(line, " ");
    strncpy(subject, ptr, MAX_SMACK_LABEL_SIZE);

    // object
    ptr = strtok(NULL, " ");
    strncpy(object, ptr, MAX_SMACK_LABEL_SIZE);

    // access
    ptr = strtok(NULL, " ");
    strncpy(access, ptr, MAX_ACCESS_SIZE);

    int rc = smack_accesses_add(smack_handle->smack_accesses, subject, object, access);
    if (rc < 0) {
        ERROR("smack_accesses_add");
        return rc;
    }

    return 0;
}

/**
 * @brief Parse the smack template file and add rules in smack_handle
 *
 * @param[in] smack_template_file The template file
 * @param[in] smack_handle smack_handle handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() static int parse_template_file(const char *smack_template_file, const smack_handle_t *smack_handle,
                                           path_type_definitions_t path_type_definitions[number_path_type]) __wur {
    CHECK_NO_NULL(smack_template_file, "smack_template_file");
    CHECK_NO_NULL(smack_handle, "smack_handle");

    int rc = 0;
    char line[MAX_SMACK_LABEL_SIZE];
    FILE *f = fopen(smack_template_file, "r");

    if (!f) {
        rc = -errno;
        ERROR("fopen : %m");
        return rc;
    }

    while (rc >= 0 && fgets(line, MAX_SMACK_LABEL_SIZE, f)) {
        rc = parse_line(line, smack_handle, path_type_definitions);
    }

    if (rc < 0) {
        ERROR("parse_line : %s", line);
    }

    if (fclose(f)) {
        ERROR("fclose %s : %m", smack_template_file);
    }

    return rc;
}

/**
 * @brief Get smack rules file path specification
 *
 * @param[in] smack_rules_dir some value or NULL for getting default
 * @param[in] id id of the application
 * @return smack rules file path if success, NULL if error
 */
__nonnull() static char *get_smack_rules_file_path(const char *smack_rules_dir, const char *id) __wur {
    CHECK_NO_NULL_RETURN_NULL(smack_rules_dir, "smack_rules_dir");
    CHECK_NO_NULL_RETURN_NULL(id, "id");

    size_t len = strlen(smack_rules_dir) + strlen(prefix_app_rules) + strlen(id);
    char *file = (char *)malloc(len + 1);
    if (!file) {
        ERROR("malloc");
        return NULL;
    }
    memset(file, 0, len + 1);

    strcpy(file, smack_rules_dir);
    strcat(file, prefix_app_rules);
    strcat(file, id);

    return file;
}

/**
 * @brief save the rules in smack rules dir and if smack enable load them directly
 *
 * @param[in] smack_rules_dir The smack rules directory
 * @param[in] smack_handle smack handle handler with some accesses added
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() static int apply_save_accesses_file(const char *smack_rules_dir, smack_handle_t *smack_handle) __wur {
    CHECK_NO_NULL(smack_rules_dir, "smack_rules_dir");
    CHECK_NO_NULL(smack_handle, "smack_handle");

    int rc = 0;

    if (smack_enabled()) {
        rc = smack_accesses_apply(smack_handle->smack_accesses);
        if (rc < 0) {
            ERROR("smack_accesses_apply");
            return rc;
        }
    }

    char *file = get_smack_rules_file_path(smack_rules_dir, smack_handle->id);

    if (!file) {
        ERROR("get_smack_rules_file_path");
        return -EINVAL;
    }

    int fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (fd < 0) {
        rc = -errno;
        ERROR("open : %m");
        goto end;
    }

    rc = smack_accesses_save(smack_handle->smack_accesses, fd);
    if (rc < 0) {
        ERROR("smack_accesses_save");
        goto end2;
    }

    LOG("success store smack rules in %s", file);

end2:
    if (close(fd)) {
        ERROR("close : %m");
    }
end:
    free(file);
    file = NULL;
    return rc;
}

/**
 * @brief Remove file and loaded rules
 *
 * @param[in] file The path of the file to remove
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() static int remove_load_rules(const char *file) __wur {
    CHECK_NO_NULL(file, "file");

    int rc = 0;
    int fd = open(file, O_RDONLY);
    if (fd < 0) {
        rc = -errno;
        ERROR("open file : %s", file);
        return rc;
    }

    struct smack_accesses *smack_access = NULL;
    rc = smack_accesses_new(&smack_access);
    if (rc < 0) {
        ERROR("smack_accesses_new");
        goto end;
    }

    rc = smack_accesses_add_from_file(smack_access, fd);
    if (rc < 0) {
        ERROR("smack_accesses_add_from_file");
        goto end2;
    }

    rc = smack_accesses_clear(smack_access);
    if (rc < 0) {
        ERROR("smack_accesses_clear");
        goto end2;
    }

end2:
    smack_accesses_free(smack_access);
end:
    if (close(fd)) {
        ERROR("close");
    }
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
const char *get_smack_rules_dir(const char *value) {
    return value ?: secure_getenv("SMACK_RULES_DIR") ?: default_smack_rules_dir;
}

/* see smack-template.h */
int create_smack_rules(const secure_app_t *secure_app, path_type_definitions_t path_type_definitions[number_path_type],
                       const char *smack_template_file, const char *smack_rules_dir) {
    CHECK_NO_NULL(secure_app, "secure_app");

    smack_handle_t smack_handle = {0};

    smack_template_file = get_smack_template_file(smack_template_file);
    smack_rules_dir = get_smack_rules_dir(smack_rules_dir);

    int rc = init_smack_handle(&smack_handle, secure_app->id);
    if (rc < 0) {
        ERROR("init_smack_handle");
        return rc;
    }

    rc = parse_template_file(smack_template_file, &smack_handle, path_type_definitions);
    if (rc < 0) {
        ERROR("parse_template_file")
        free_smack_handle(&smack_handle);
        return rc;
    }

    rc = apply_save_accesses_file(smack_rules_dir, &smack_handle);
    if (rc < 0) {
        ERROR("apply_save_accesses_file");
        free_smack_handle(&smack_handle);
        return rc;
    }

    free_smack_handle(&smack_handle);

    LOG("create_smack_rules success");

    return 0;
}

/* see smack-template.h */
int remove_smack_rules(const secure_app_t *secure_app, const char *smack_rules_dir) {
    CHECK_NO_NULL(secure_app, "secure_app");

    int rc = 0;
    smack_rules_dir = get_smack_rules_dir(smack_rules_dir);
    char *file = get_smack_rules_file_path(smack_rules_dir, secure_app->id);

    if (!file) {
        ERROR("get_smack_rules_file_path");
        rc = -EINVAL;
        goto end;
    }

    if (smack_enabled()) {
        rc = remove_load_rules(file);
        if (rc < 0) {
            ERROR("remove_load_rules");
        }
    }

    rc = remove_file(file);
    if (rc < 0) {
        ERROR("remove");
    }

end:
    file = NULL;
    return rc;
}
