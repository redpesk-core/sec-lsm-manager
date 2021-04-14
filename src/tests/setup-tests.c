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

#include "setup-tests.h"

#include "../log.c"
#include "../mustach/mustach.c"
#include "../template.c"

Suite *suite;
TCase *tcase;

void mksuite(const char *name) { suite = suite_create(name); }

void addtcase(const char *name) {
    tcase = tcase_create(name);
    tcase_set_timeout(tcase, 30);
    suite_add_tcase(suite, tcase);
}

void addtest(const TTest *fun) { tcase_add_test(tcase, fun); }

int srun() {
    int nerr;
    SRunner *srunner = srunner_create(suite);
    srunner_run_all(srunner, CK_NORMAL);
    nerr = srunner_ntests_failed(srunner);
    srunner_free(srunner);
    return nerr;
}

void create_file(const char *path) {
    int fd = creat(path, S_IRWXU | S_IRWXG);
    ck_assert_int_ne(fd, -1);
    close(fd);
}

void create_tmp_dir(char *tmp_dir) {
    secure_strncpy(tmp_dir, "/tmp/sec-lsm-XXXXXX", 20);
    ck_assert_ptr_ne(mkdtemp(tmp_dir), NULL);
}

void create_tmp_file(char *tmp_file) {
    secure_strncpy(tmp_file, "/tmp/sec-lsm-XXXXXX", 20);
    int fd = mkstemp(tmp_file);
    ck_assert_int_gt(fd, 0);
    close(fd);
}

void create_etc_tmp_file(char *tmp_file) {
    secure_strncpy(tmp_file, "/etc/sec-lsm-XXXXXX", 20);
    int fd = mkstemp(tmp_file);
    ck_assert_int_gt(fd, 0);
    close(fd);
}

bool compare_xattr(const char *path, const char *xattr, const char *value) {
    char buf[500] = {0};

    lgetxattr(path, xattr, buf, 500);

    if (!strcmp(buf, value)) {
        return true;
    }

    return false;
}

extern void test_paths();
extern void test_permissions();
extern void test_secure_app();
extern void test_utils();

#if !defined(SIMULATE_CYNAGORA)
extern void test_cynagora();
#endif

#if defined(WITH_SMACK)
extern void test_smack();
extern void test_smack_label();
#endif

#if defined(WITH_SELINUX)
extern void test_selinux_template();
extern void test_selinux();
#endif

int main() {
    mksuite("tests");

    addtcase("paths");
    test_paths();

    addtcase("permissions");
    test_permissions();

    addtcase("secure_app");
    test_secure_app();

    addtcase("utils");
    test_utils();

#if !defined(SIMULATE_CYNAGORA)
    addtcase("cynagora");
    test_cynagora();
#endif

#if defined(WITH_SMACK)
    addtcase("smack");
    test_smack();
    test_smack_label();
#endif

#if defined(WITH_SELINUX)
    addtcase("selinux");
    test_selinux_template();
    test_selinux();
#endif

    return !!srun();
}