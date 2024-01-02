/*
 * Copyright (C) 2020-2024 IoT.bzh Company
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

#include "smack-template.h"

START_TEST(test_init_path_type_definitions) {
    path_type_definitions_t path_type_definitions[number_path_type];
    memset(&path_type_definitions, 0, sizeof(path_type_definitions_t) * number_path_type);

    memset(&path_type_definitions, 0, sizeof(path_type_definitions_t) * number_path_type);
    init_path_type_definitions(path_type_definitions, "testid");

    ck_assert_str_eq(path_type_definitions[type_conf].label, "App:testid:Conf");

    ck_assert_str_eq(path_type_definitions[type_data].label, "App:testid:Data");

    ck_assert_str_eq(path_type_definitions[type_exec].label, "App:testid:Exec");

    ck_assert_str_eq(path_type_definitions[type_http].label, "App:testid:Http");

    ck_assert_str_eq(path_type_definitions[type_icon].label, "App:testid:Icon");

    ck_assert_str_eq(path_type_definitions[type_id].label, "App:testid");

    ck_assert_str_eq(path_type_definitions[type_lib].label, "App:testid:Lib");

    ck_assert_str_eq(path_type_definitions[type_public].label, "System:Shared");

    ck_assert_int_eq(path_type_definitions[type_exec].is_executable, 1);

    ck_assert_int_eq(path_type_definitions[type_data].is_transmute, 1);
    ck_assert_int_eq(path_type_definitions[type_http].is_transmute, 1);
    ck_assert_int_eq(path_type_definitions[type_id].is_transmute, 1);
    ck_assert_int_eq(path_type_definitions[type_lib].is_transmute, 1);
    ck_assert_int_eq(path_type_definitions[type_public].is_transmute, 1);
}
END_TEST

void test_smack_label() { addtest(test_init_path_type_definitions); }
