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

#include "test_policies.h"

#include <CUnit/Basic.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../policies.h"

void test_init_policies(void) {
    CU_ASSERT_EQUAL(init_policies(NULL), -EINVAL);
    policies_t policies;
    CU_ASSERT_EQUAL(init_policies(&policies), 0);
    CU_ASSERT_EQUAL(policies.policies, NULL);
    CU_ASSERT_EQUAL(policies.size, 0);
    free_policies(&policies);
}

void test_free_policies(void) {
    policies_t policies;
    CU_ASSERT_EQUAL(init_policies(&policies), 0);
    cynagora_key_t k = {"id", SELECT_ALL, SELECT_ALL, SELECT_ALL};
    cynagora_value_t v = {AUTHORIZED, 0};
    CU_ASSERT_EQUAL(policies_add_policy(&policies, &k, &v), 0);
    free_policies(&policies);
    CU_ASSERT_EQUAL(policies.policies, NULL);
    CU_ASSERT_EQUAL(policies.size, 0);
}

void test_policies_add_policy(void) {
    policies_t policies;
    CU_ASSERT_EQUAL(init_policies(&policies), 0);
    cynagora_key_t k = {"id", SELECT_ALL, SELECT_ALL, SELECT_ALL};
    cynagora_value_t v = {AUTHORIZED, 0};
    CU_ASSERT_EQUAL(policies_add_policy(&policies, &k, &v), 0);

    CU_ASSERT_EQUAL(policies.size, 1);
    CU_ASSERT_STRING_EQUAL(policies.policies[0].k.client, "id");
    CU_ASSERT_STRING_EQUAL(policies.policies[0].v.value, AUTHORIZED);
    free_policies(&policies);
}
