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

#include "context.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "utils.h"

#if WITH_SMACK
# include "smack.h"
# define install_mac install_smack
# define uninstall_mac uninstall_smack
# define app_label_mac app_label_smack
#elif WITH_SELINUX
# include "selinux.h"
# define install_mac install_selinux
# define uninstall_mac uninstall_selinux
# define app_label_mac app_label_selinux
#else
# error "unrecognized LSM backend"
#endif

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Initialize the fields 'id', 'id_underscore', 'permission_set', 'path_set' and error_flag
 *
 * @param[in] context handler
 */
__nonnull() void init_context(context_t *context) {
    memset(context->id, '\0', SEC_LSM_MANAGER_MAX_SIZE_ID);
    memset(context->id_underscore, '\0', SEC_LSM_MANAGER_MAX_SIZE_ID);
    memset(context->label, '\0', SEC_LSM_MANAGER_MAX_SIZE_LABEL);
    init_path_set(&(context->path_set));
    plugset_init(&(context->plugset));
    init_permission_set(&(context->permission_set));
    context->need_id = false;
    context->error_flag = false;
}

/**
 * @brief Set and validate the ids from the source
 *
 * @param[in] src the source id
 * @param[out] id output of the id
 * @param[out] id_underscore output of the id with underscores for dashes
 * @param[out] label output of the LSM id for the application
 * @return
 *    * 0 in case of success
 *    * -EINVAL        the id has bad characters
 */
__nonnull()
static int setids(
    const char *src,
    char id[SEC_LSM_MANAGER_MAX_SIZE_ID + 1],
    char id_underscore[SEC_LSM_MANAGER_MAX_SIZE_ID + 1],
    char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1]
) {
    char car;
    int rc = context_is_valid_id(src);
    if (rc >= 0) {
        memcpy(id, src, (unsigned)rc);
        id_underscore[rc] = 0;
        while (rc) {
            car = id[--rc];
            id_underscore[rc] = car == '-' ? '_' : car;
        }
        app_label_mac(label, id, id_underscore);
    }
    return rc;
}


/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see context.h */
int create_context(context_t **context) {
    *context = (context_t *)malloc(sizeof(context_t));
    if (*context == NULL) {
        ERROR("malloc failed");
        return -ENOMEM;
    }

    init_context(*context);
    return 0;
}

/* see context.h */
void destroy_context(context_t *context) {
    clear_context(context);
    free(context);
}

/* see context.h */
void clear_context(context_t *context) {
    if (context) {
        context->label[0] = context->id_underscore[0] = context->id[0] = '\0';
        free_permission_set(&(context->permission_set));
        plugset_deinit(&(context->plugset));
        free_path_set(&(context->path_set));
        context->need_id = false;
        context->error_flag = false;
    }
}

/* see context.h */
void context_raise_error(context_t *context) {
    context->error_flag = true;
}

/* see context.h */
bool context_has_error(context_t *context) {
    return context->error_flag;
}

/* see context.h */
int context_is_valid_id(const char *id)
{
    char car;
    int idx;

    for (idx = 0;; idx++) {

        /* validate length isn't too big */
        if (idx >= SEC_LSM_MANAGER_MAX_SIZE_ID) {
            ERROR("invalid id size, bigger than %d for %.*s...",
                SEC_LSM_MANAGER_MAX_SIZE_ID, SEC_LSM_MANAGER_MAX_SIZE_ID, id);
            return -EINVAL;
        }

        /* get the character */
        car = id[idx];
        if (car == '\0') {
            /* validate length isn't too small */
            if (idx < SEC_LSM_MANAGER_MIN_SIZE_ID) {
                ERROR("invalid id size, at least %d characters are needed, but %s",
                    SEC_LSM_MANAGER_MIN_SIZE_ID, id);
                return -EINVAL;
            }
            /* return the legth */
            return idx;
        }

        /* validate character is valid */
        if (!isalnum(car) && car != '-' && car != '_') {
            ERROR("invalid id, only alphanumeric, '-', '_', but %s", id);
            return -EINVAL;
        }
    }
}

/* see context.h */
int context_set_id(context_t *context, const char *id) {
    int rc;

    /* check error state */
    if (context->error_flag) {
        ERROR("error flag has been raised");
        return -ENOTRECOVERABLE;
    }

    /* check duplication */
    if (context->id[0] != '\0') {
        ERROR("id already set");
        return -EEXIST;
    }

    rc = setids(id, context->id, context->id_underscore, context->label);
    if (rc < 0)
        context->id[0] = context->id_underscore[0] = context->label[0] = '\0';

    return rc;
}

