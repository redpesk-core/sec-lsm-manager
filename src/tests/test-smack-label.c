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

#include "../smack-label.c"
#include "../smack-template.c"
#include "setup-tests.h"

START_TEST(test_generate_label) {
    char *label = NULL;
    char id[200] = {'\0'};
    char prefix[200] = {'\0'};
    char suffix[200] = {'\0'};

    ck_assert_int_lt(generate_label(&label, id, prefix, suffix), 0);

    strcpy(id, "testid");

    ck_assert_int_eq(generate_label(&label, id, prefix, suffix), 0);

    ck_assert_str_eq(label, "testid");

    free(label);
    label = NULL;

    strcpy(prefix, "App:");

    ck_assert_int_eq(generate_label(&label, id, prefix, suffix), 0);

    ck_assert_str_eq(label, "App:testid");

    free(label);
    label = NULL;

    strcpy(suffix, ":Conf");
    ck_assert_int_eq(generate_label(&label, id, prefix, suffix), 0);

    ck_assert_str_eq(label, "App:testid:Conf");

    free(label);
    label = NULL;

    strcpy(suffix, ":Con£f");

    ck_assert_int_lt(generate_label(&label, id, prefix, suffix), 0);
}

START_TEST(test_init_path_type_definitions) {
    path_type_definitions_t path_type_definitions[number_path_type];
    memset(&path_type_definitions, 0, sizeof(path_type_definitions_t) * number_path_type);

    ck_assert_int_lt(init_path_type_definitions(path_type_definitions, "tes£tid"), 0);

    memset(&path_type_definitions, 0, sizeof(path_type_definitions_t) * number_path_type);
    ck_assert_int_eq(init_path_type_definitions(path_type_definitions, "testid"), 0);

    ck_assert_str_eq(path_type_definitions[type_conf].label, "App:testid:Conf");

    ck_assert_str_eq(path_type_definitions[type_data].label, "App:testid:Data");

    ck_assert_str_eq(path_type_definitions[type_exec].label, "App:testid:Exec");

    ck_assert_str_eq(path_type_definitions[type_http].label, "App:testid:Http");

    ck_assert_str_eq(path_type_definitions[type_icon].label, "App:testid:Icon");

    ck_assert_str_eq(path_type_definitions[type_id].label, "App:testid");

    ck_assert_str_eq(path_type_definitions[type_lib].label, "App:testid:Lib");

    ck_assert_str_eq(path_type_definitions[type_public].label, "_");

    ck_assert_int_eq(path_type_definitions[type_exec].is_executable, 1);

    ck_assert_int_eq(path_type_definitions[type_data].is_transmute, 1);
    ck_assert_int_eq(path_type_definitions[type_http].is_transmute, 1);
    ck_assert_int_eq(path_type_definitions[type_id].is_transmute, 1);
    ck_assert_int_eq(path_type_definitions[type_lib].is_transmute, 1);
    ck_assert_int_eq(path_type_definitions[type_public].is_transmute, 1);

    free_path_type_definitions(path_type_definitions);
}
END_TEST

void test_smack_label() {
    addtest(test_generate_label);
    addtest(test_init_path_type_definitions);
}
