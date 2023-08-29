/*
 * Copyright (C) 2020-2023 IoT.bzh Company
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

#include "plugs.h"

START_TEST(test_plugset_init)
{
    plugset_t plugset;

    plugset = (plug_t*)&plugset;
    plugset_init(&plugset);
    ck_assert_ptr_null(plugset);

    plugset_deinit(&plugset);
    ck_assert_ptr_null(plugset);
}
END_TEST

START_TEST(test_plugset_deinit)
{
    plugset_t plugset;

    plugset = (plug_t*)&plugset;
    plugset_init(&plugset);
    ck_assert_ptr_null(plugset);

    ck_assert_int_eq(plugset_add(&plugset, "a", "b", "c"), 0);
    ck_assert_ptr_nonnull(plugset);

    ck_assert_int_eq(plugset_add(&plugset, "u", "v", "w"), 0);
    ck_assert_ptr_nonnull(plugset);

    plugset_deinit(&plugset);
    ck_assert_ptr_null(plugset);
}
END_TEST

START_TEST(test_plugset_add) {
    plugset_t plugset;

    plugset = (plug_t*)&plugset;
    plugset_init(&plugset);
    ck_assert_ptr_null(plugset);

    ck_assert_int_eq(plugset_add(&plugset, "a", "b", "c"), 0);
    ck_assert_ptr_nonnull(plugset);
    ck_assert_str_eq("a", plugset->expdir);
    ck_assert_str_eq("b", plugset->impid);
    ck_assert_str_eq("c", plugset->impdir);

    ck_assert_int_eq(plugset_add(&plugset, "u", "v", "w"), 0);
    ck_assert_ptr_nonnull(plugset);
    ck_assert_str_eq("u", plugset->expdir);
    ck_assert_str_eq("v", plugset->impid);
    ck_assert_str_eq("w", plugset->impdir);
    ck_assert_str_eq("a", plugset->next->expdir);
    ck_assert_str_eq("b", plugset->next->impid);
    ck_assert_str_eq("c", plugset->next->impdir);

    plugset_deinit(&plugset);
    ck_assert_ptr_null(plugset);
}
END_TEST


void test_plugs(void) {
    addtest(test_plugset_init);
    addtest(test_plugset_deinit);
    addtest(test_plugset_add);
}
