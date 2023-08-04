/*
 * Copyright (C) 2020-2023 IoT.bzh Company
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

#include "../cynagora-interface.c"
#include "setup-tests.h"

static void list(void *closure, const cynagora_key_t *key, const cynagora_value_t *value) {
    (void)value;
    permission_set_t *permission_set = closure;
    ck_assert_int_eq(permission_set_add_permission(permission_set, key->permission), 0);
}

static int cynagora_get_policies(cynagora_t *cynagora, const char *client, permission_set_t *permission_set) {
    init_permission_set(permission_set);
    if (cynagora_enter(cynagora) < 0) {
        return -1;
    }
    cynagora_key_t k = {client, CYNAGORA_SELECT_ALL, CYNAGORA_SELECT_ALL, CYNAGORA_SELECT_ALL};

    if (cynagora_get(cynagora, &k, list, permission_set) < 0) {
        return -2;
    }

    if (cynagora_leave(cynagora, 1) < 0) {
        return -3;
    }

    return 0;
}

START_TEST(test_cynagora_set_policies) {
    cynagora_t *cynagora_admin_client = NULL;
    char *id = "testid";
    ck_assert_int_eq(cynagora_create(&cynagora_admin_client, cynagora_Admin, 1, 0), 0);

    permission_set_t permission_set;
    init_permission_set(&permission_set);

    ck_assert_int_eq(permission_set_add_permission(&permission_set, "perm1"), 0);
    ck_assert_int_eq(permission_set_add_permission(&permission_set, "perm2"), 0);

    ck_assert_int_eq(cynagora_set_policies(cynagora_admin_client, id, &permission_set, 0), 0);

    int found = 0;
    permission_set_t permission_set2;
    ck_assert_int_eq(cynagora_get_policies(cynagora_admin_client, id, &permission_set2), 0);

    for (size_t i = 0; i < permission_set.size; i++) {
        for (size_t j = 0; j < permission_set2.size; j++) {
            if (!strcmp(permission_set.permissions[i], permission_set2.permissions[j])) {
                found = 1;
                break;
            }
        }
        ck_assert_int_eq(found, 1);
        found = 0;
    }

    ck_assert_int_eq(cynagora_drop_policies(cynagora_admin_client, id), 0);

    free_permission_set(&permission_set);
    free_permission_set(&permission_set2);
    cynagora_destroy(cynagora_admin_client);
}
END_TEST

START_TEST(test_cynagora_drop_policies) {
    cynagora_t *cynagora_admin_client = NULL;
    char *id = "testid";
    ck_assert_int_eq(cynagora_create(&cynagora_admin_client, cynagora_Admin, 1, 0), 0);

    permission_set_t permission_set;
    init_permission_set(&permission_set);

    ck_assert_int_eq(permission_set_add_permission(&permission_set, "perm1"), 0);
    ck_assert_int_eq(permission_set_add_permission(&permission_set, "perm2"), 0);

    ck_assert_int_eq(cynagora_set_policies(cynagora_admin_client, id, &permission_set, 0), 0);

    ck_assert_int_eq(cynagora_drop_policies(cynagora_admin_client, id), 0);

    int found = 0;
    permission_set_t permission_set2;
    ck_assert_int_eq(cynagora_get_policies(cynagora_admin_client, id, &permission_set2), 0);

    for (size_t i = 0; i < permission_set.size; i++) {
        for (size_t j = 0; j < permission_set2.size; j++) {
            if (!strcmp(permission_set.permissions[i], permission_set2.permissions[j])) {
                found = 1;
                break;
            }
        }
        ck_assert_int_eq(found, 0);
        found = 0;
    }

    free_permission_set(&permission_set);
    free_permission_set(&permission_set2);
    cynagora_destroy(cynagora_admin_client);
}
END_TEST

void test_cynagora() {
    addtest(test_cynagora_set_policies);
    addtest(test_cynagora_drop_policies);
}
