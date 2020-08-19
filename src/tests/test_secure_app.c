/*
 * Copyright (C) 2020 "IoT.bzh"
 * Author Arthur Guyader <arthur.guyader@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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