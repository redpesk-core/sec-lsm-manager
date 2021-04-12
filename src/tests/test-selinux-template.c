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

START_TEST(test_generate_app_module_fc) {
    char fs_file[200] = "/sys/fs/test";
    char tmp_file[200] = {'\0'};

    path_type_definitions_t path_type_definitions[number_path_type];
    init_path_type_definitions(path_type_definitions, TESTID);

    secure_app_t *secure_app = NULL;
    create_secure_app(&secure_app);
    ck_assert_int_eq(secure_app_add_path(secure_app, "/tmp/data", type_data), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, "/tmp/conf", type_conf), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, "/tmp/lib", type_lib), 0);
    ck_assert_int_lt(generate_app_module_fc(fs_file, secure_app, path_type_definitions), 0);

    create_tmp_file(tmp_file);
    ck_assert_int_eq(generate_app_module_fc(tmp_file, secure_app, path_type_definitions), 0);
    remove(tmp_file);
}
END_TEST

START_TEST(test_generate_app_module_files) {
    char tmp_dir[200] = {'\0'};
    create_tmp_dir(tmp_dir);

    secure_app_t *secure_app = NULL;
    create_secure_app(&secure_app);
    ck_assert_int_eq(secure_app_set_id(secure_app, TESTID), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, "/tmp", type_conf), 0);

    selinux_module_t selinux_module = {0};
    path_type_definitions_t path_type_definitions[number_path_type];
    init_path_type_definitions(path_type_definitions, TESTID);

    DEBUG("generate_app_module_files");

    ck_assert_int_lt(generate_app_module_files(&selinux_module, secure_app, path_type_definitions), 0);

    strncpy(selinux_module.selinux_te_template_file, "/usr/share/sec-lsm-manager/app-template.te",
            SEC_LSM_MANAGER_MAX_SIZE_PATH);
    snprintf(selinux_module.selinux_te_file, SEC_LSM_MANAGER_MAX_SIZE_PATH, "%s/%s", tmp_dir, "tefile");

    ck_assert_int_lt(generate_app_module_files(&selinux_module, secure_app, path_type_definitions), 0);

    strncpy(selinux_module.selinux_if_template_file, "/usr/share/sec-lsm-manager/app-template.if",
            SEC_LSM_MANAGER_MAX_SIZE_PATH);
    snprintf(selinux_module.selinux_if_file, SEC_LSM_MANAGER_MAX_SIZE_PATH, "%s/%s", tmp_dir, "iffile");

    ck_assert_int_lt(generate_app_module_files(&selinux_module, secure_app, path_type_definitions), 0);

    snprintf(selinux_module.selinux_fc_file, SEC_LSM_MANAGER_MAX_SIZE_PATH, "%s/%s", tmp_dir, "fcfile");

    remove(selinux_module.selinux_fc_file);
    remove(selinux_module.selinux_if_file);
    remove(selinux_module.selinux_te_file);
    rmdir(tmp_dir);
}
END_TEST

void test_selinux_template() {
    addtest(test_generate_app_module_fc);
    addtest(test_generate_app_module_files);
}
