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

#include "setup-tests.h"

#include <errno.h>

#include "context.h"

START_TEST(test_init_context) {
    context_t context;
    init_context(&context);
    ck_assert_str_eq(context.id, "");
    ck_assert_ptr_eq(context.permission_set.permissions, NULL);
    ck_assert_int_eq((int)context.permission_set.size, 0);
    ck_assert_ptr_eq(context.path_set.paths, NULL);
    ck_assert_int_eq((int)context.path_set.size, 0);
}
END_TEST

START_TEST(test_create_context) {
    context_t *context = NULL;
    ck_assert_int_eq(create_context(&context), 0);
    ck_assert_ptr_ne(context, NULL);
    ck_assert_str_eq(context->id, "");
    ck_assert_ptr_eq(context->permission_set.permissions, NULL);
    ck_assert_int_eq((int)context->permission_set.size, 0);
    ck_assert_ptr_eq(context->path_set.paths, NULL);
    ck_assert_int_eq((int)context->path_set.size, 0);
    destroy_context(context);
}
END_TEST

START_TEST(test_context_set_id) {
    context_t *context = NULL;
    ck_assert_int_eq(create_context(&context), 0);
    // test set id
    ck_assert_int_eq(context_set_id(context, "id"), 0);
    ck_assert_str_eq(context->id, "id");
    // test duplicate set id
    ck_assert_int_eq(context_set_id(context, "id2"), -EEXIST);
    destroy_context(context);

    ck_assert_int_eq(create_context(&context), 0);
    ck_assert_int_eq(context_set_id(context, "i"), -EINVAL);
    destroy_context(context);

    ck_assert_int_eq(create_context(&context), 0);
    ck_assert_int_eq(context_set_id(context, ""), -EINVAL);
    destroy_context(context);

    ck_assert_int_eq(create_context(&context), 0);
    ck_assert_int_eq(context_set_id(context, "id?bad/name"), -EINVAL);
    destroy_context(context);

    ck_assert_int_eq(create_context(&context), 0);
    // test error flag raise
    ck_assert_int_eq((int)context_has_error(context), 0);
    context_raise_error(context);
    ck_assert_int_eq((int)context_has_error(context), 1);
    ck_assert_int_eq(context_set_id(context, "id3"), -ENOTRECOVERABLE);
    destroy_context(context);
}
END_TEST

START_TEST(test_context_add_permission) {
    context_t *context = NULL;
    ck_assert_int_eq(create_context(&context), 0);
    // test add perm
    ck_assert_int_eq(context_add_permission(context, "perm"), 0);
    ck_assert_int_eq((int)context->permission_set.size, 1);
    ck_assert_str_eq(context->permission_set.permissions[0], "perm");

    // test duplicate perm
    ck_assert_int_eq(context_add_permission(context, "perm"), -EEXIST);

    // test error flag raise
    ck_assert_int_eq((int)context_has_error(context), 0);
    context_raise_error(context);
    ck_assert_int_eq((int)context_has_error(context), 1);
    ck_assert_int_eq(context_add_permission(context, "perm2"), -ENOTRECOVERABLE);
    destroy_context(context);
}
END_TEST

START_TEST(test_context_add_path) {
    context_t *context = NULL;
    ck_assert_int_eq(create_context(&context), 0);
    // test add path
    ck_assert_int_eq(context_add_path(context, "/tmp", "conf"), 0);
    ck_assert_int_eq((int)context->path_set.size, 1);
    ck_assert_str_eq(context->path_set.paths[0]->path, "/tmp");
    ck_assert_int_eq((int)context->path_set.paths[0]->path_type, (int)type_conf);

    // test bad path type
    ck_assert_int_eq(context_add_path(context, "/tmp3", "<unset>"), -EINVAL);

    // test duplicate path
    ck_assert_int_eq(context_add_path(context, "/tmp", "data"), -EEXIST);
    ck_assert_int_eq(context_add_path(context, "/tmp", "conf"), -EEXIST);

    // test error flag raise
    ck_assert_int_eq((int)context_has_error(context), 0);
    context_raise_error(context);
    ck_assert_int_eq((int)context_has_error(context), 1);
    ck_assert_int_eq(context_add_path(context, "/tmp2", "conf"), -ENOTRECOVERABLE);
    destroy_context(context);
}
END_TEST

START_TEST(test_free_context) {
    context_t *context = NULL;
    ck_assert_int_eq(create_context(&context), 0);
    ck_assert_int_eq(context_set_id(context, "id"), 0);
    ck_assert_int_eq(context_add_path(context, "/tmp", "conf"), 0);
    ck_assert_int_eq(context_add_permission(context, "perm"), 0);
    clear_context(context);
    ck_assert_int_eq((int)context->path_set.size, 0);
    ck_assert_int_eq((int)context->permission_set.size, 0);
    destroy_context(context);
}
END_TEST

START_TEST(test_destroy_context) {
    context_t *context = NULL;
    ck_assert_int_eq(create_context(&context), 0);
    ck_assert_int_eq(context_set_id(context, "id"), 0);
    ck_assert_int_eq(context_add_path(context, "/tmp", "conf"), 0);
    ck_assert_int_eq(context_add_permission(context, "perm"), 0);
    destroy_context(context);
}
END_TEST

void test_context(void) {
    addtest(test_init_context);
    addtest(test_create_context);
    addtest(test_context_set_id);
    addtest(test_context_add_permission);
    addtest(test_context_add_path);
    addtest(test_free_context);
    addtest(test_destroy_context);
}
