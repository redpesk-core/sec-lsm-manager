/*
 * Copyright (C) 2020 "IoT.bzh"
 * Author Arthur Guyader <arthur.guyader@iot.bzh>
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

#include "test_smack.h"

#include <CUnit/Basic.h>
#include <fcntl.h>
#include <linux/xattr.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <unistd.h>

#include "../security-manager.h"
#include "../utils.h"

#if !defined(DEFAULT_TESTS_DIR)
#define DEFAULT_TESTS_DIR "/tmp/tests/"
#endif

static bool create_file(const char *path, int mode) {
    int fd = open(path, mode);
    if (fd != -1) {
        close(fd);
        return true;
    }
    return false;
}

static bool compare_xattr(const char *path, const char *xattr, const char *value) {
    char buf[500] = {0};

    lgetxattr(path, xattr, buf, 500);

    if (!strcmp(buf, value)) {
        return true;
    }

    return false;
}

void test_smack_install(void) {
    security_manager_t *security_manager = NULL;
    CU_ASSERT_EQUAL(security_manager_create(&security_manager, NULL), 0);

    char data_dir[100] = DEFAULT_TESTS_DIR;
    strcat(data_dir, "data/");
    char data_file[100] = DEFAULT_TESTS_DIR;
    strcat(data_file, "data/");
    strcat(data_file, "data_file");

    char exec_dir[100] = DEFAULT_TESTS_DIR;
    strcat(exec_dir, "exec/");
    char exec_file[100] = DEFAULT_TESTS_DIR;
    strcat(exec_file, "exec/");
    strcat(exec_file, "exec_file");

    char id_dir[100] = DEFAULT_TESTS_DIR;
    strcat(id_dir, "id/");
    char id_file[100] = DEFAULT_TESTS_DIR;
    strcat(id_file, "id/");
    strcat(id_file, "id_file");

    char public_dir[100] = DEFAULT_TESTS_DIR;
    strcat(public_dir, "public/");
    char public_file[100] = DEFAULT_TESTS_DIR;
    strcat(public_file, "public/");
    strcat(public_file, "public_file");

    CU_ASSERT_EQUAL(mkdir(DEFAULT_TESTS_DIR, 0770), 0);
    CU_ASSERT_EQUAL(mkdir(data_dir, 0770), 0);
    CU_ASSERT_EQUAL(mkdir(exec_dir, 0770), 0);
    CU_ASSERT_EQUAL(mkdir(id_dir, 0770), 0);
    CU_ASSERT_EQUAL(mkdir(public_dir, 0770), 0);
    CU_ASSERT_EQUAL(create_file(data_file, 0770), true);
    CU_ASSERT_EQUAL(create_file(exec_file, 0770), true);
    CU_ASSERT_EQUAL(create_file(id_file, 0770), true);
    CU_ASSERT_EQUAL(create_file(public_file, 0770), true);

    CU_ASSERT_EQUAL(security_manager_add_path(security_manager, data_dir, "data"), 0);
    CU_ASSERT_EQUAL(security_manager_add_path(security_manager, data_file, "data"), 0);
    CU_ASSERT_EQUAL(security_manager_add_path(security_manager, exec_dir, "exec"), 0);
    CU_ASSERT_EQUAL(security_manager_add_path(security_manager, exec_file, "exec"), 0);
    CU_ASSERT_EQUAL(security_manager_add_path(security_manager, id_dir, "id"), 0);
    CU_ASSERT_EQUAL(security_manager_add_path(security_manager, id_file, "id"), 0);
    CU_ASSERT_EQUAL(security_manager_add_path(security_manager, public_dir, "public"), 0);
    CU_ASSERT_EQUAL(security_manager_add_path(security_manager, public_file, "public"), 0);

    CU_ASSERT_EQUAL(security_manager_add_permission(security_manager, "perm1"), 0);
    CU_ASSERT_EQUAL(security_manager_add_permission(security_manager, "perm2"), 0);

    CU_ASSERT_EQUAL(security_manager_set_id(security_manager, "testid"), 0);

    CU_ASSERT_EQUAL(security_manager_install(security_manager), 0);

    CU_ASSERT_EQUAL(check_file_exists("/etc/smack/accesses.d/app-testid"), true);

    CU_ASSERT_EQUAL(compare_xattr(data_dir, XATTR_NAME_SMACK, "App:testid:Data"), true);
    CU_ASSERT_EQUAL(compare_xattr(data_dir, XATTR_NAME_SMACKTRANSMUTE, "TRUE"), true);

    CU_ASSERT_EQUAL(compare_xattr(data_file, XATTR_NAME_SMACK, "App:testid:Data"), true);

    CU_ASSERT_EQUAL(compare_xattr(exec_dir, XATTR_NAME_SMACK, "App:testid:Exec"), true);

    CU_ASSERT_EQUAL(compare_xattr(exec_file, XATTR_NAME_SMACK, "App:testid:Exec"), true);
    CU_ASSERT_EQUAL(compare_xattr(exec_file, XATTR_NAME_SMACKEXEC, "App:testid"), true);

    CU_ASSERT_EQUAL(compare_xattr(id_dir, XATTR_NAME_SMACK, "App:testid"), true);
    CU_ASSERT_EQUAL(compare_xattr(id_dir, XATTR_NAME_SMACKTRANSMUTE, "TRUE"), true);

    CU_ASSERT_EQUAL(compare_xattr(id_file, XATTR_NAME_SMACK, "App:testid"), true);

    CU_ASSERT_EQUAL(compare_xattr(public_dir, XATTR_NAME_SMACK, "_"), true);
    CU_ASSERT_EQUAL(compare_xattr(public_dir, XATTR_NAME_SMACKTRANSMUTE, "TRUE"), true);

    CU_ASSERT_EQUAL(compare_xattr(public_file, XATTR_NAME_SMACK, "_"), true);

    remove(data_file);
    remove(exec_file);
    remove(id_file);
    remove(public_file);
    rmdir(data_dir);
    rmdir(exec_dir);
    rmdir(id_dir);
    rmdir(public_dir);
    rmdir(DEFAULT_TESTS_DIR);
}