/* see context.h */
int context_add_permission(context_t *context, const char *permission)
{
    size_t i;
    int rc;

    /* check error state */
    if (context->error_flag) {
        ERROR("error flag has been raised");
        return -ENOTRECOVERABLE;
    }

    /* check duplication */
    for (i = 0; i < context->permission_set.size; i++) {
        if (!strcmp(context->permission_set.permissions[i], permission)) {
            ERROR("permission already defined");
            return -EEXIST;
        }
    }

    rc = permission_set_add_permission(&(context->permission_set), permission);
    if (rc < 0) {
        ERROR("permission_set_add_permission: %d %s", -rc, strerror(-rc));
        return rc;
    }
    context->need_id = true;

    return 0;
}

/* see context.h */
__wur __nonnull()
int context_add_path(context_t *context, const char *path, const char *type)
{
    enum path_type path_type;
    size_t i;
    int rc;

    /* check error state */
    if (context->error_flag) {
        ERROR("error flag has been raised");
        return -ENOTRECOVERABLE;
    }

    /* check type validity */
    path_type = get_path_type(type);
    if (!valid_path_type(path_type)) {
        ERROR("type invalid: %d", path_type);
        return -EINVAL;
    }

    /* check duplication */
    for (i = 0; i < context->path_set.size; i++) {
        if (!strcmp(context->path_set.paths[i]->path, path)) {
            ERROR("path already added");
            return -EEXIST;
        }
    }

    /* check existing path */
    rc = check_path_exists(path);
    if (rc < 0) {
        ERROR("path %s isn't accessible: %s", path, strerror(-rc));
        return rc;
    }

    /* add the path to the set */
    rc = path_set_add_path(&(context->path_set), path, path_type);
    if (rc < 0) {
        ERROR("can't add path %s: %d %s", path, -rc, strerror(-rc));
        return rc;
    }

    /* compute the new need of id */
    if (path_type != type_default)
        context->need_id = true;

    return 0;
}

/* see context.h */
__wur __nonnull()
int context_add_plug(context_t *context, const char *expdir, const char *impid, const char *impdir)
{
    int rc;
    plug_t *iter;

    /* check error state */
    if (context->error_flag) {
        ERROR("error flag has been raised");
        return -ENOTRECOVERABLE;
    }

    /* check duplication */
    for (iter = context->plugset; iter != NULL ; iter = iter->next) {
        if (!strcmp(iter->impdir, impdir)) {
            ERROR("import directory already added");
            return -EEXIST;
        }
    }

    /* check validity of id */
    rc = context_is_valid_id(impid);
    if (rc < 0) {
        ERROR("invalid plug id %s", impid);
        return rc;
    }

    /* check directories */
    rc = check_directory_exists(expdir);
    if (rc < 0) {
        ERROR("invalid exported directory %s", expdir);
        return rc;
    }
    rc = check_directory_exists(impdir);
    if (rc < 0) {
        ERROR("invalid import directory %s", impdir);
        return rc;
    }

    /* add the plug property */
    rc = plugset_add(&(context->plugset), expdir, impid, impdir);
    if (rc < 0) {
        ERROR("can't add plug %d %s", -rc, strerror(-rc));
        return rc;
    }
    context->need_id = true;
    return 0;
}

/* see context.h */
__nonnull() __wur
int context_install(context_t *context, cynagora_t *cynagora)
{
    int rc, rc2;
    bool has_id;

    /* check error state */
    if (context->error_flag) {
        ERROR("error flag has been raised, clear context");
        return -ENOTRECOVERABLE;
    }

    /* check application id need */
    has_id = context->id[0] != '\0';
    if (!has_id && context->need_id) {
        ERROR("an application identifier is needed");
        return -EINVAL;
    }

    /* check consistency */
    rc = context_check(context, cynagora);
    if (rc < 0)
        return rc;

    /* set cynagora policies */
    if (has_id) {
        rc = cynagora_set_policies(cynagora, context->label, &(context->permission_set), 1);
        if (rc < 0) {
            ERROR("cynagora_set_policies: %d %s", -rc, strerror(-rc));
            return rc;
        }
        DEBUG("cynagora_set_policies success");
    }

    /* set LSM / MAC policies */
    rc = install_mac(context);
    if (rc < 0) {
        ERROR("install_mac: %d %s", -rc, strerror(-rc));
        if (has_id) {
            rc2 = cynagora_drop_policies(cynagora, context->label);
            if (rc2 < 0) {
                ERROR("cannot delete policy: %d %s", -rc2, strerror(-rc2));
            }
        }
        return rc;
    }

    /* success */
    DEBUG("install success");
    return 0;
}

