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

#include "../smack.c"
#include "./test-smack-label.c"
#include "setup-tests.h"

START_TEST(test_set_smack) {
    char tmp_dir[200] = {'\0'};
    char path[200] = {'\0'};
    char val[200] = {'\0'};
    char label[200] = {'\0'};
    // path and label not set + file not created
    ck_assert_int_lt(set_smack(path, XATTR_NAME_SMACK, label), 0);
    // set label
    strcpy(label, "label");
    ck_assert_int_lt(set_smack(path, XATTR_NAME_SMACK, label), 0);
    // set path
    create_tmp_dir(tmp_dir);
    strcpy(path, tmp_dir);
    strcat(path, "/test.txt");
    ck_assert_int_lt(set_smack(path, XATTR_NAME_SMACK, label), 0);
    // create file
    create_file(path);
    ck_assert_int_eq(set_smack(path, XATTR_NAME_SMACK, label), 0);
    ck_assert_int_gt(lgetxattr(path, XATTR_NAME_SMACK, val, 200), 0);
    ck_assert_str_eq(val, label);
    ck_assert_int_eq(remove(path), 0);
    ck_assert_int_eq(rmdir(tmp_dir), 0);
}
END_TEST

START_TEST(test_generate_app_label) {
    char *id = "id";
    char *label = NULL;
    ck_assert_int_eq(generate_label(&label, id, "App::", "::Lib"), 0);
    ck_assert_str_eq(label, "App::id::Lib");
    free(label);
}
END_TEST

START_TEST(test_label_file) {
    char path[200] = {'\0'};
    char label[200] = {'\0'};
    char tmp_dir[200] = {'\0'};
    // path and label not set + file not created
    ck_assert_int_lt(label_file(path, label), 0);
    // set path
    create_tmp_dir(tmp_dir);
    strcpy(path, tmp_dir);
    strcat(path, "/test.txt");
    ck_assert_int_lt(label_file(path, label), 0);
    // set label
    strcpy(label, "label");
    ck_assert_int_lt(label_file(path, label), 0);
    // create file
    create_file(path);
    ck_assert_int_eq(label_file(path, label), 0);
    // set label = ""
    strcpy(label, "");
    ck_assert_int_lt(label_file(path, label), 0);
    remove(path);
}
END_TEST

START_TEST(test_label_dir_transmute) {
    char path[200] = {'\0'};
    char tmp_dir[200] = {'\0'};
    // path not set + dir not created
    ck_assert_int_eq(label_dir_transmute(tmp_dir), 0);
    // create dir
    create_tmp_dir(tmp_dir);
    // set path
    strcpy(path, tmp_dir);
    strcat(path, "/test.txt");
    ck_assert_int_eq(label_dir_transmute(tmp_dir), 0);
    ck_assert_int_eq(label_dir_transmute(path), 0);
    // create file
    create_file(path);
    ck_assert_int_eq(label_dir_transmute(path), 0);
    remove(path);
    rmdir(tmp_dir);
}
END_TEST

START_TEST(test_label_exec) {
    char path[200] = {'\0'};
    char label[200] = {'\0'};
    char tmp_dir[200] = {'\0'};
    // path and label not set + file not created
    ck_assert_int_eq(label_exec(path, label), 0);
    // create dir
    create_tmp_dir(tmp_dir);
    // set path
    strcpy(path, tmp_dir);
    strcat(path, "/test.bin");
    ck_assert_int_eq(label_exec(path, label), 0);
    // set label
    strcpy(label, "label");
    ck_assert_int_eq(label_exec(path, label), 0);
    // create file
    create_file(path);
    ck_assert_int_eq(label_exec(path, label), -EINVAL);
    // set label with suffix :Exec
    strcat(label, suffix_exec);
    ck_assert_int_eq(label_exec(path, label), 0);
    remove(path);
    rmdir(tmp_dir);
}
END_TEST

