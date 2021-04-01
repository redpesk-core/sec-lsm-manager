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

#include "../selinux-template.c"
#include "setup-tests.h"

#define TESTID "testid-binding"
#define TESTID_SELINUX "testid_binding"

START_TEST(test_template_to_module) {
    char tmp_file[200] = {'\0'};
    char tmp_file2[200] = {'\0'};
    ck_assert_int_lt(template_to_module("/tmp/template_path", "/tmp/module_path", TESTID, TESTID_SELINUX), 0);
    create_tmp_file(tmp_file);
    create_tmp_file(tmp_file2);
    ck_assert_int_eq(template_to_module(tmp_file, tmp_file2, TESTID, TESTID_SELINUX), 0);
    remove(tmp_file);
    remove(tmp_file2);
}
END_TEST

START_TEST(test_generate_app_module_if) {
    char tmp_file[200] = {'\0'};
    char tmp_file2[200] = {'\0'};
    ck_assert_int_lt(generate_app_module_if(tmp_file, tmp_file2, TESTID, TESTID_SELINUX), 0);
    create_tmp_file(tmp_file);
    create_tmp_file(tmp_file2);
    ck_assert_int_eq(generate_app_module_if(tmp_file, tmp_file2, TESTID, TESTID_SELINUX), 0);
    remove(tmp_file);
    remove(tmp_file2);
}
END_TEST

START_TEST(test_generate_app_module_te) {
    char tmp_file[200] = {'\0'};
    char tmp_file2[200] = {'\0'};
    ck_assert_int_lt(generate_app_module_te(tmp_file, tmp_file2, TESTID, TESTID_SELINUX), 0);
    create_tmp_file(tmp_file);
    create_tmp_file(tmp_file2);
    ck_assert_int_eq(generate_app_module_te(tmp_file, tmp_file2, TESTID, TESTID_SELINUX), 0);
    remove(tmp_file);
    remove(tmp_file2);
}
END_TEST

// START_TEST(test_generate_app_module_files) {
//     char tmp_file[200] = {'\0'};
//     char tmp_file2[200] = {'\0'};
//     char tmp_dir[200] = {'\0'};

//     create_tmp_file(tmp_file);
//     create_tmp_file(tmp_file2);
//     create_tmp_dir(tmp_dir);

//     selinux_module_t selinux_module;
//     init_selinux_module(&selinux_module, TESTID, tmp_file, tmp_file2, tmp_dir);

//     secure_app_t *secure_app;

//     create_secure_app(&secure_app);

//     ck_assert_int_eq(generate_app_module_files(&selinux_module, secure_app), 0);

//     init_selinux_module(&selinux_module, TESTID, NULL, NULL, tmp_dir);

//     ck_assert_int_eq(generate_app_module_files(&selinux_module, secure_app), 0);

//     destroy_secure_app(secure_app);
//     remove(tmp_file);
//     remove(tmp_file2);
//     rmdir(tmp_dir);
// }
// END_TEST

// START_TEST(test_generate_app_module_fc) {
//     char tmp_file[200] = {'\0'};
//     char tmp_file2[200] = {'\0'};
//     ck_assert_int_lt(template_to_module("/tmp/template_path", "/tmp/module_path", TESTID,
//     TESTID_SELINUX),
//                      0);
//     create_tmp_file(tmp_file);
// }
// END_TEST

void test_selinux_template() {
    addtest(test_template_to_module);
    addtest(test_generate_app_module_if);
    addtest(test_generate_app_module_te);
    // addtest(test_generate_app_module_files);
}
