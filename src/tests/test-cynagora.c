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

#include "setup-tests.h"

#include "action/cynagora-interface.h"
#if SIMULATE_CYNAGORA
#include "simulation/cynagora/cynagora.h"
#else
#include <cynagora.h>
#endif

#define CYNAGORA_SELECT_ALL "#"

START_TEST(test_cynagora_set_policies) {
    char *id = "testid";

    permission_set_t permission_set;
    permission_set_init(&permission_set);

    ck_assert_int_eq(permission_set_add(&permission_set, "perm1"), 0);
    ck_assert_int_eq(permission_set_add(&permission_set, "perm2"), 0);

    ck_assert_int_eq(cynagora_set_policies(id, &permission_set, 0), 0);

    int found = 0;
    permission_set_t permission_set2;
    ck_assert_int_eq(cynagora_get_policies(id, &permission_set2), 0);

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

    ck_assert_int_eq(cynagora_drop_policies(id), 0);

    permission_set_clear(&permission_set);
    permission_set_clear(&permission_set2);
}
END_TEST

START_TEST(test_cynagora_drop_policies) {
    char *id = "testid";

    permission_set_t permission_set;
    permission_set_init(&permission_set);

    ck_assert_int_eq(permission_set_add(&permission_set, "perm1"), 0);
    ck_assert_int_eq(permission_set_add(&permission_set, "perm2"), 0);

    ck_assert_int_eq(cynagora_set_policies(id, &permission_set, 0), 0);

    ck_assert_int_eq(cynagora_drop_policies(id), 0);

    int found = 0;
    permission_set_t permission_set2;
    ck_assert_int_eq(cynagora_get_policies(id, &permission_set2), 0);

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

    permission_set_clear(&permission_set);
    permission_set_clear(&permission_set2);
}
END_TEST

void test_cynagora() {
    addtest(test_cynagora_set_policies);
    addtest(test_cynagora_drop_policies);
}
