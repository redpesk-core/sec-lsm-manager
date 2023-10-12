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

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <linux/xattr.h>

#if SIMULATE_SMACK
#include "simulation/smack/smack.h"
#else
#include <sys/smack.h>
#endif

#include "utils.h"
#include "context.h"
#include "smack.h"
#include "smack-template.h"

// START_TEST(test_label_file) {
//     char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL] = {'\0'};
//     char tmp_file[SEC_LSM_MANAGER_MAX_SIZE_PATH] = {'\0'};
//     // path and label not set + file not created
//     ck_assert_int_lt(label_file(tmp_file, label), 0);
//     // set label
//     secure_strncpy(label, "label", SEC_LSM_MANAGER_MAX_SIZE_LABEL);
//     ck_assert_int_lt(label_file(tmp_file, label), 0);
//     // create file
//     create_tmp_file(tmp_file);
//     ck_assert_int_eq(label_file(tmp_file, label), 0);
//     // set label = ""
//     secure_strncpy(label, "", SEC_LSM_MANAGER_MAX_SIZE_LABEL);
//     ck_assert_int_lt(label_file(tmp_file, label), 0);
//     remove(tmp_file);
// }
// END_TEST

// START_TEST(test_label_dir_transmute) {
//     char path[SEC_LSM_MANAGER_MAX_SIZE_PATH] = {'\0'};
//     char tmp_dir[SEC_LSM_MANAGER_MAX_SIZE_DIR] = {'\0'};
//     // path not set + dir not created
//     ck_assert_int_eq(label_dir_transmute(tmp_dir), 0);
//     // create dir
//     create_tmp_dir(tmp_dir);
//     // set path
//     snprintf(path, SEC_LSM_MANAGER_MAX_SIZE_PATH, "%s/test.txt", tmp_dir);
//     ck_assert_int_eq(label_dir_transmute(tmp_dir), 0);
//     ck_assert_int_eq(label_dir_transmute(path), 0);
//     // create file
//     ck_assert_int_eq(create_file(path), 0);
//     ck_assert_int_eq(label_dir_transmute(path), 0);
//     remove(path);
//     rmdir(tmp_dir);
// }
// END_TEST

START_TEST(test_label_path) {
    char path[SEC_LSM_MANAGER_MAX_SIZE_PATH] = {'\0'};
    char path2[SEC_LSM_MANAGER_MAX_SIZE_PATH] = {'\0'};
    char label[SEC_LSM_MANAGER_MAX_SIZE_LABEL] = {'\0'};
    char tmp_dir[SEC_LSM_MANAGER_MAX_SIZE_DIR] = {'\0'};

    // path not set + file and dir not created
    secure_strncpy(label, "label", SEC_LSM_MANAGER_MAX_SIZE_LABEL);
    ck_assert_int_lt(set_path_labels(tmp_dir, label, 0, 1), 0);
    ck_assert_int_lt(set_path_labels(path, label, label, 1), 0);

    // create dir
    create_tmp_dir(tmp_dir);
    snprintf(path, SEC_LSM_MANAGER_MAX_SIZE_PATH, "%s/test.txt", tmp_dir);
    // create file
    ck_assert_int_eq(create_file(path), 0);

    // label dir with label and transmute
    ck_assert_int_eq(set_path_labels(tmp_dir, label, 0, 1), 0);
    // label file with label
    ck_assert_int_eq(set_path_labels(path, label, 0, 0), 0);

    snprintf(path2, SEC_LSM_MANAGER_MAX_SIZE_PATH, "%s/test.bin", tmp_dir);
    // create file 2
    ck_assert_int_eq(create_file(path2), 0);

    // label file 2 with label+suffix and executable
    ck_assert_int_eq(set_path_labels(path2, label, label, 0), 0);

    remove(path);
    remove(path2);
    rmdir(tmp_dir);
}
END_TEST

