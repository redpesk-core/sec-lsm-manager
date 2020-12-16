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
