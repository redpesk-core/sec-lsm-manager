/*
 * Copyright (C) 2020 "IoT.bzh"
 * Author Arthur Guyader <arthur.guyader@iot.bzh>
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

#include "test_permissions.h"

#include <CUnit/Basic.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../permissions.h"

void test_init_permission_set(void) {
    permission_set_t permission_set;
    CU_ASSERT_EQUAL(init_permission_set(&permission_set), 0);
    CU_ASSERT_EQUAL(permission_set.permissions, NULL);
    CU_ASSERT_EQUAL(permission_set.size, 0);
    free_permission_set(&permission_set);
}

void test_free_permission_set(void) {
    permission_set_t permission_set;
    CU_ASSERT_EQUAL(init_permission_set(&permission_set), 0);
    CU_ASSERT_EQUAL(permission_set_add_permission(&permission_set, "perm"), 0);
    free_permission_set(&permission_set);
    CU_ASSERT_EQUAL(permission_set.permissions, NULL);
    CU_ASSERT_EQUAL(permission_set.size, 0);
}

void test_permission_set_add_permission(void) {
    permission_set_t permission_set;
    CU_ASSERT_EQUAL(init_permission_set(&permission_set), 0);
    CU_ASSERT_EQUAL(permission_set_add_permission(&permission_set, "perm"), 0);

    CU_ASSERT_EQUAL(permission_set.size, 1);
    CU_ASSERT_STRING_EQUAL(permission_set.permissions[0], "perm");
    free_permission_set(&permission_set);
}
