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
 * @param[in] secure_app handler
 */
__nonnull() void init_secure_app(secure_app_t *secure_app) {
    memset(secure_app->id, '\0', SEC_LSM_MANAGER_MAX_SIZE_ID);
    memset(secure_app->id_underscore, '\0', SEC_LSM_MANAGER_MAX_SIZE_ID);
    memset(secure_app->label, '\0', SEC_LSM_MANAGER_MAX_SIZE_LABEL);
    init_path_set(&(secure_app->path_set));
    plugset_init(&(secure_app->plugset));
    init_permission_set(&(secure_app->permission_set));
    secure_app->need_id = false;
    secure_app->error_flag = false;
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
    int rc = secure_app_is_valid_id(src);
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
int create_secure_app(secure_app_t **secure_app) {
    *secure_app = (secure_app_t *)malloc(sizeof(secure_app_t));
    if (*secure_app == NULL) {
        ERROR("malloc failed");
        return -ENOMEM;
    }

    init_secure_app(*secure_app);
    return 0;
}

/* see context.h */
void destroy_secure_app(secure_app_t *secure_app) {
    clear_secure_app(secure_app);
    free(secure_app);
}

/* see context.h */
void clear_secure_app(secure_app_t *secure_app) {
    if (secure_app) {
        secure_app->label[0] = secure_app->id_underscore[0] = secure_app->id[0] = '\0';
        free_permission_set(&(secure_app->permission_set));
        plugset_deinit(&(secure_app->plugset));
        free_path_set(&(secure_app->path_set));
        secure_app->need_id = false;
        secure_app->error_flag = false;
    }
}

/* see context.h */
void secure_app_raise_error(secure_app_t *secure_app) {
    secure_app->error_flag = true;
}

/* see context.h */
bool secure_app_has_error(secure_app_t *secure_app) {
    return secure_app->error_flag;
}

/* see context.h */
int secure_app_is_valid_id(const char *id)
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
int secure_app_set_id(secure_app_t *secure_app, const char *id) {
    int rc;

    /* check error state */
    if (secure_app->error_flag) {
        ERROR("error flag has been raised");
        return -ENOTRECOVERABLE;
    }

    /* check duplication */
    if (secure_app->id[0] != '\0') {
        ERROR("id already set");
        return -EEXIST;
    }

    rc = setids(id, secure_app->id, secure_app->id_underscore, secure_app->label);
    if (rc < 0)
        secure_app->id[0] = secure_app->id_underscore[0] = secure_app->label[0] = '\0';

    return rc;
}

/* see context.h */
int secure_app_add_permission(secure_app_t *secure_app, const char *permission)
{
    size_t i;
    int rc;

    /* check error state */
    if (secure_app->error_flag) {
        ERROR("error flag has been raised");
        return -ENOTRECOVERABLE;
    }

    /* check duplication */
    for (i = 0; i < secure_app->permission_set.size; i++) {
        if (!strcmp(secure_app->permission_set.permissions[i], permission)) {
            ERROR("permission already defined");
            return -EEXIST;
        }
    }

    rc = permission_set_add_permission(&(secure_app->permission_set), permission);
    if (rc < 0) {
        ERROR("permission_set_add_permission: %d %s", -rc, strerror(-rc));
        return rc;
    }
    secure_app->need_id = true;

    return 0;
}

/* see context.h */
__wur __nonnull()
int secure_app_add_path(secure_app_t *secure_app, const char *path, const char *type)
{
    enum path_type path_type;
    size_t i;
    int rc;

    /* check error state */
    if (secure_app->error_flag) {
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
    for (i = 0; i < secure_app->path_set.size; i++) {
        if (!strcmp(secure_app->path_set.paths[i]->path, path)) {
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
    rc = path_set_add_path(&(secure_app->path_set), path, path_type);
    if (rc < 0) {
        ERROR("can't add path %s: %d %s", path, -rc, strerror(-rc));
        return rc;
    }

    /* compute the new need of id */
    if (path_type != type_default)
        secure_app->need_id = true;

    return 0;
}

/* see context.h */
__wur __nonnull()
int secure_app_add_plug(secure_app_t *secure_app, const char *expdir, const char *impid, const char *impdir)
{
    int rc;
    plug_t *iter;

    /* check error state */
    if (secure_app->error_flag) {
        ERROR("error flag has been raised");
        return -ENOTRECOVERABLE;
    }

    /* check duplication */
    for (iter = secure_app->plugset; iter != NULL ; iter = iter->next) {
        if (!strcmp(iter->impdir, impdir)) {
            ERROR("import directory already added");
            return -EEXIST;
        }
    }

    /* check validity of id */
    rc = secure_app_is_valid_id(impid);
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
    rc = plugset_add(&(secure_app->plugset), expdir, impid, impdir);
    if (rc < 0) {
        ERROR("can't add plug %d %s", -rc, strerror(-rc));
        return rc;
    }
    secure_app->need_id = true;
    return 0;
}

/* see context.h */
__nonnull() __wur
int secure_app_install(secure_app_t *secure_app, cynagora_t *cynagora)
{
    int rc, rc2;
    bool has_id;

    /* check error state */
    if (secure_app->error_flag) {
        ERROR("error flag has been raised, clear secure app");
        return -ENOTRECOVERABLE;
    }

    /* check application id need */
    has_id = secure_app->id[0] != '\0';
    if (!has_id && secure_app->need_id) {
        ERROR("an application identifier is needed");
        return -EINVAL;
    }

    /* check consistency */
    rc = secure_app_check(secure_app, cynagora);
    if (rc < 0)
        return rc;

    /* set cynagora policies */
    if (has_id) {
        rc = cynagora_set_policies(cynagora, secure_app->label, &(secure_app->permission_set), 1);
        if (rc < 0) {
            ERROR("cynagora_set_policies: %d %s", -rc, strerror(-rc));
            return rc;
        }
        DEBUG("cynagora_set_policies success");
    }

    /* set LSM / MAC policies */
    rc = install_mac(secure_app);
    if (rc < 0) {
        ERROR("install_mac: %d %s", -rc, strerror(-rc));
        if (has_id) {
            rc2 = cynagora_drop_policies(cynagora, secure_app->label);
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
int secure_app_uninstall(secure_app_t *secure_app, cynagora_t *cynagora)
{
    /* check error state */
    if (secure_app->error_flag) {
        ERROR("error flag has been raised, clear secure app");
        return -ENOTRECOVERABLE;
    }

    /* check application id need */
    bool has_id = secure_app->id[0] != '\0';
    if (!has_id && secure_app->need_id) {
        ERROR("an application identifier is needed");
        return -EINVAL;
    }

    /* drop cynagora policies */
    if (has_id) {
        int rc = cynagora_drop_policies(cynagora, secure_app->label);
        if (rc < 0) {
            ERROR("cynagora_drop_policies: %d %s", -rc, strerror(-rc));
            return rc;
        }
    }

    /* drop LSM / MAC policies */
    int rc = uninstall_mac(secure_app);
    if (rc < 0) {
        ERROR("uninstall_mac: %d %s", -rc, strerror(-rc));
        return rc;
    }

    DEBUG("uninstall success");
    return 0;
}

/* see context.h */
__nonnull() __wur
int secure_app_has_permission(const secure_app_t *secure_app, const char *permission)
{
    return permission_set_has_permission(&secure_app->permission_set, permission);
}

/**
 * @brief Check if application plugs can be installed
 *
 * @param[in] secure_app the application to be checked
 * @param[in] cynagora handler to cynagora access
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur
static int check_plugs(secure_app_t *secure_app, cynagora_t *cynagora)
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
    for(plugit = secure_app->plugset ; plugit != NULL ; plugit = plugit->next) {

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
                sts = secure_app_has_permission(secure_app, permission);
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
int secure_app_check(secure_app_t *secure_app, cynagora_t *cynagora)
{
    return check_plugs(secure_app, cynagora);
}

/* see context.h */
__nonnull((1,2)) __wur
int secure_app_visit(secure_app_t *secure_app, void *visitor, const secure_app_visitor_itf_t *itf)
{
    plug_t *plugit;
    size_t i;
    int rc = 0;

    if (itf->id != NULL && secure_app->id[0] != '\0')
        rc = itf->id(visitor, secure_app->id);

    if (itf->path != NULL)
        for (i = 0; !rc && i < secure_app->path_set.size; i++)
            rc = itf->path(visitor,
                        secure_app->path_set.paths[i]->path,
                        get_path_type_string(secure_app->path_set.paths[i]->path_type));

    if (itf->permission != NULL)
        for (i = 0; !rc && i < secure_app->permission_set.size; i++)
            rc = itf->permission(visitor, secure_app->permission_set.permissions[i]);

    if (itf->plug != NULL)
        for (plugit = secure_app->plugset ; !rc && plugit != NULL ; plugit = plugit->next)
            rc = itf->plug(visitor, plugit->expdir, plugit->impid, plugit->impdir);

    return rc;
}

