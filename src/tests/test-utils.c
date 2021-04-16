/*
 * Copyright (C) 2020-2021 IoT.bzh Company
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

#include "../utils.c"
#include "setup-tests.h"

START_TEST(test_check_file_exists) {
    char tmp_file[SEC_LSM_MANAGER_MAX_SIZE_PATH] = {'\0'};
    create_tmp_file(tmp_file);
    ck_assert_int_eq((int)check_file_exists(tmp_file), 1);
    remove(tmp_file);
    ck_assert_int_eq((int)check_file_exists(tmp_file), 0);
}
END_TEST

START_TEST(test_check_file_type) {
    char tmp_file[SEC_LSM_MANAGER_MAX_SIZE_PATH] = {'\0'};
    char tmp_dir[SEC_LSM_MANAGER_MAX_SIZE_DIR] = {'\0'};
    create_tmp_file(tmp_file);
    ck_assert_int_eq((int)check_file_type(tmp_file, 123), 0);
    ck_assert_int_eq((int)check_file_type(tmp_file, S_IFDIR), 0);
    ck_assert_int_eq((int)check_file_type(tmp_file, S_IFREG), 1);
    remove(tmp_file);
    ck_assert_int_eq((int)check_file_type(tmp_file, S_IFREG), 0);
    create_tmp_dir(tmp_dir);
    ck_assert_int_eq((int)check_file_type(tmp_dir, S_IFREG), 0);
    ck_assert_int_eq((int)check_file_type(tmp_dir, S_IFDIR), 1);
    rmdir(tmp_dir);
}
END_TEST

START_TEST(test_check_executable) {
    char tmp_file[SEC_LSM_MANAGER_MAX_SIZE_PATH] = {'\0'};
    char tmp_dir[SEC_LSM_MANAGER_MAX_SIZE_DIR] = {'\0'};
    create_tmp_file(tmp_file);
    ck_assert_int_eq((int)check_executable(tmp_file), 0);
    chmod(tmp_file, 0700);
    ck_assert_int_eq((int)check_executable(tmp_file), 1);
    remove(tmp_file);
    create_tmp_dir(tmp_dir);
    ck_assert_int_eq((int)check_executable(tmp_dir), 1);
    chmod(tmp_dir, 0400);
    ck_assert_int_eq((int)check_executable(tmp_dir), 0);
    rmdir(tmp_dir);
    ck_assert_int_eq((int)check_executable(tmp_file), 0);
}
END_TEST

START_TEST(test_remove_file) {
    char tmp_file[SEC_LSM_MANAGER_MAX_SIZE_PATH] = {'\0'};
    create_tmp_file(tmp_file);
    ck_assert_int_eq(remove_file(tmp_file), 0);
    ck_assert_int_lt(remove_file(tmp_file), 0);
}
END_TEST

void test_utils() {
    addtest(test_check_file_exists);
    addtest(test_check_file_type);
    addtest(test_check_executable);
    addtest(test_remove_file);
}
