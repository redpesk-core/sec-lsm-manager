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

#include "test_secure_app.h"

#include <CUnit/Basic.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../secure-app.h"

void test_create_secure_app(void) {
    secure_app_t *secure_app = NULL;
    CU_ASSERT_EQUAL(create_secure_app(&secure_app), 0);
    CU_ASSERT_NOT_EQUAL(secure_app, NULL);
    CU_ASSERT_EQUAL(secure_app->id, NULL);
    CU_ASSERT_EQUAL(secure_app->policies.policies, NULL);
    CU_ASSERT_EQUAL(secure_app->policies.size, 0);
    CU_ASSERT_EQUAL(secure_app->paths.paths, NULL);
    CU_ASSERT_EQUAL(secure_app->paths.size, 0);
    destroy_secure_app(secure_app);
}

void test_secure_app_set_id(void) {
    secure_app_t *secure_app = NULL;
    CU_ASSERT_EQUAL(create_secure_app(&secure_app), 0);
    CU_ASSERT_EQUAL(secure_app_set_id(secure_app, "id"), 0);
    CU_ASSERT_STRING_EQUAL(secure_app->id, "id");
    CU_ASSERT_EQUAL(secure_app_set_id(secure_app, "id2"), 1);
    destroy_secure_app(secure_app);
}

void test_secure_app_add_permission(void) {
    secure_app_t *secure_app = NULL;
    CU_ASSERT_EQUAL(create_secure_app(&secure_app), 0);
    CU_ASSERT_EQUAL(secure_app_add_permission(secure_app, "perm"), -EINVAL);
    CU_ASSERT_EQUAL(secure_app_set_id(secure_app, "id"), 0);
    CU_ASSERT_EQUAL(secure_app_add_permission(secure_app, "perm"), 0);
    CU_ASSERT_EQUAL(secure_app->policies.size, 1);
    CU_ASSERT_STRING_EQUAL(secure_app->policies.policies[0].k.client, "id");
    CU_ASSERT_STRING_EQUAL(secure_app->policies.policies[0].k.permission, "perm");
    CU_ASSERT_STRING_EQUAL(secure_app->policies.policies[0].v.value, AUTHORIZED);
    CU_ASSERT_EQUAL(secure_app->policies.policies->v.expire, 0);
    destroy_secure_app(secure_app);
}

void test_secure_app_add_path(void) {
    secure_app_t *secure_app = NULL;
    CU_ASSERT_EQUAL(create_secure_app(&secure_app), 0);
    CU_ASSERT_EQUAL(secure_app_add_path(secure_app, "/tmp", type_conf), 0);
    CU_ASSERT_EQUAL(secure_app->paths.size, 1);
    CU_ASSERT_STRING_EQUAL(secure_app->paths.paths[0].path, "/tmp");
    CU_ASSERT_EQUAL(secure_app->paths.paths[0].path_type, type_conf);
    destroy_secure_app(secure_app);
}

void test_free_secure_app(void) {
    secure_app_t *secure_app = NULL;
    CU_ASSERT_EQUAL(create_secure_app(&secure_app), 0);
    CU_ASSERT_EQUAL(secure_app_set_id(secure_app, "id"), 0);
    CU_ASSERT_EQUAL(secure_app_add_path(secure_app, "/tmp", type_conf), 0);
    CU_ASSERT_EQUAL(secure_app_add_permission(secure_app, "perm"), 0);
    free_secure_app(secure_app);
    CU_ASSERT_EQUAL(secure_app->paths.size, 0);
    CU_ASSERT_EQUAL(secure_app->policies.size, 0);
    CU_ASSERT_EQUAL(secure_app->id, NULL);
    destroy_secure_app(secure_app);
}

void test_destroy_secure_app(void) {
    secure_app_t *secure_app = NULL;
    CU_ASSERT_EQUAL(create_secure_app(&secure_app), 0);
    CU_ASSERT_EQUAL(secure_app_set_id(secure_app, "id"), 0);
    CU_ASSERT_EQUAL(secure_app_add_path(secure_app, "/tmp", type_conf), 0);
    CU_ASSERT_EQUAL(secure_app_add_permission(secure_app, "perm"), 0);
    destroy_secure_app(secure_app);
}
