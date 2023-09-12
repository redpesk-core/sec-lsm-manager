/*
 * Copyright (C) 2020-2023 IoT.bzh Company
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

#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "setup-tests.h"

#include "utils.h"
#include "sizes.h"

START_TEST(test_check_file_exists) {
    bool exists;
    char tmp_file[SEC_LSM_MANAGER_MAX_SIZE_PATH] = {'\0'};
    create_tmp_file(tmp_file);
    get_file_informations(tmp_file, &exists, NULL, NULL);
    ck_assert_int_eq((int)exists, 1);
    remove(tmp_file);
    get_file_informations(tmp_file, &exists, NULL, NULL);
    ck_assert_int_eq((int)exists, 0);
}
END_TEST

START_TEST(test_check_dir) {
    bool is_dir;
    char tmp_file[SEC_LSM_MANAGER_MAX_SIZE_PATH] = {'\0'};
    char tmp_dir[SEC_LSM_MANAGER_MAX_SIZE_DIR] = {'\0'};
    create_tmp_file(tmp_file);
    get_file_informations(tmp_file, NULL, NULL, &is_dir);
    ck_assert_int_eq((int)is_dir, 0);
    remove(tmp_file);
    get_file_informations(tmp_file, NULL, NULL, &is_dir);
    ck_assert_int_eq((int)is_dir, 0);
    create_tmp_dir(tmp_dir);
    get_file_informations(tmp_dir, NULL, NULL, &is_dir);
    ck_assert_int_eq((int)is_dir, 1);
    rmdir(tmp_dir);
}
END_TEST

START_TEST(test_check_executable) {
    bool is_exec;
    char tmp_file[SEC_LSM_MANAGER_MAX_SIZE_PATH] = {'\0'};
    char tmp_dir[SEC_LSM_MANAGER_MAX_SIZE_DIR] = {'\0'};
    create_tmp_file(tmp_file);
    get_file_informations(tmp_file, NULL, &is_exec, NULL);
    ck_assert_int_eq((int)is_exec, 0);
    chmod(tmp_file, 0700);
    get_file_informations(tmp_file, NULL, &is_exec, NULL);
    ck_assert_int_eq((int)is_exec, 1);
    remove(tmp_file);
    create_tmp_dir(tmp_dir);
    get_file_informations(tmp_dir, NULL, &is_exec, NULL);
    ck_assert_int_eq((int)is_exec, 0);
    chmod(tmp_dir, 0400);
    get_file_informations(tmp_dir, NULL, &is_exec, NULL);
    ck_assert_int_eq((int)is_exec, 0);
    rmdir(tmp_dir);
    get_file_informations(tmp_dir, NULL, &is_exec, NULL);
    ck_assert_int_eq((int)is_exec, 0);
}
END_TEST

START_TEST(test_remove_file) {
    char tmp_file[SEC_LSM_MANAGER_MAX_SIZE_PATH] = {'\0'};
    create_tmp_file(tmp_file);
    ck_assert_int_eq(remove_file(tmp_file), 0);
    ck_assert_int_lt(remove_file(tmp_file), 0);
}
END_TEST

void test_utils(void) {
    addtest(test_check_file_exists);
    addtest(test_check_dir);
    addtest(test_check_executable);
    addtest(test_remove_file);
}