/* see context.h */
__nonnull() __wur
int context_uninstall(context_t *context, cynagora_t *cynagora)
{
    /* check error state */
    if (context->error_flag) {
        ERROR("error flag has been raised, clear context");
        return -ENOTRECOVERABLE;
    }

    /* check application id need */
    bool has_id = context->id[0] != '\0';
    if (!has_id && context->need_id) {
        ERROR("an application identifier is needed");
        return -EINVAL;
    }

    /* drop cynagora policies */
    if (has_id) {
        int rc = cynagora_drop_policies(cynagora, context->label);
        if (rc < 0) {
            ERROR("cynagora_drop_policies: %d %s", -rc, strerror(-rc));
            return rc;
        }
    }

    /* drop LSM / MAC policies */
    int rc = uninstall_mac(context);
    if (rc < 0) {
        ERROR("uninstall_mac: %d %s", -rc, strerror(-rc));
        return rc;
    }

    DEBUG("uninstall success");
    return 0;
}

/* see context.h */
__nonnull() __wur
int context_has_permission(const context_t *context, const char *permission)
{
    return permission_set_has_permission(&context->permission_set, permission);
}

/**
 * @brief Check if application plugs can be installed
 *
 * @param[in] context the application to be checked
 * @param[in] cynagora handler to cynagora access
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur
static int check_plugs(context_t *context, cynagora_t *cynagora)
{
    static const char perm_public_plug[] = "urn:redpesk:permission::public:plugs";
    static const char perm_export_template[] = "urn:redpesk:permission:%s:%s:export:plug";
    static const char scope_public[] = "public";
    static const char scope_partner[] = "partner";

    char permission[SEC_LSM_MANAGER_MAX_SIZE_ID + sizeof perm_export_template + sizeof scope_partner];
    char id[SEC_LSM_MANAGER_MAX_SIZE_ID + 1];
    char _id_[SEC_LSM_MANAGER_MAX_SIZE_ID + 1];
    char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1];
    plug_t *plugit;
    const char *scope;
    int sts;
    int rc = 0;

    /* iterate over the plug requests */
    for(plugit = context->plugset ; plugit != NULL ; plugit = plugit->next) {

        /* compute the label of the application importing the plug */
        sts = setids(plugit->impid, id, _id_, label);
        if (sts == 0) {
            sts = cynagora_check_permission(cynagora, label, perm_public_plug);
            if (sts < 0) {
                ERROR("can't query cynagora");
            }
            else {
                /* compute the scope of the required permision */
                scope = sts ? scope_public : scope_partner;
                /* compute the required permision */
                snprintf(permission, sizeof permission, perm_export_template, id, scope);
                /* check if the permision is granted for the app */
                sts = context_has_permission(context, permission);
                if (!sts) {
                    ERROR("no permission to install plugs for %s", id);
                    sts = -EPERM;
                }
            }
        }
        if (sts < 0 && rc == 0)
            rc = sts;
    }
    return rc;
}

/* see context.h */
__nonnull() __wur
int context_check(context_t *context, cynagora_t *cynagora)
{
    return check_plugs(context, cynagora);
}

/* see context.h */
__nonnull((1,2)) __wur
int context_visit(context_t *context, void *visitor, const context_visitor_itf_t *itf)
{
    plug_t *plugit;
    size_t i;
    int rc = 0;

    if (itf->id != NULL && context->id[0] != '\0')
        rc = itf->id(visitor, context->id);

    if (itf->path != NULL)
        for (i = 0; !rc && i < context->path_set.size; i++)
            rc = itf->path(visitor,
                        context->path_set.paths[i]->path,
                        get_path_type_string(context->path_set.paths[i]->path_type));

    if (itf->permission != NULL)
        for (i = 0; !rc && i < context->permission_set.size; i++)
            rc = itf->permission(visitor, context->permission_set.permissions[i]);

    if (itf->plug != NULL)
        for (plugit = context->plugset ; !rc && plugit != NULL ; plugit = plugit->next)
            rc = itf->plug(visitor, plugit->expdir, plugit->impid, plugit->impdir);

    return rc;
}

