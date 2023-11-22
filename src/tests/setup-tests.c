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
#include <stdlib.h>
#include <sys/xattr.h>
#include <unistd.h>

#include "file-utils.h"
#include "sizes.h"

Suite *suite;
TCase *tcase;

void mksuite(const char *name) { suite = suite_create(name); }

void addtcase(const char *name) {
    tcase = tcase_create(name);
    tcase_set_timeout(tcase, 30);
    suite_add_tcase(suite, tcase);
}

#if (CHECK_MAJOR_VERSION <= 0) && (CHECK_MINOR_VERSION < 13)
void addtest(TFun *fun) { tcase_add_test(tcase, fun); }
#else
void addtest(const TTest *fun) { tcase_add_test(tcase, fun); }
#endif

int srun(const char *tapfile) {
    int nerr;
    SRunner *srunner = srunner_create(suite);
    srunner_set_tap(srunner, tapfile);
    srunner_run_all(srunner, CK_NORMAL);
    nerr = srunner_ntests_failed(srunner);
    srunner_free(srunner);
    return nerr;
}

void create_tmp_dir(char tmp_dir[TMP_MIN_LENGTH]) {
    strcpy(tmp_dir, "/tmp/sec-lsm-XXXXXX");
    ck_assert_ptr_ne(mkdtemp(tmp_dir), NULL);
}

void create_tmp_file(char tmp_file[TMP_MIN_LENGTH]) {
    strcpy(tmp_file, "/tmp/sec-lsm-XXXXXX");
    int fd = mkstemp(tmp_file);
    ck_assert_int_gt(fd, 0);
    close(fd);
}

void create_etc_tmp_file(char tmp_file[TMP_MIN_LENGTH]) {
    strcpy(tmp_file, "/etc/sec-lsm-XXXXXX");
    int fd = mkstemp(tmp_file);
    ck_assert_int_gt(fd, 0);
    close(fd);
}

bool compare_xattr(const char *path, const char *xattr, const char *value) {
    char buf[SEC_LSM_MANAGER_MAX_SIZE_LABEL] = {0};

    lgetxattr(path, xattr, buf, SEC_LSM_MANAGER_MAX_SIZE_LABEL);

    if (!strcmp(buf, value)) {
        return true;
    }

    return false;
}

int main(int argc, char const *argv[]) {
    const char *tapfile = argc > 1 ? argv[1] : "/dev/stdout";

    mksuite("tests");

    addtcase("prot");
    test_prot();

    addtcase("paths");
    test_paths();

    addtcase("permissions");
    test_permissions();

    addtcase("context");
    test_context();

    addtcase("utils");
    test_utils();

    addtcase("plug");
    test_plugs();

#if !SIMULATE_CYNAGORA
    addtcase("cynagora");
    test_cynagora();
#endif

#if WITH_SMACK && !SIMULATE_SMACK
    addtcase("smack");
    test_smack();
    test_smack_label();
#endif

#if WITH_SELINUX && !SIMULATE_SELINUX
    addtcase("selinux");
    test_selinux_template();
    test_selinux();
#endif

    return !!srun(tapfile);
}
