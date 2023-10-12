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

#include "action.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "utils.h"

 __wur __nonnull() extern int install_mac(const context_t *context);
 __wur __nonnull() extern int uninstall_mac(const context_t *context);
__nonnull() extern void app_label_mac(char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1], const char *appid);

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Check if application plugs can be installed
 *
 * @param[in] context the application to be checked
 * @param[in] cynagora handler to cynagora access
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur
static int check_plug_installable(context_t *context, cynagora_t *cynagora)
{
    static const char perm_public_plug[] = "urn:redpesk:permission::public:plugs";
    static const char perm_export_template[] = "urn:redpesk:permission:%s:%s:export:plug";
    static const char scope_public[] = "public";
    static const char scope_partner[] = "partner";

    char permission[SEC_LSM_MANAGER_MAX_SIZE_ID + sizeof perm_export_template + sizeof scope_partner];
    char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1];
    plug_t *plugit;
    const char *scope;
    int sts;
    int rc = 0;

    /* iterate over the plug requests */
    for(plugit = context->plugset ; plugit != NULL ; plugit = plugit->next) {

        /* compute the label of the application importing the plug */
        app_label_mac(label, plugit->impid);
        if (sts == 0) {
            sts = cynagora_check_permission(cynagora, label, perm_public_plug);
            if (sts < 0) {
                ERROR("can't query cynagora");
            }
            else {
                /* compute the scope of the required permision */
                scope = sts ? scope_public : scope_partner;
                /* compute the required permision */
                snprintf(permission, sizeof permission, perm_export_template, plugit->impid, scope);
                /* check if the permision is granted for the app */
                sts = context_has_permission(context, permission);
                if (!sts) {
                    ERROR("no permission to install plugs for %s", plugit->impid);
                    sts = -EPERM;
                }
            }
        }
        if (sts < 0 && rc == 0)
            rc = sts;
    }
    return rc;
}

/**
 * @brief Check if context is valid
 *
 * @param[in] context the application to be checked
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur
static int check_context(context_t *context, char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1])
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
    app_label_mac(label, context->id);
    return 0;
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see action.h */
__nonnull() __wur
int action_install(context_t *context, cynagora_t *cynagora)
{
    /* check consistency */
    char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1];
    bool has_id = context->id[0] != '\0';
    int rc2, rc = check_context(context, label);
    if (rc < 0)
        return rc;
    rc = check_plug_installable(context, cynagora);
    if (rc < 0)
        return rc;

    /* set cynagora policies */
    if (has_id) {
        rc = cynagora_set_policies(cynagora, label, &(context->permission_set), 1);
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
            rc2 = cynagora_drop_policies(cynagora, label);
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

/* see action.h */
__nonnull() __wur
int action_uninstall(context_t *context, cynagora_t *cynagora)
{
    /* check consistency */
    char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1];
    bool has_id = context->id[0] != '\0';
    int rc = check_context(context, label);
    if (rc < 0)
        return rc;

    /* drop cynagora policies */
    if (has_id) {
        rc = cynagora_drop_policies(cynagora, label);
        if (rc < 0) {
            ERROR("cynagora_drop_policies: %d %s", -rc, strerror(-rc));
            return rc;
        }
    }

    /* drop LSM / MAC policies */
    rc = uninstall_mac(context);
    if (rc < 0) {
        ERROR("uninstall_mac: %d %s", -rc, strerror(-rc));
        return rc;
    }

    DEBUG("uninstall success");
    return 0;
}

