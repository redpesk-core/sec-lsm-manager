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

#include "test_paths.h"

#include <CUnit/Basic.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../paths.h"

void test_init_paths(void) {
    CU_ASSERT_EQUAL(init_paths(NULL), -EINVAL);
    paths_t paths;
    CU_ASSERT_EQUAL(init_paths(&paths), 0);
    CU_ASSERT_EQUAL(paths.paths, NULL);
    CU_ASSERT_EQUAL(paths.size, 0);
    free_paths(&paths);
}

void test_free_paths(void) {
    paths_t paths;
    CU_ASSERT_EQUAL(init_paths(&paths), 0);
    CU_ASSERT_EQUAL(paths_add_path(&paths, "/test", type_data), 0);
    free_paths(&paths);
    CU_ASSERT_EQUAL(paths.paths, NULL);
    CU_ASSERT_EQUAL(paths.size, 0);
}

void test_paths_add_path(void) {
    paths_t paths;
    CU_ASSERT_EQUAL(init_paths(&paths), 0);
    CU_ASSERT_EQUAL(paths_add_path(&paths, "/test", type_data), 0);

    CU_ASSERT_EQUAL(paths.size, 1);
    CU_ASSERT_STRING_EQUAL(paths.paths[0].path, "/test");
    CU_ASSERT_EQUAL(paths.paths[0].path_type, type_data);
    int i = 0;
    while (i < 50) {
        char buf[50];
        sprintf(buf, "/test/n%d", i);
        CU_ASSERT_EQUAL(paths_add_path(&paths, buf, type_conf), 0);
        i++;
    }
    CU_ASSERT_STRING_EQUAL(paths.paths[0].path, "/test");
    CU_ASSERT_EQUAL(paths.paths[0].path_type, type_data);
    CU_ASSERT_STRING_EQUAL(paths.paths[41].path, "/test/n40");
    CU_ASSERT_EQUAL(paths.paths[41].path_type, type_conf);
    free_paths(&paths);
}

void test_valid_path_type(void) {
    CU_ASSERT_EQUAL(valid_path_type(type_none), false);
    CU_ASSERT_EQUAL(valid_path_type(0), false);
    CU_ASSERT_EQUAL(valid_path_type(type_conf), true);
    CU_ASSERT_EQUAL(valid_path_type(type_data), true);
    CU_ASSERT_EQUAL(valid_path_type(type_exec), true);
    CU_ASSERT_EQUAL(valid_path_type(type_http), true);
    CU_ASSERT_EQUAL(valid_path_type(type_icon), true);
    CU_ASSERT_EQUAL(valid_path_type(type_id), true);
    CU_ASSERT_EQUAL(valid_path_type(type_lib), true);
    CU_ASSERT_EQUAL(valid_path_type(type_public), true);
    CU_ASSERT_EQUAL(valid_path_type(number_path_type), false);
}

void test_get_path_type(void) {
    CU_ASSERT_EQUAL(get_path_type("conf"), type_conf);
    CU_ASSERT_EQUAL(get_path_type("data"), type_data);
    CU_ASSERT_EQUAL(get_path_type("exec"), type_exec);
    CU_ASSERT_EQUAL(get_path_type("http"), type_http);
    CU_ASSERT_EQUAL(get_path_type("icon"), type_icon);
    CU_ASSERT_EQUAL(get_path_type("id"), type_id);
    CU_ASSERT_EQUAL(get_path_type("lib"), type_lib);
    CU_ASSERT_EQUAL(get_path_type("public"), type_public);
}

void test_get_path_type_string(void) {
    CU_ASSERT_STRING_EQUAL(get_path_type_string(type_none), "none");
    CU_ASSERT_STRING_EQUAL(get_path_type_string(type_conf), "conf");
    CU_ASSERT_STRING_EQUAL(get_path_type_string(type_data), "data");
    CU_ASSERT_STRING_EQUAL(get_path_type_string(type_exec), "exec");
    CU_ASSERT_STRING_EQUAL(get_path_type_string(type_http), "http");
    CU_ASSERT_STRING_EQUAL(get_path_type_string(type_icon), "icon");
    CU_ASSERT_STRING_EQUAL(get_path_type_string(type_id), "id");
    CU_ASSERT_STRING_EQUAL(get_path_type_string(type_lib), "lib");
    CU_ASSERT_STRING_EQUAL(get_path_type_string(type_public), "public");
    CU_ASSERT_STRING_EQUAL(get_path_type_string(number_path_type), "invalid");
}