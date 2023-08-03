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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "secure-app.h"
#include "log.h"
#include "utils.h"

#ifdef WITH_SMACK
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
__nonnull() static void init_secure_app(secure_app_t *secure_app) {
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
 * @return 0 in case of success or a negative value when an error occurs
 */
__nonnull()
static int setids(
    const char *src,
    char id[SEC_LSM_MANAGER_MAX_SIZE_ID + 1],
    char id_underscore[SEC_LSM_MANAGER_MAX_SIZE_ID + 1],
    char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1]
) {
    char car;
    int idx;

    /* copy the id */
    for (idx = 0 ; (car = src[idx]) != '\0' ; idx++) {
        /* validate length isn't too big */
        if (idx >= SEC_LSM_MANAGER_MAX_SIZE_ID) {
            ERROR("invalid id size, bigger than %d for %s", SEC_LSM_MANAGER_MAX_SIZE_ID, src);
            return -ENAMETOOLONG;
        }
        /* validate character is valid */
        if (!isalnum(car) && car != '-' && car != '_') {
            ERROR("invalid id, only alphanumeric, '-', '_', but %s", src);
            return -EINVAL;
        }
        /* set the copied character */
        id[idx] = car;
        id_underscore[idx] = car == '-' ? '_' : car;
    }
    /* validate length isn't too small */
    if (idx < SEC_LSM_MANAGER_MIN_SIZE_ID) {
        ERROR("invalid id size, at least %d characters are needed, but %s", SEC_LSM_MANAGER_MIN_SIZE_ID, src);
        return -EINVAL;
    }
    /* end mark */
    id[idx] = id_underscore[idx] = 0;
    app_label_mac(label, id, id_underscore);
    return 0;
}


/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see secure-app.h */
int create_secure_app(secure_app_t **secure_app) {
    *secure_app = (secure_app_t *)malloc(sizeof(secure_app_t));
    if (*secure_app == NULL) {
        ERROR("malloc failed");
        return -ENOMEM;
    }

    init_secure_app(*secure_app);
    return 0;
}

/* see secure-app.h */
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

/* see secure-app.h */
void destroy_secure_app(secure_app_t *secure_app) {
    clear_secure_app(secure_app);
    free(secure_app);
}

/* see secure-app.h */
int secure_app_set_id(secure_app_t *secure_app, const char *id) {
    int rc;

    if (secure_app->error_flag) {
        ERROR("error flag has been raised");
        rc = -ENOTRECOVERABLE;
    }

    else if (secure_app->id[0] != '\0') {
        ERROR("id already set");
        rc = -EBUSY;
    }

    else {
        rc = setids(id, secure_app->id, secure_app->id_underscore, secure_app->label);
        if (rc < 0)
            secure_app->id[0] = secure_app->id_underscore[0] = secure_app->label[0] = '\0';
    }

    return rc;
}

/* see secure-app.h */
int secure_app_add_permission(secure_app_t *secure_app, const char *permission) {
    if (secure_app->error_flag) {
        ERROR("error flag has been raised");
        return -EPERM;
    }

    for (size_t i = 0; i < secure_app->permission_set.size; i++) {
        if (!strcmp(secure_app->permission_set.permissions[i], permission)) {
            ERROR("permission already defined");
            return -EINVAL;
        }
    }

    int rc = permission_set_add_permission(&(secure_app->permission_set), permission);
    if (rc < 0) {
        ERROR("permission_set_add_permission : %d %s", -rc, strerror(-rc));
        return rc;
    }
    secure_app->need_id = true;

    return 0;
}

/* see secure-app.h */
__wur __nonnull()
int secure_app_add_path(secure_app_t *secure_app, const char *path, enum path_type path_type) {
    if (!valid_path_type(path_type)) {
        ERROR("path_type invalid : %d", path_type);
        return -EINVAL;
    }

    if (secure_app->error_flag) {
        ERROR("error flag has been raised");
        return -EPERM;
    }

    for (size_t i = 0; i < secure_app->path_set.size; i++) {
        if (!strcmp(secure_app->path_set.paths[i]->path, path)) {
            ERROR("path already defined");
            return -EINVAL;
        }
    }

    int rc = path_set_add_path(&(secure_app->path_set), path, path_type);
    if (rc < 0) {
        ERROR("path_set_add_path %d %s", -rc, strerror(-rc));
        return rc;
    }
    if (path_type != type_default)
        secure_app->need_id = true;

    return 0;
}

/* see secure-app.h */
__wur __nonnull()
int secure_app_add_plug(secure_app_t *secure_app, const char *expdir, const char *impid, const char *impdir)
{
    int rc;

    if (secure_app->error_flag) {
        ERROR("error flag has been raised");
        return -EPERM;
    }

    rc = plugset_add(&(secure_app->plugset), expdir, impid, impdir);
    if (rc < 0) {
        ERROR("can't add plug %d %s", -rc, strerror(-rc));
        return rc;
    }
    secure_app->need_id = true;
    return 0;
}

