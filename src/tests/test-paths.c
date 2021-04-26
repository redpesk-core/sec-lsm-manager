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

#include <stdio.h>

#include "../paths.c"
#include "setup-tests.h"

START_TEST(test_init_path_set) {
    path_set_t path_set;
    init_path_set(&path_set);
    ck_assert_ptr_eq(path_set.paths, NULL);
    ck_assert_int_eq((int)path_set.size, 0);
    free_path_set(&path_set);
}
END_TEST

START_TEST(test_free_path_set) {
    path_set_t path_set;
    init_path_set(&path_set);
    ck_assert_int_eq(path_set_add_path(&path_set, "/test", type_data), 0);
    free_path_set(&path_set);
    ck_assert_ptr_eq(path_set.paths, NULL);
    ck_assert_int_eq((int)path_set.size, 0);
}
END_TEST

START_TEST(test_path_set_add_path) {
    path_set_t paths;
    init_path_set(&paths);

    ck_assert_int_eq(path_set_add_path(&paths, "/test", 10000), -EINVAL);

    ck_assert_int_eq(path_set_add_path(&paths, "/", type_data), -EINVAL);

    ck_assert_int_eq(path_set_add_path(&paths, "/test", type_data), 0);

    ck_assert_int_eq((int)paths.size, 1);
    ck_assert_str_eq(paths.paths[0]->path, "/test");
    ck_assert_int_eq(paths.paths[0]->path_type, type_data);
    int i = 0;
    while (i < 50) {
        char buf[50];
        snprintf(buf, 50, "/test/n%d", i);
        ck_assert_int_eq(path_set_add_path(&paths, buf, type_conf), 0);
        i++;
    }
    ck_assert_str_eq(paths.paths[0]->path, "/test");
    ck_assert_int_eq((int)paths.paths[0]->path_type, type_data);
    ck_assert_str_eq(paths.paths[41]->path, "/test/n40");
    ck_assert_int_eq((int)paths.paths[41]->path_type, type_conf);
    ck_assert_int_eq(path_set_add_path(&paths, "/test_slash/", type_lib), 0);
    ck_assert_str_eq(paths.paths[51]->path, "/test_slash");
    ck_assert_int_eq((int)paths.paths[51]->path_type, type_lib);

    ck_assert_int_eq(path_set_add_path(&paths, "//", type_data), 0);
    ck_assert_str_eq(paths.paths[52]->path, "/");
    ck_assert_int_eq((int)paths.paths[52]->path_type, type_data);

    free_path_set(&paths);
}
END_TEST

START_TEST(test_valid_path_type) {
    ck_assert_int_eq(valid_path_type(type_none), false);
    ck_assert_int_eq(valid_path_type(0), false);
    ck_assert_int_eq(valid_path_type(type_conf), true);
    ck_assert_int_eq(valid_path_type(type_data), true);
    ck_assert_int_eq(valid_path_type(type_exec), true);
    ck_assert_int_eq(valid_path_type(type_http), true);
    ck_assert_int_eq(valid_path_type(type_icon), true);
    ck_assert_int_eq(valid_path_type(type_id), true);
    ck_assert_int_eq(valid_path_type(type_lib), true);
    ck_assert_int_eq(valid_path_type(type_public), true);
    ck_assert_int_eq(valid_path_type(number_path_type), false);
}
END_TEST

START_TEST(test_get_path_type) {
    ck_assert_int_eq(get_path_type("conf"), type_conf);
    ck_assert_int_eq(get_path_type("data"), type_data);
    ck_assert_int_eq(get_path_type("exec"), type_exec);
    ck_assert_int_eq(get_path_type("http"), type_http);
    ck_assert_int_eq(get_path_type("icon"), type_icon);
    ck_assert_int_eq(get_path_type("id"), type_id);
    ck_assert_int_eq(get_path_type("lib"), type_lib);
    ck_assert_int_eq(get_path_type("public"), type_public);

    ck_assert_int_eq(get_path_type("co"), type_none);
    ck_assert_int_eq(get_path_type("dat"), type_none);
    ck_assert_int_eq(get_path_type("exe"), type_none);
    ck_assert_int_eq(get_path_type("htt"), type_none);
    ck_assert_int_eq(get_path_type("ico"), type_none);
    ck_assert_int_eq(get_path_type("ip"), type_none);
    ck_assert_int_eq(get_path_type("li"), type_none);
    ck_assert_int_eq(get_path_type("pub"), type_none);
    ck_assert_int_eq(get_path_type("zzzz"), type_none);
}
END_TEST

START_TEST(test_get_path_type_string) {
    ck_assert_str_eq(get_path_type_string(type_none), "none");
    ck_assert_str_eq(get_path_type_string(type_conf), "conf");
    ck_assert_str_eq(get_path_type_string(type_data), "data");
    ck_assert_str_eq(get_path_type_string(type_exec), "exec");
    ck_assert_str_eq(get_path_type_string(type_http), "http");
    ck_assert_str_eq(get_path_type_string(type_icon), "icon");
    ck_assert_str_eq(get_path_type_string(type_id), "id");
    ck_assert_str_eq(get_path_type_string(type_lib), "lib");
    ck_assert_str_eq(get_path_type_string(type_public), "public");
    ck_assert_str_eq(get_path_type_string(number_path_type), "invalid");
}
END_TEST

void test_paths() {
    addtest(test_init_path_set);
    addtest(test_free_path_set);
    addtest(test_path_set_add_path);
    addtest(test_valid_path_type);
    addtest(test_get_path_type);
    addtest(test_get_path_type_string);
}