START_TEST(test_label_path) {
    char path[200] = {'\0'};
    char path2[200] = {'\0'};
    char label[200] = {'\0'};
    char tmp_dir[200] = {'\0'};

    // path not set + file and dir not created
    strcpy(label, "label");
    ck_assert_int_lt(label_path(tmp_dir, label, 0, 1), 0);
    ck_assert_int_lt(label_path(path, label, 1, 1), 0);

    // create dir
    create_tmp_dir(tmp_dir);
    strcpy(path, tmp_dir);
    strcat(path, "/test.txt");
    // create file
    create_file(path);

    // label dir with label and transmute
    ck_assert_int_eq(label_path(tmp_dir, label, 0, 1), 0);
    // label file with label
    ck_assert_int_eq(label_path(path, label, 0, 0), 0);

    strcpy(path2, tmp_dir);
    strcat(path2, "/test.bin");
    // create file 2
    create_file(path2);

    // label file 2 with label and executable
    ck_assert_int_eq(label_path(path2, label, 1, 0), -EINVAL);

    // set label with suffix :Exec
    strcat(label, suffix_exec);

    // label file 2 with label+suffix and executable
    ck_assert_int_eq(label_path(path2, label, 1, 0), 0);

    remove(path);
    remove(path2);
    rmdir(tmp_dir);
}
END_TEST

START_TEST(test_smack_install) {
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
    ck_assert_int_eq(secure_app_set_id(secure_app, "testid"), 0);
    ck_assert_int_eq(install_smack(secure_app), 0);

    // test settings

    ck_assert_int_eq(check_file_exists("/etc/smack/accesses.d/app-testid"), true);
    ck_assert_int_eq(compare_xattr(data_dir, XATTR_NAME_SMACK, "App:testid:Data"), true);
    ck_assert_int_eq(compare_xattr(data_dir, XATTR_NAME_SMACKTRANSMUTE, "TRUE"), true);
    ck_assert_int_eq(compare_xattr(data_file, XATTR_NAME_SMACK, "App:testid:Data"), true);
    ck_assert_int_eq(compare_xattr(exec_dir, XATTR_NAME_SMACK, "App:testid:Exec"), true);
    ck_assert_int_eq(compare_xattr(exec_file, XATTR_NAME_SMACK, "App:testid:Exec"), true);
    ck_assert_int_eq(compare_xattr(exec_file, XATTR_NAME_SMACKEXEC, "App:testid"), true);
    ck_assert_int_eq(compare_xattr(id_dir, XATTR_NAME_SMACK, "App:testid"), true);
    ck_assert_int_eq(compare_xattr(id_dir, XATTR_NAME_SMACKTRANSMUTE, "TRUE"), true);
    ck_assert_int_eq(compare_xattr(id_file, XATTR_NAME_SMACK, "App:testid"), true);
    ck_assert_int_eq(compare_xattr(public_dir, XATTR_NAME_SMACK, "_"), true);
    ck_assert_int_eq(compare_xattr(public_dir, XATTR_NAME_SMACKTRANSMUTE, "TRUE"), true);
    ck_assert_int_eq(compare_xattr(public_file, XATTR_NAME_SMACK, "_"), true);

    ck_assert_int_eq(uninstall_smack(secure_app), 0);

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

START_TEST(test_smack_uninstall) {
    char tmp_dir[200] = {'\0'};
    create_tmp_dir(tmp_dir);

    char data_dir[100];
    strcpy(data_dir, tmp_dir);
    strcat(data_dir, "/data/");
    char data_file[100];
    strcpy(data_file, tmp_dir);
    strcat(data_file, "/data/");
    strcat(data_file, "data_file");

    ck_assert_int_eq(mkdir(data_dir, 0777), 0);
    create_file(data_file);

    // create secure app
    secure_app_t *secure_app = NULL;
    ck_assert_int_eq(create_secure_app(&secure_app), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, data_dir, type_data), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, data_file, type_data), 0);
    ck_assert_int_eq(secure_app_set_id(secure_app, "testid"), 0);
    ck_assert_int_eq(install_smack(secure_app), 0);

    ck_assert_int_eq(uninstall_smack(secure_app), 0);

    ck_assert_int_eq(check_file_exists("/etc/smack/accesses.d/app-testid"), 0);

    remove(data_file);
    rmdir(data_dir);
    rmdir(tmp_dir);
}
END_TEST

void test_smack() {
    addtest(test_set_smack);
    addtest(test_generate_app_label);
    addtest(test_label_file);
    addtest(test_label_dir_transmute);
    addtest(test_label_exec);
    addtest(test_label_path);
    addtest(test_smack_install);
    addtest(test_smack_uninstall);
}
