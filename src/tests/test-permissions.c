/*
 * Copyright (C) 2020-2021 IoT.bzh Company
 * Author: Arthur Guyader <arthur.guyader@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../permissions.c"
#include "setup-tests.h"

START_TEST(test_init_permission_set) {
    permission_set_t permission_set;
    init_permission_set(&permission_set);
    ck_assert_ptr_eq(permission_set.permissions, NULL);
    ck_assert_int_eq((int)permission_set.size, 0);
    free_permission_set(&permission_set);
}
END_TEST

START_TEST(test_free_permission_set) {
    permission_set_t permission_set;
    init_permission_set(&permission_set);
    ck_assert_int_eq(permission_set_add_permission(&permission_set, "perm"), 0);
    free_permission_set(&permission_set);
    ck_assert_ptr_eq(permission_set.permissions, NULL);
    ck_assert_int_eq((int)permission_set.size, 0);
}
END_TEST

START_TEST(test_permission_set_add_permission) {
    permission_set_t permission_set;
    init_permission_set(&permission_set);
    ck_assert_int_eq(permission_set_add_permission(&permission_set, "perm"), 0);
    ck_assert_int_eq((int)permission_set.size, 1);
    ck_assert_str_eq(permission_set.permissions[0], "perm");
    ck_assert_int_lt(permission_set_add_permission(&permission_set, "m"), 0);
    ck_assert_int_lt(permission_set_add_permission(&permission_set, ""), 0);
    free_permission_set(&permission_set);
}
END_TEST

void test_permissions() {
    addtest(test_init_permission_set);
    addtest(test_free_permission_set);
    addtest(test_permission_set_add_permission);
}
