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

#include "context/plugs.h"

START_TEST(test_plugset_init)
{
    plugset_t plugset;

    plugset = (plug_t*)(intptr_t)8;
    plugset_init(&plugset);
    ck_assert_ptr_null(plugset);

    plugset_clear(&plugset);
    ck_assert_ptr_null(plugset);
}
END_TEST

START_TEST(test_plugset_deinit)
{
    plugset_t plugset;

    plugset = (plug_t*)(intptr_t)8;
    plugset_init(&plugset);
    ck_assert_ptr_null(plugset);

    ck_assert_int_eq(plugset_add(&plugset, "/tmp/a", "xb", "/tmp/c"), 0);
    ck_assert_ptr_nonnull(plugset);

    ck_assert_int_eq(plugset_add(&plugset, "/tmp/u", "xv", "/tmp/w"), 0);
    ck_assert_ptr_nonnull(plugset);

    plugset_clear(&plugset);
    ck_assert_ptr_null(plugset);
}
END_TEST

START_TEST(test_plugset_add) {
    plugset_t plugset;

    plugset = (plug_t*)(intptr_t)8;
    plugset_init(&plugset);
    ck_assert_ptr_null(plugset);

    ck_assert_int_eq(plugset_add(&plugset, "/tmp/a", "xb", "/tmp/c"), 0);
    ck_assert_ptr_nonnull(plugset);
    ck_assert_str_eq("/tmp/a", plugset->expdir);
    ck_assert_str_eq("xb", plugset->impid);
    ck_assert_str_eq("/tmp/c", plugset->impdir);

    ck_assert_int_eq(plugset_add(&plugset, "/tmp/u", "xv", "/tmp/w"), 0);
    ck_assert_ptr_nonnull(plugset);
    ck_assert_str_eq("/tmp/u", plugset->expdir);
    ck_assert_str_eq("xv", plugset->impid);
    ck_assert_str_eq("/tmp/w", plugset->impdir);
    ck_assert_str_eq("/tmp/a", plugset->next->expdir);
    ck_assert_str_eq("xb", plugset->next->impid);
    ck_assert_str_eq("/tmp/c", plugset->next->impdir);

    plugset_clear(&plugset);
    ck_assert_ptr_null(plugset);
}
END_TEST


void test_plugs(void) {
    addtest(test_plugset_init);
    addtest(test_plugset_deinit);
    addtest(test_plugset_add);
}
