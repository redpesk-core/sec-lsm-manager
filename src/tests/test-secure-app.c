/*
 * Copyright (C) 2020-2022 IoT.bzh Company
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

#include "../secure-app.c"
#include "setup-tests.h"

START_TEST(test_init_secure_app) {
    secure_app_t secure_app;
    init_secure_app(&secure_app);
    ck_assert_str_eq(secure_app.id, "");
    ck_assert_str_eq(secure_app.id_underscore, "");
    ck_assert_ptr_eq(secure_app.permission_set.permissions, NULL);
    ck_assert_int_eq((int)secure_app.permission_set.size, 0);
    ck_assert_ptr_eq(secure_app.path_set.paths, NULL);
    ck_assert_int_eq((int)secure_app.path_set.size, 0);
}
END_TEST

START_TEST(test_create_secure_app) {
    secure_app_t *secure_app = NULL;
    ck_assert_int_eq(create_secure_app(&secure_app), 0);
    ck_assert_ptr_ne(secure_app, NULL);
    ck_assert_str_eq(secure_app->id, "");
    ck_assert_str_eq(secure_app->id_underscore, "");
    ck_assert_ptr_eq(secure_app->permission_set.permissions, NULL);
    ck_assert_int_eq((int)secure_app->permission_set.size, 0);
    ck_assert_ptr_eq(secure_app->path_set.paths, NULL);
    ck_assert_int_eq((int)secure_app->path_set.size, 0);
    destroy_secure_app(secure_app);
}
END_TEST

START_TEST(test_secure_app_set_id) {
    secure_app_t *secure_app = NULL;
    ck_assert_int_eq(create_secure_app(&secure_app), 0);
    // test set id
    ck_assert_int_eq(secure_app_set_id(secure_app, "id"), 0);
    ck_assert_str_eq(secure_app->id, "id");
    // test duplicate set id
    ck_assert_int_eq(secure_app_set_id(secure_app, "id2"), -EINVAL);
    destroy_secure_app(secure_app);

    ck_assert_int_eq(create_secure_app(&secure_app), 0);
    ck_assert_int_lt(secure_app_set_id(secure_app, "i"), 0);
    destroy_secure_app(secure_app);

    ck_assert_int_eq(create_secure_app(&secure_app), 0);
    ck_assert_int_lt(secure_app_set_id(secure_app, ""), 0);
    destroy_secure_app(secure_app);

    ck_assert_int_eq(create_secure_app(&secure_app), 0);
    ck_assert_int_lt(secure_app_set_id(secure_app, "id?bad/name"), 0);
    destroy_secure_app(secure_app);

    ck_assert_int_eq(create_secure_app(&secure_app), 0);
    // test error flag raise
    raise_error_flag(secure_app);
    ck_assert_int_eq(secure_app_set_id(secure_app, "id3"), -EPERM);
    destroy_secure_app(secure_app);
}
END_TEST

START_TEST(test_secure_app_add_permission) {
    secure_app_t *secure_app = NULL;
    ck_assert_int_eq(create_secure_app(&secure_app), 0);
    // test add perm
    ck_assert_int_eq(secure_app_add_permission(secure_app, "perm"), 0);
    ck_assert_int_eq((int)secure_app->permission_set.size, 1);
    ck_assert_str_eq(secure_app->permission_set.permissions[0], "perm");

    // test duplicate perm
    ck_assert_int_eq(secure_app_add_permission(secure_app, "perm"), -EINVAL);

    // test error flag raise
    raise_error_flag(secure_app);
    ck_assert_int_eq(secure_app_add_permission(secure_app, "perm2"), -EPERM);
    destroy_secure_app(secure_app);
}
END_TEST

START_TEST(test_secure_app_add_path) {
    secure_app_t *secure_app = NULL;
    ck_assert_int_eq(create_secure_app(&secure_app), 0);
    // test add path
    ck_assert_int_eq(secure_app_add_path(secure_app, "/tmp", type_conf), 0);
    ck_assert_int_eq((int)secure_app->path_set.size, 1);
    ck_assert_str_eq(secure_app->path_set.paths[0]->path, "/tmp");
    ck_assert_int_eq((int)secure_app->path_set.paths[0]->path_type, (int)type_conf);

    // test bad path type
    ck_assert_int_eq(secure_app_add_path(secure_app, "/tmp3", type_none), -EINVAL);

    // test duplicate path
    ck_assert_int_eq(secure_app_add_path(secure_app, "/tmp", type_data), -EINVAL);
    ck_assert_int_eq(secure_app_add_path(secure_app, "/tmp", type_conf), -EINVAL);

    // test error flag raise
    raise_error_flag(secure_app);
    ck_assert_int_eq(secure_app_add_path(secure_app, "/tmp2", type_conf), -EPERM);
    destroy_secure_app(secure_app);
}
END_TEST

START_TEST(test_free_secure_app) {
    secure_app_t *secure_app = NULL;
    ck_assert_int_eq(create_secure_app(&secure_app), 0);
    ck_assert_int_eq(secure_app_set_id(secure_app, "id"), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, "/tmp", type_conf), 0);
    ck_assert_int_eq(secure_app_add_permission(secure_app, "perm"), 0);
    clear_secure_app(secure_app);
    ck_assert_int_eq((int)secure_app->path_set.size, 0);
    ck_assert_int_eq((int)secure_app->permission_set.size, 0);
    destroy_secure_app(secure_app);
}
END_TEST

START_TEST(test_destroy_secure_app) {
    secure_app_t *secure_app = NULL;
    ck_assert_int_eq(create_secure_app(&secure_app), 0);
    ck_assert_int_eq(secure_app_set_id(secure_app, "id"), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, "/tmp", type_conf), 0);
    ck_assert_int_eq(secure_app_add_permission(secure_app, "perm"), 0);
    destroy_secure_app(secure_app);
}
END_TEST

void test_secure_app() {
    addtest(test_init_secure_app);
    addtest(test_create_secure_app);
    addtest(test_secure_app_set_id);
    addtest(test_secure_app_add_permission);
    addtest(test_secure_app_add_path);
    addtest(test_free_secure_app);
    addtest(test_destroy_secure_app);
}