START_TEST(test_smack_install) {
    char tmp_dir[SEC_LSM_MANAGER_MAX_SIZE_DIR] = {'\0'};
    create_tmp_dir(tmp_dir);

    char data_dir[SEC_LSM_MANAGER_MAX_SIZE_DIR+20];
    char data_file[SEC_LSM_MANAGER_MAX_SIZE_PATH+20];
    char exec_dir[SEC_LSM_MANAGER_MAX_SIZE_DIR+20];
    char exec_file[SEC_LSM_MANAGER_MAX_SIZE_PATH+20];
    char id_dir[SEC_LSM_MANAGER_MAX_SIZE_DIR+20];
    char id_file[SEC_LSM_MANAGER_MAX_SIZE_PATH+20];
    char public_dir[SEC_LSM_MANAGER_MAX_SIZE_DIR+20];
    char public_file[SEC_LSM_MANAGER_MAX_SIZE_PATH+20];

    snprintf(data_dir, sizeof data_dir, "%s/data/", tmp_dir);
    snprintf(data_file, sizeof data_file, "%s/data/data_file", tmp_dir);

    snprintf(exec_dir, sizeof exec_dir, "%s/exec/", tmp_dir);
    snprintf(exec_file, sizeof exec_file, "%s/exec/exec_file", tmp_dir);

    snprintf(id_dir, sizeof id_dir, "%s/id/", tmp_dir);
    snprintf(id_file, sizeof id_file, "%s/id/id_file", tmp_dir);

    snprintf(public_dir, sizeof public_dir, "%s/public/", tmp_dir);
    snprintf(public_file, sizeof public_file, "%s/public/public_file", tmp_dir);

    ck_assert_int_eq(mkdir(data_dir, 0777), 0);
    ck_assert_int_eq(mkdir(exec_dir, 0777), 0);
    ck_assert_int_eq(mkdir(id_dir, 0777), 0);
    ck_assert_int_eq(mkdir(public_dir, 0777), 0);
    ck_assert_int_eq(create_file(data_file), 0);
    ck_assert_int_eq(create_file(exec_file), 0);
    ck_assert_int_eq(create_file(id_file), 0);
    ck_assert_int_eq(create_file(public_file), 0);

    // create secure app
    secure_app_t *secure_app = NULL;
    ck_assert_int_eq(create_secure_app(&secure_app), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, data_dir, "data"), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, data_file, "data"), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, exec_dir, "exec"), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, exec_file, "exec"), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, id_dir, "id"), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, id_file, "id"), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, public_dir, "public"), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, public_file, "public"), 0);
    ck_assert_int_eq(secure_app_add_permission(secure_app, "perm1"), 0);
    ck_assert_int_eq(secure_app_add_permission(secure_app, "perm2"), 0);
    ck_assert_int_eq(secure_app_set_id(secure_app, "testid"), 0);
    ck_assert_int_eq(install_smack(secure_app), 0);

    // test settings

    bool exists;
    get_file_informations("/etc/smack/accesses.d/testid.smack", &exists, NULL, NULL);
    ck_assert_int_eq(exists, true);
    ck_assert_int_eq(compare_xattr(data_dir, XATTR_NAME_SMACK, "App:testid:Data"), true);
    ck_assert_int_eq(compare_xattr(data_dir, XATTR_NAME_SMACKTRANSMUTE, "TRUE"), true);
    ck_assert_int_eq(compare_xattr(data_file, XATTR_NAME_SMACK, "App:testid:Data"), true);
    ck_assert_int_eq(compare_xattr(exec_dir, XATTR_NAME_SMACK, "App:testid:Exec"), true);
    ck_assert_int_eq(compare_xattr(exec_file, XATTR_NAME_SMACK, "App:testid:Exec"), true);
    ck_assert_int_eq(compare_xattr(exec_file, XATTR_NAME_SMACKEXEC, "App:testid"), true);
    ck_assert_int_eq(compare_xattr(id_dir, XATTR_NAME_SMACK, "App:testid"), true);
    ck_assert_int_eq(compare_xattr(id_dir, XATTR_NAME_SMACKTRANSMUTE, "TRUE"), true);
    ck_assert_int_eq(compare_xattr(id_file, XATTR_NAME_SMACK, "App:testid"), true);
    ck_assert_int_eq(compare_xattr(public_dir, XATTR_NAME_SMACK, "System:Shared"), true);
    ck_assert_int_eq(compare_xattr(public_dir, XATTR_NAME_SMACKTRANSMUTE, "TRUE"), true);
    ck_assert_int_eq(compare_xattr(public_file, XATTR_NAME_SMACK, "System:Shared"), true);

    ck_assert_int_eq(uninstall_smack(secure_app), 0);

    destroy_secure_app(secure_app);
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
    char tmp_dir[SEC_LSM_MANAGER_MAX_SIZE_DIR] = {'\0'};
    create_tmp_dir(tmp_dir);

    char data_dir[SEC_LSM_MANAGER_MAX_SIZE_DIR + 20];
    char data_file[SEC_LSM_MANAGER_MAX_SIZE_PATH + 20];
    snprintf(data_dir, sizeof data_dir, "%s/data/", tmp_dir);
    snprintf(data_file, sizeof data_file, "%s/data/data_file", tmp_dir);

    ck_assert_int_eq(mkdir(data_dir, 0777), 0);
    ck_assert_int_eq(create_file(data_file), 0);

    // create secure app
    secure_app_t *secure_app = NULL;
    ck_assert_int_eq(create_secure_app(&secure_app), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, data_dir, "data"), 0);
    ck_assert_int_eq(secure_app_add_path(secure_app, data_file, "data"), 0);
    ck_assert_int_eq(secure_app_set_id(secure_app, "testid"), 0);
    ck_assert_int_eq(install_smack(secure_app), 0);

    ck_assert_int_eq(uninstall_smack(secure_app), 0);

    bool exists;
    get_file_informations("/etc/smack/accesses.d/app-testid", &exists, NULL, NULL);
    ck_assert_int_eq(exists, false);

    destroy_secure_app(secure_app);
    remove(data_file);
    rmdir(data_dir);
    rmdir(tmp_dir);
}
END_TEST

void test_smack() {
    addtest(test_label_path);
    addtest(test_smack_install);
    addtest(test_smack_uninstall);
}
