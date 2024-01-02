/*
 * Copyright (C) 2020-2024 IoT.bzh Company
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

#include "smack.h"

#include <errno.h>
#include <linux/xattr.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

#include "log.h"
#include "smack-template.h"
#include "file-utils.h"
#include "xattr-smack.h"

#define DROP_LABEL "User:Home"

#if WITH_SMACK
#include "action/mac-interface.h"
__wur __nonnull()
int mac_install(const context_t *context)
         __attribute__ ((alias ("smack_install")));

__wur __nonnull()
int mac_uninstall(const context_t *context)
         __attribute__ ((alias ("smack_uninstall")));

__nonnull()
void mac_get_label(char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1], const char *appid)
         __attribute__ ((alias ("smack_get_label")));
#endif

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Remove labels of a path entry
 *
 * @param[in] path The path of the file
 * @return 0 in case of success or a negative -errno value
 */
__nonnull()
static int unset_path_labels(const char *path) {
    int rc, rc2, pp = get_path_property(path);

    DEBUG("unset_path_labels %s pp=%d", path, pp);

    if (pp < 0)
        return pp;

    // file
    rc = unset_xattr(path, XATTR_NAME_SMACK);
    if (rc < 0)
        ERROR("unlabel(%s): %d %s", path, -rc, strerror(-rc));

    // exec
    rc2 = unset_xattr(path, XATTR_NAME_SMACKEXEC);
    if (rc2 < 0) {
        ERROR("unlabel exec(%s): %d %s", path, -rc2, strerror(-rc2));
        rc = rc < 0 ? rc : rc2;
    }

    // dir
    rc2 = unset_xattr(path, XATTR_NAME_SMACKTRANSMUTE);
    if (rc2 < 0) {
        ERROR("unlabel transmute(%s): %d %s", path, -rc2, strerror(-rc2));
        rc = rc < 0 || pp != PATH_DIRECTORY ? rc : rc2;
    }

    return rc;
}

/**
 * @brief Wrap writing default value of xattr as unsetting it
 * @param[in] path the path of the file
 * @param[in] xattr name of the extended attribute
 * @param[in] value value of the extended attribute
 * @return 0 in case of success or a negative -errno value
 */
static int put_xattr(const char *path, const char *xattr, const char *value)
{
    return value && *value ? set_xattr(path, xattr, value) : unset_xattr(path, xattr);
}

/**
 * @brief Sets labels of a path entry
 *
 * @param[in] path The path of the file
 * @param[in] label The label of the file
 * @param[in] execlabel The execution label or NULL
 * @param[in] transmute True if transmuting
 * @return 0 in case of success or a negative -errno value
 */
__nonnull((1, 2)) __wur
int smack_set_path_labels(const char *path, const char *label, const char *execlabel, bool transmute)
{
    int rc, rc2;

    // access
    rc = put_xattr(path, XATTR_NAME_SMACK, label);
    if (rc < 0)
        ERROR("put_xattr(%s,%s,%s) : %d %s", path, XATTR_NAME_SMACK, label, -rc, strerror(-rc));

    // exec
    rc2 = put_xattr(path, XATTR_NAME_SMACKEXEC, execlabel);
    if (rc < 0) {
        ERROR("put_smack(%s,%s,%s) : %d %s", path, XATTR_NAME_SMACKEXEC,
                   execlabel ? execlabel : "<none>", -rc2, strerror(-rc2));
        rc = rc < 0 ? rc : rc2;
    }

    // transmute
    rc2 = put_xattr(path, XATTR_NAME_SMACKTRANSMUTE, transmute ? "TRUE" : NULL);
    if (rc2 < 0) {
        ERROR("set_xattr(%s,%s,%s) : %d %s", path, XATTR_NAME_SMACKTRANSMUTE,
                   transmute ? "TRUE" : "<none>", -rc2, strerror(-rc2));
        rc = rc < 0 || !transmute ? rc : rc2;
    }

    return rc;
}

/**
 * @brief Set smack labels for context
 *
 * @param[in] context context handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur
static int label_all_paths(const context_t *context, bool set)
{
    size_t i;
    int pp, rc = 0;
    path_t *path;
    path_type_definitions_t path_type_definitions[number_path_type];
    path_type_definitions_t *def;
    const char *exec_label;

    init_path_type_definitions(path_type_definitions, context->id);
    exec_label = path_type_definitions[type_id].label;
    for (i = 0; i < context->path_set.size; i++) {

        path = context->path_set.paths[i];
        def = &path_type_definitions[path->path_type];

        pp = get_path_property(path->path);
        DEBUG("labbelling %s pp=%d", path->path, pp);
        if (pp < 0)
            rc = pp;
        else if (!set)
            rc = unset_path_labels(path->path);
        else
            rc = smack_set_path_labels(path->path,
                                 def->label,
                                 (pp == PATH_FILE_EXEC) && def->is_executable ? exec_label : NULL,
                                 (pp == PATH_DIRECTORY) && def->is_transmute);
        if (rc < 0) {
            ERROR("%sset_path_labels(%s,%s,%s): %d %s", set ? "" : "un", path->path,
                                    def->label, context->id, -rc, strerror(-rc));
            return rc;
        }
    }

    return 0;
}

/**
 * @brief Removes smack labels for context
 *
 * @param[in] context context handler
 * @return 0 in case of success or a negative -errno value
 */
