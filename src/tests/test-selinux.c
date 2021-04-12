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

#include "../selinux-compile.c"
#include "../selinux.c"
#include "./test-selinux-template.c"
#include "setup-tests.h"

START_TEST(test_selinux_process_paths) {
    char etc_tmp_file[200] = {'\0'};
    create_etc_tmp_file(etc_tmp_file);

    path_type_definitions_t path_type_definitions[number_path_type];
    init_path_type_definitions(path_type_definitions, TESTID);

    secure_app_t *secure_app = NULL;
    create_secure_app(&secure_app);
    ck_assert_int_eq(secure_app_add_path(secure_app, etc_tmp_file, type_id), 0);

    ck_assert_int_eq(selinux_process_paths(secure_app, path_type_definitions), 0);

    ck_assert_int_eq(compare_xattr(etc_tmp_file, XATTR_NAME_SELINUX, "system_u:object_r:etc_t:s0"), true);

    ck_assert_int_eq(secure_app_add_path(secure_app, "bad_path", type_id), 0);

    ck_assert_int_eq(selinux_process_paths(secure_app, path_type_definitions), 0);

    remove(etc_tmp_file);
}
END_TEST

START_TEST(test_selinux_install) {
    char tmp_dir[200] = {'\0'};
    create_tmp_dir(tmp_dir);

    char data_dir[100];
    strcpy(data_dir, tmp_dir);
    strcat(data_dir, "/data/");
    char data_file[100];
    strcpy(data_file, tmp_dir);
    strcat(data_file, "/data/");
    strcat(data_file, "data_file");

    char exec_dir[100];
    strcpy(exec_dir, tmp_dir);
    strcat(exec_dir, "/exec/");
    char exec_file[100];
    strcpy(exec_file, tmp_dir);
    strcat(exec_file, "/exec/");
    strcat(exec_file, "exec_file");

    char id_dir[100];
    strcpy(id_dir, tmp_dir);
    strcat(id_dir, "/id/");
    char id_file[100];
    strcpy(id_file, tmp_dir);
    strcat(id_file, "/id/");
    strcat(id_file, "id_file");

    char public_dir[100];
    strcpy(public_dir, tmp_dir);
    strcat(public_dir, "/public/");
    char public_file[100];
    strcpy(public_file, tmp_dir);
    strcat(public_file, "/public/");
    strcat(public_file, "public_file");

    ck_assert_int_eq(mkdir(data_dir, 0777), 0);
    ck_assert_int_eq(mkdir(exec_dir, 0777), 0);
    ck_assert_int_eq(mkdir(id_dir, 0777), 0);
    ck_assert_int_eq(mkdir(public_dir, 0777), 0);
    create_file(data_file);
    create_file(exec_file);
    create_file(id_file);
    create_file(public_file);

    // create secure app
    secure_app_t *secure_app = NULL;
    ck_assert_int_eq(create_secure_app(&secure_app), 0);

    ck_assert_int_lt(install_selinux(secure_app), 0);

    ck_assert_int_eq(secure_app_add_path(secure_app, data_dir, type_data), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, data_file, type_data), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, exec_dir, type_exec), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, exec_file, type_exec), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, id_dir, type_id), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, id_file, type_id), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, public_dir, type_public), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, public_file, type_public), 0);
    ck_assert_int_eq(secure_app_add_permission(secure_app, "perm1"), 0);
    ck_assert_int_eq(secure_app_add_permission(secure_app, "perm2"), 0);
    ck_assert_int_eq(secure_app_set_id(secure_app, "testid-binding"), 0);
    ck_assert_int_eq(install_selinux(secure_app), 0);

    // test settings

    ck_assert_int_eq(compare_xattr(data_dir, XATTR_NAME_SELINUX, "system_u:object_r:testid_binding_data_t:s0"), true);
    ck_assert_int_eq(compare_xattr(data_file, XATTR_NAME_SELINUX, "system_u:object_r:testid_binding_data_t:s0"), true);
    ck_assert_int_eq(compare_xattr(exec_dir, XATTR_NAME_SELINUX, "system_u:object_r:testid_binding_exec_t:s0"), true);
    ck_assert_int_eq(compare_xattr(exec_file, XATTR_NAME_SELINUX, "system_u:object_r:testid_binding_exec_t:s0"), true);
    ck_assert_int_eq(compare_xattr(id_dir, XATTR_NAME_SELINUX, "system_u:object_r:testid_binding_t:s0"), true);
    ck_assert_int_eq(compare_xattr(id_file, XATTR_NAME_SELINUX, "system_u:object_r:testid_binding_t:s0"), true);
    ck_assert_int_eq(compare_xattr(public_dir, XATTR_NAME_SELINUX, "system_u:object_r:redpesk_public_t:s0"), true);
    ck_assert_int_eq(compare_xattr(public_file, XATTR_NAME_SELINUX, "system_u:object_r:redpesk_public_t:s0"), true);

    ck_assert_int_eq(uninstall_selinux(secure_app), 0);

    remove(data_file);
    remove(exec_file);
    remove(id_file);
    remove(public_file);
    rmdir(data_dir);
    rmdir(exec_dir);
    rmdir(id_dir);
    rmdir(public_dir);
    rmdir(tmp_dir);
}
END_TEST

void test_selinux() {
    addtest(test_selinux_process_paths);
    addtest(test_selinux_install);
}