/* see secure-app.h */
void raise_error_flag(secure_app_t *secure_app) {
    secure_app->error_flag = true;
}

/**
 * @brief Update the policy (drop the old and set the new)
 *
 * @param[in] sm_handle sec_lsm_manager_handle handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur static
int update_policy(secure_app_t *secure_app, cynagora_t *cynagora) {
    // drop old policies if any
    int rc = cynagora_drop_policies(cynagora, secure_app->label);
    if (rc < 0) {
        ERROR("cynagora_drop_policies %s : %d %s", secure_app->label, -rc, strerror(-rc));
        return rc;
    }

    // apply new policies
    rc = cynagora_set_policies(cynagora, secure_app->label, &(secure_app->permission_set));
    if (rc < 0) {
        ERROR("cynagora_set_policies %s : %d %s", secure_app->label, -rc, strerror(-rc));
        return rc;
    }

    return 0;
}

/* see secure-app.h */
__nonnull() __wur
int secure_app_install(secure_app_t *secure_app, cynagora_t *cynagora)
{
    int rc;
    if (secure_app->error_flag) {
        ERROR("error flag has been raised, clear secure app");
        return -EINVAL;
    }

    bool has_id = secure_app->id[0] != '\0';
    if (!has_id && secure_app->need_id) {
        ERROR("an application identifier is needed");
        return -EINVAL;
    }

    rc = secure_app_check(secure_app, cynagora);
    if (rc < 0)
        return rc;

    if (has_id) {
        rc = update_policy(secure_app, cynagora);
        if (rc < 0) {
            ERROR("update_policy : %d %s", -rc, strerror(-rc));
            return rc;
        }
        DEBUG("update_policy success");
    }

    rc = install_mac(secure_app);
    if (rc < 0) {
        ERROR("install_mac : %d %s", -rc, strerror(-rc));
        if (has_id) {
            int rc2 = cynagora_drop_policies(cynagora, secure_app->label);
            if (rc2 < 0) {
                ERROR("cannot delete policy : %d %s", -rc2, strerror(-rc2));
            }
        }
        return rc;
    }

    DEBUG("install success");

    return 0;
}

/* see secure-app.h */
__nonnull() __wur
int secure_app_uninstall(secure_app_t *secure_app, cynagora_t *cynagora)
{
    if (secure_app->error_flag) {
        ERROR("error flag has been raised, clear secure app");
        return -EINVAL;
    }

    bool has_id = secure_app->id[0] != '\0';
    if (!has_id && secure_app->need_id) {
        ERROR("an application identifier is needed");
        return -EINVAL;
    }

    if (has_id) {
        int rc = cynagora_drop_policies(cynagora, secure_app->label);
        if (rc < 0) {
            ERROR("cynagora_drop_policies : %d %s", -rc, strerror(-rc));
            return rc;
        }
    }

    int rc = uninstall_mac(secure_app);
    if (rc < 0) {
        ERROR("uninstall_mac : %d %s", -rc, strerror(-rc));
        return rc;
    }

    DEBUG("uninstall success");

    return 0;
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
    static const char perm_export_template[] = "urn:redpesk:permission::%s:export:plug:%s";
    static const char scope_public[] = "public";
    static const char scope_partner[] = "partner";

    char permission[SEC_LSM_MANAGER_MAX_SIZE_ID + sizeof perm_export_template + sizeof scope_partner];
    char id[SEC_LSM_MANAGER_MAX_SIZE_ID + 1];
    char _id_[SEC_LSM_MANAGER_MAX_SIZE_ID + 1];
    char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1];
    cynagora_key_t cynkey;
    plug_t *plugit;
    const char *scope;
    char **parray;
    size_t idxp, nrp;
    int sts;
    int rc = 0;

    /* iterate over the plug requests */
    for(plugit = secure_app->plugset ; plugit != NULL ; plugit = plugit->next) {

        /* compute the label of the application importing the plug */
        sts = setids(plugit->impid, id, _id_, label);
        if (sts == 0) {
            /* check if importing application (its label) has urn:redpesk:permission::public:plug */
            cynkey.client = label;
            cynkey.session = "*";
            cynkey.user = "*";
            cynkey.permission = perm_public_plug;
            sts = cynagora_check(cynagora, &cynkey, 0);
            if (sts < 0) {
                ERROR("can't query cynagora");
            }
            else {
                /* compute the scope of the required permision */
                scope = sts ? scope_public : scope_partner;
                /* compute the required permision */
                snprintf(permission, sizeof permission, perm_export_template, scope, id);
                /* check if the permision is granted for the app */
                parray = secure_app->permission_set.permissions;
                nrp = secure_app->permission_set.size;
                idxp = 0;
                while (idxp < nrp && strcmp(permission, parray[idxp]) != 0)
                    idxp++;
                if (idxp < nrp)
                    sts = 0;
                else {
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

/* see secure-app.h */
__nonnull() __wur
int secure_app_check(secure_app_t *secure_app, cynagora_t *cynagora)
{
    return check_plugs(secure_app, cynagora);
}
