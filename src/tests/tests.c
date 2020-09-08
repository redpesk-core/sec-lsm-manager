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

#include "tests.h"

#include "test_paths.h"
#include "test_permissions.h"
#include "test_secure_app.h"
#include "test_smack.h"
#include "test_utils.h"

Suite *suite;
TCase *tcase;

void mksuite(const char *name) { suite = suite_create(name); }

void addtcase(const char *name) {
    tcase = tcase_create(name);
    suite_add_tcase(suite, tcase);
}

void addtest(TFun fun) { tcase_add_test(tcase, fun); }

int srun() {
    int nerr;
    SRunner *srunner = srunner_create(suite);
    srunner_run_all(srunner, CK_NORMAL);
    nerr = srunner_ntests_failed(srunner);
    srunner_free(srunner);
    return nerr;
}

int main() {
    mksuite("tests");
    addtcase("paths");
    tests_paths();
    addtcase("permissions");
    tests_permissions();
    addtcase("secure_app");
    tests_secure_app();
    addtcase("utils");
    tests_utils();

#if WITH_SMACK
    addtcase("smack");
    tests_smack();
#endif
    return !!srun();
}
