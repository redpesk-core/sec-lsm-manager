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

#include "test_utils.h"

#include <check.h>
#include <errno.h>
#include <linux/stat.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../utils.h"
#include "tests.h"

START_TEST(test_check_file_exists) {
    char *path = "test.txt";
    ck_assert_int_eq((int)check_file_exists(path), 0);
    FILE *fp = fopen(path, "w");
    fclose(fp);
    ck_assert_int_eq((int)check_file_exists(path), 1);
    remove(path);
}
END_TEST

START_TEST(test_check_file_type) {
    char *path = "test";
    ck_assert_int_eq((int)check_file_type(path, S_IFREG), 0);
    FILE *fp = fopen(path, "w");
    fclose(fp);
    ck_assert_int_eq((int)check_file_type(path, 123), 0);
    ck_assert_int_eq((int)check_file_type(path, S_IFDIR), 0);
    ck_assert_int_eq((int)check_file_type(path, S_IFREG), 1);
    remove(path);
    mkdir(path, 0700);
    ck_assert_int_eq((int)check_file_type(path, S_IFREG), 0);
    ck_assert_int_eq((int)check_file_type(path, S_IFDIR), 1);
    rmdir(path);
}
END_TEST

START_TEST(test_check_executable) {
    char *path = "test";
    ck_assert_int_eq((int)check_executable(path), 0);
    FILE *fp = fopen(path, "w");
    fclose(fp);
    ck_assert_int_eq((int)check_executable(path), 0);
    chmod(path, 0700);
    ck_assert_int_eq((int)check_executable(path), 1);
    remove(path);
    mkdir(path, 0700);
    ck_assert_int_eq((int)check_executable(path), 1);
    rmdir(path);
}
END_TEST

START_TEST(test_remove_file) {
    char *path = "test";
    FILE *fp = fopen(path, "w");
    fclose(fp);
    ck_assert_int_eq(remove_file(path), 0);
}
END_TEST

void tests_utils(void) {
    addtest(test_check_file_exists);
    addtest(test_check_file_type);
    addtest(test_check_executable);
    addtest(test_remove_file);
}