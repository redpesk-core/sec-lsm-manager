/*
 * Copyright (C) 2020-2025 IoT.bzh Company
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

#include "setup-tests.h"

#include <stdio.h>
#include <errno.h>

#include "context/paths.h"

START_TEST(test_init_path_set) {
    path_set_t path_set;
    path_set_init(&path_set);
    ck_assert_ptr_eq(path_set.paths, NULL);
    ck_assert_int_eq((int)path_set.size, 0);
    path_set_clear(&path_set);
}
END_TEST

START_TEST(test_free_path_set) {
    path_set_t path_set;
    path_set_init(&path_set);
    ck_assert_int_eq(path_set_add(&path_set, "/test", type_data), 0);
    path_set_clear(&path_set);
    ck_assert_ptr_eq(path_set.paths, NULL);
    ck_assert_int_eq((int)path_set.size, 0);
}
END_TEST

START_TEST(test_path_set_add_path) {
    path_set_t paths;
    path_set_init(&paths);

    ck_assert_int_eq(path_set_add(&paths, "/test", 10000), -EINVAL);

    ck_assert_int_eq(path_set_add(&paths, "", type_data), -EINVAL);

    ck_assert_int_eq(path_set_add(&paths, "/test", type_data), 0);

    ck_assert_int_eq((int)paths.size, 1);
    ck_assert_str_eq(paths.paths[0]->path, "/test");
    ck_assert_int_eq(paths.paths[0]->path_type, type_data);
    int i = 0;
    while (i < 50) {
        char buf[50];
        snprintf(buf, 50, "/test/n%d", i);
        ck_assert_int_eq(path_set_add(&paths, buf, type_conf), 0);
        i++;
    }
    ck_assert_str_eq(paths.paths[0]->path, "/test");
    ck_assert_int_eq((int)paths.paths[0]->path_type, type_data);
    ck_assert_str_eq(paths.paths[41]->path, "/test/n40");
    ck_assert_int_eq((int)paths.paths[41]->path_type, type_conf);
    ck_assert_int_eq(path_set_add(&paths, "/test_slash/", type_lib), 0);
    ck_assert_str_eq(paths.paths[51]->path, "/test_slash/");
    ck_assert_int_eq((int)paths.paths[51]->path_type, type_lib);

    ck_assert_int_eq(path_set_add(&paths, "//", type_data), 0);
    /* IREV2: path normalization is in the context, so path_set_add doesn't normalize
              Is this test still relevant? */
    ck_assert_str_eq(paths.paths[52]->path, "//");
    ck_assert_int_eq((int)paths.paths[52]->path_type, type_data);

    path_set_clear(&paths);
}
END_TEST

START_TEST(test_valid_path_type) {
    ck_assert_int_eq(path_type_is_valid(type_unset), false);
    ck_assert_int_eq(path_type_is_valid(0), false);
    ck_assert_int_eq(path_type_is_valid(type_default), true);
    ck_assert_int_eq(path_type_is_valid(type_conf), true);
    ck_assert_int_eq(path_type_is_valid(type_data), true);
    ck_assert_int_eq(path_type_is_valid(type_exec), true);
    ck_assert_int_eq(path_type_is_valid(type_http), true);
    ck_assert_int_eq(path_type_is_valid(type_icon), true);
    ck_assert_int_eq(path_type_is_valid(type_id), true);
    ck_assert_int_eq(path_type_is_valid(type_lib), true);
    ck_assert_int_eq(path_type_is_valid(type_public), true);
    ck_assert_int_eq(path_type_is_valid(number_path_type), false);
}
END_TEST

START_TEST(test_get_path_type) {
    ck_assert_int_eq(path_type_get("default"), type_default);
    ck_assert_int_eq(path_type_get("conf"), type_conf);
    ck_assert_int_eq(path_type_get("data"), type_data);
    ck_assert_int_eq(path_type_get("exec"), type_exec);
    ck_assert_int_eq(path_type_get("http"), type_http);
    ck_assert_int_eq(path_type_get("icon"), type_icon);
    ck_assert_int_eq(path_type_get("id"), type_id);
    ck_assert_int_eq(path_type_get("lib"), type_lib);
    ck_assert_int_eq(path_type_get("public"), type_public);

    ck_assert_int_eq(path_type_get("no"), type_unset);
    ck_assert_int_eq(path_type_get("co"), type_unset);
    ck_assert_int_eq(path_type_get("dat"), type_unset);
    ck_assert_int_eq(path_type_get("exe"), type_unset);
    ck_assert_int_eq(path_type_get("htt"), type_unset);
    ck_assert_int_eq(path_type_get("ico"), type_unset);
    ck_assert_int_eq(path_type_get("ip"), type_unset);
    ck_assert_int_eq(path_type_get("li"), type_unset);
    ck_assert_int_eq(path_type_get("pub"), type_unset);
    ck_assert_int_eq(path_type_get("zzzz"), type_unset);
}
END_TEST

START_TEST(test_get_path_type_string) {
    ck_assert_str_eq(path_type_name(type_unset), "<unset>");
    ck_assert_str_eq(path_type_name(type_default), "default");
    ck_assert_str_eq(path_type_name(type_conf), "conf");
    ck_assert_str_eq(path_type_name(type_data), "data");
    ck_assert_str_eq(path_type_name(type_exec), "exec");
    ck_assert_str_eq(path_type_name(type_http), "http");
    ck_assert_str_eq(path_type_name(type_icon), "icon");
    ck_assert_str_eq(path_type_name(type_id), "id");
    ck_assert_str_eq(path_type_name(type_lib), "lib");
    ck_assert_str_eq(path_type_name(type_public), "public");
    ck_assert_str_eq(path_type_name(number_path_type), "invalid");
}
END_TEST

void test_paths(void) {
    addtest(test_init_path_set);
    addtest(test_free_path_set);
    addtest(test_path_set_add_path);
    addtest(test_valid_path_type);
    addtest(test_get_path_type);
    addtest(test_get_path_type_string);
}
