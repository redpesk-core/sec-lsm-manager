/*
 * Copyright (C) 2020 "IoT.bzh"
 * Author Arthur Guyader <arthur.guyader@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "test_utils.h"

#include <CUnit/Basic.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../utils.h"

void test_check_file_exists(void) {
    char *path = NULL;
    CU_ASSERT_EQUAL(check_file_exists(path), false)
    path = "test.txt";
    CU_ASSERT_EQUAL(check_file_exists(path), false)
    FILE *fp = fopen(path, "w");
    fclose(fp);
    CU_ASSERT_EQUAL(check_file_exists(path), true);
    remove(path);
}

void test_check_file_type(void) {
    char *path = NULL;
    CU_ASSERT_EQUAL(check_file_type(path, S_IFREG), false);
    path = "test";
    CU_ASSERT_EQUAL(check_file_type(path, S_IFREG), false);
    FILE *fp = fopen(path, "w");
    fclose(fp);
    CU_ASSERT_EQUAL(check_file_type(path, 123), false);
    CU_ASSERT_EQUAL(check_file_type(path, S_IFDIR), false);
    CU_ASSERT_EQUAL(check_file_type(path, S_IFREG), true);
    remove(path);
    mkdir(path, 0700);
    CU_ASSERT_EQUAL(check_file_type(path, S_IFREG), false);
    CU_ASSERT_EQUAL(check_file_type(path, S_IFDIR), true);
    rmdir(path);
}

void test_check_executable(void) {
    char *path = NULL;
    CU_ASSERT_EQUAL(check_executable(path), false);
    path = "test";
    CU_ASSERT_EQUAL(check_executable(path), false);
    FILE *fp = fopen(path, "w");
    fclose(fp);
    CU_ASSERT_EQUAL(check_executable(path), false);
    chmod(path, 0700);
    CU_ASSERT_EQUAL(check_executable(path), true);
    remove(path);
    mkdir(path, 0700);
    CU_ASSERT_EQUAL(check_executable(path), true);
    rmdir(path);
}

void test_remove_file(void) {
    char *path = NULL;
    CU_ASSERT_EQUAL(remove_file(path), -EINVAL);
    path = "test";
    FILE *fp = fopen(path, "w");
    fclose(fp);
    CU_ASSERT_EQUAL(remove_file(path), 0);
}
