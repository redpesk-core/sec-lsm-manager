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

#include <CUnit/Basic.h>
#include <linux/xattr.h>
#include <sys/xattr.h>

#include "../smack.h"

void test_set_smack(void) {
    char val[200];
    char *path = NULL;
    CU_ASSERT(set_smack(path, XATTR_NAME_SMACK, "value") == -1);
    path = "test.txt";
    CU_ASSERT(set_smack(path, NULL, "value") == -2);
    CU_ASSERT(set_smack(path, XATTR_NAME_SMACK, NULL) == -3);
    FILE *fp;
    fp = fopen(path, "w");
    fclose(fp);
    CU_ASSERT(set_smack(path, XATTR_NAME_SMACK, "value") >= 0);
    CU_ASSERT(lgetxattr(path, XATTR_NAME_SMACK, val, 200) > 0)
    CU_ASSERT(!strcmp(val, "value"));
    remove(path);
}

// int smack_enabled();
void test_generate_app_label(void) {
    char *label = NULL;
    char *id = NULL;
    CU_ASSERT(generate_label(&label, id, "App::", "::Lib") == -1);
    id = "id";
    CU_ASSERT(generate_label(&label, id, "App::", "::Lib") == 0);
    CU_ASSERT(!strcmp(label, "App::id::Lib"));
}

void test_label_file(void) {
    char *path = NULL;
    char *label = NULL;
    CU_ASSERT(label_file(path, label) == -1);
    path = "test.txt";
    CU_ASSERT(label_file(path, label) == -2);
    label = "label";
    CU_ASSERT(label_file(path, label) == -4);
    FILE *fp;
    fp = fopen(path, "w");
    fclose(fp);
    CU_ASSERT(label_file(path, label) == 0);
    remove(path);
}

void test_label_dir_transmute(void) {
    char *path = NULL;
    CU_ASSERT(label_dir_transmute(path) == -1);
    path = "test.txt";

    path = "test";
    mkdir(path, 0700);
    CU_ASSERT(label_dir_transmute(path) == 0);
    rmdir(path);
}

void test_label_exec(void) {
    char *path = NULL;
    char *label = NULL;
    CU_ASSERT(label_exec(path, label) == -1);
    path = "test.sh";
    CU_ASSERT(label_exec(path, label) == -2);
    label = "label";
    CU_ASSERT(label_exec(path, label) == -4);
    FILE *fp;
    fp = fopen(path, "w");
    fclose(fp);
    CU_ASSERT(label_exec(path, label) == 0);
    remove(path);
}

void test_label_path(void) {
    char *path = NULL;
    char *label = NULL;
    int is_executable = 0;
    int is_transmute = 0;

    CU_ASSERT(label_path(path, label, is_executable, is_transmute) == -1);
    path = "test.txt";
    CU_ASSERT(label_path(path, label, is_executable, is_transmute) == -2);
    label = "label";

    FILE *fp;
    fp = fopen(path, "w");
    fclose(fp);
    CU_ASSERT(label_path(path, label, is_executable, is_transmute) == 0);
    remove(path);

    is_executable = 1;

    fp = fopen(path, "w");
    fclose(fp);
    CU_ASSERT(label_path(path, label, is_executable, is_transmute) == 0);
    remove(path);

    is_executable = 0;
    is_transmute = 1;

    mkdir(path, 0700);
    CU_ASSERT(label_path(path, label, is_executable, is_transmute) == 0);
    rmdir(path);
}

// int label_path(const char *path, const char *label, const int is_executable, const int
// is_transmute); int process_path(const Path *path, const char *id);

int main(int argc, char const *argv[]) {
    int ret = 0;
    CU_pSuite pSuite = NULL;

    if (CU_initialize_registry() != CUE_SUCCESS)
        goto error;

    // No pInit pClean
    pSuite = CU_add_suite("test_smack", NULL, NULL);
    if (pSuite == NULL)
        goto error;

    if (CU_add_test(pSuite, "test set smack", test_set_smack) == NULL)
        goto error;

    if (CU_add_test(pSuite, "test generate app label", test_generate_app_label) == NULL)
        goto error;

    if (CU_add_test(pSuite, "test label file", test_label_file) == NULL)
        goto error;

    if (CU_add_test(pSuite, "test label dir transmute", test_label_dir_transmute) == NULL)
        goto error;

    if (CU_add_test(pSuite, "test label exec", test_label_exec) == NULL)
        goto error;

    if (CU_add_test(pSuite, "test label path", test_label_path) == NULL)
        goto error;

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    // Number of failures
    ret = CU_get_number_of_failures();
    goto clean;

error:
    ret = CU_get_error();
clean:
    CU_cleanup_registry();
end:
    return ret;
}