__nonnull() __wur
static int smack_drop_path_labels(const context_t *context) {
    int rc = 0;
    path_t *path = NULL;
    for (size_t i = 0; i < context->path_set.size; i++) {
        path = context->path_set.paths[i];
        rc = smack_set_path_labels(path->path, DROP_LABEL, NULL, false);
        if (rc < 0) {
            ERROR("smack_set_path_labels((%s,%s),%s) : %d %s", context->path_set.paths[i]->path,
                  path_type_name(context->path_set.paths[i]->path_type), context->id, -rc, strerror(-rc));
            return rc;
        }
    }

    return 0;
}

/**
* @brief Installs the files for plugs
*
* @param[in] context the application specification
 * @return 0 in case of success or a negative -errno value
*/
__nonnull() __wur
static int install_smack_plugs(const context_t *context)
{
    char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1];
    char buffer[PATH_MAX + 1];
    plug_t *plugit;
    int rc2, rc = 0;

    for (plugit = context->plugset ; plugit != NULL ; plugit = plugit->next) {
        rc2 = snprintf(buffer, sizeof buffer, "%s/%s", plugit->impdir, context->id);
        if (rc2 > PATH_MAX)
            rc2 = -ENAMETOOLONG;
        if (rc2 < 0)
            ERROR("install_smack_plugs: can't set name: %d, %s", -rc2, strerror(-rc2));
        else {
            rc2 = symlink(plugit->expdir, buffer);
            if (rc2 < 0) {
                rc2 = -errno;
                ERROR("install_smack_plugs: can't create link: %d, %s", -rc2, strerror(-rc2));
            }
            else {
                smack_get_label(label, context->id);
                rc2 = set_xattr(buffer, XATTR_NAME_SMACK, label);
#if COVERAGE
                rc2 = 0;
#endif
            }
        }
        if (rc2 < 0 && rc == 0)
            rc = rc2;
    }
    return rc;
}

/**
* @brief Uninstalls the files for plugs
*
* @param[in] context the application specification
 * @return 0 in case of success or a negative -errno value
*/
__nonnull() __wur
static int uninstall_smack_plugs(const context_t *context)
{
    char buffer[PATH_MAX + 1];
    plug_t *plugit;
    int rc2, rc = 0;

    for (plugit = context->plugset ; plugit != NULL ; plugit = plugit->next) {
        rc2 = snprintf(buffer, sizeof buffer, "%s/%s", plugit->impdir, context->id);
        if (rc2 > PATH_MAX)
            rc2 = -ENAMETOOLONG;
        if (rc2 < 0)
            ERROR("uninstall_smack_plugs: can't set name: %d, %s", -rc2, strerror(-rc2));
        else {
            rc2 = unlink(buffer);
            if (rc2 < 0) {
                if (errno == ENOENT)
                    rc2 = 0;
                else {
                    rc2 = -errno;
                    ERROR("uninstall_smack_plugs: can't unlink: %d, %s", -rc2, strerror(-rc2));
                }
            }
        }
        if (rc2 < 0 && rc == 0)
            rc = rc2;
    }
    return rc;
}

/**
 * @brief Uninstall a context for smack without stopping on failure
 *
 * @param[in] context context handler
 * @return 0 in case of success or a negative -errno value
 */
static int force_uninstall_smack(const context_t *context)
{
    int rc2, rc;

    rc = smack_drop_path_labels(context);
    if (rc < 0)
        ERROR("smack_drop_path_labels: %d %s", -rc, strerror(-rc));

    if (context->id[0] != '\0') {
        rc2 = uninstall_smack_plugs(context);
        if (rc2 < 0) {
            rc = rc2;
            ERROR("uninstall_smack_plugs: %d %s", -rc, strerror(-rc));
        }
        rc2 = remove_smack_rules(context);
        if (rc2 < 0) {
            rc = rc2;
            ERROR("remove_smack_rules: %d %s", -rc, strerror(-rc));
        }
    }

    return rc;
}

/**
 * @brief check the install status and cleanup if its bad
 *
 * @param[in] context the context handler
 * @param[in] rc          the status of the installation
 * @return rc
 */
__nonnull()
static int install_status(const context_t *context, int rc)
{
    if (rc >= 0)
        DEBUG("smack install success");
    else {
        ERROR("smack install failed: %d %s", -rc, strerror(-rc));
        force_uninstall_smack(context);
    }
    return rc;
}

/**
 * @brief install procedure when id is given
 *
 * @param[in] context the context handler
 * @return the installation status
 */
__nonnull()
static int install_smack_with_id(const context_t *context) {
    int rc;

    rc = create_smack_rules(context);
    if (rc >= 0)
        rc = install_smack_plugs(context);
    if (rc >= 0)
        rc = label_all_paths(context, true);

    return install_status(context, rc);
}

/**
 * @brief install procedure when no id is given
 *
 * @param[in] context the context handler
 * @return the installation status
 */
__nonnull()
static int install_smack_no_id(const context_t *context) {
    int rc;

    if (context->need_id) {
        ERROR("id is needed");
        rc = -EINVAL;
    }
    else {
        rc = label_all_paths(context, false);
    }
    return install_status(context, rc);
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see smack.h */
int smack_install(const context_t *context) {
    if (context->id[0] != '\0')
        return install_smack_with_id(context);
    else
        return install_smack_no_id(context);
}

/* see smack.h */
int smack_uninstall(const context_t *context) {
    if (context->id[0] == '\0' && context->need_id) {
        ERROR("id undefined");
        return -EINVAL;
    }

    return force_uninstall_smack(context);
}

/* see smack.h */
__nonnull()
void smack_get_label(char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL + 1], const char *appid)
{
    snprintf(label, SEC_LSM_MANAGER_MAX_SIZE_LABEL, "App:%s", appid);
    label[SEC_LSM_MANAGER_MAX_SIZE_ID] = '\0';
}
