/*
 * Copyright (C) 2020-2025 IoT.bzh Company
 * Author: José Bollo <jose.bollo@iot.bzh>
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
#include <unistd.h>

#include "protocol/prot.h"

/* the format is STREAM [ FIELDS...] NULL until a NULL line */
const char *data[] = {
    /* empty records might be allowed */
    "\n",
    NULL,

    /* some arguments */
    "first second third forth fifth\n",
    "first", "second", "third", "forth", "fifth", NULL,

    /* empty arguments */
    " second third  fifth\n",
    "", "second", "third", "", "fifth", NULL,

    /* escaping chars */
    "\\  sec\\ond th\\\nird \\  fi\\ fth\\\\\n",
    " ", "sec\\ond", "th\nird", " ", "fi fth\\", NULL,

    /* end */
    NULL
};

static int is_reset_state(prot_t *prot)
{
    return prot_should_write(prot) == 0
        && prot_can_read(prot) != 0
        && prot_get(prot, NULL) == -EAGAIN;
}

static void put_all(prot_t *prot, const char **fields, int count) {
    int sts;

    // emits all and end
    sts = prot_put(prot, (unsigned)count, fields);
    ck_assert_int_eq(0, sts);
}

static void put_all_fields(prot_t *prot, const char **fields, int count) {
    int sts;

    // emits all fields
    sts = prot_put_fields(prot, (unsigned)count, fields);
    ck_assert_int_eq(0, sts);

    // end
    sts = prot_put_end(prot);
    ck_assert_int_eq(0, sts);
}

static void put_field_by_field(prot_t *prot, const char **fields, int count) {
    int i, sts;

    // emits field by field
    for (i = 0 ; i < count ; i++) {
        sts = prot_put_field(prot, fields[i]);
        ck_assert_int_eq(0, sts);
    }

    // end
    sts = prot_put_end(prot);
    ck_assert_int_eq(0, sts);
}

static void test_writing(int allow_empty, void (*put)(prot_t *prot, const char **fields, int count)) {

    char buffer[1000];
    int i, n, sts, nrd, fds[2];
    prot_t *prot = NULL;

    // create the prot object
    prot_create(&prot);
    ck_assert_ptr_ne(NULL, prot);

    // set and check allow_empty
    ck_assert_int_eq(0, prot_is_empty_allowed(prot));
    prot_set_allow_empty(prot, allow_empty);
    ck_assert_int_eq(!!allow_empty, prot_is_empty_allowed(prot));

    // creates the pipe
    sts = pipe(fds);
    ck_assert_int_eq(0, sts);

    // iterate over samples
    for (i = 0; data[i] != NULL ; i = i + n + 2) {

        // check it is clean
        sts = prot_should_write(prot);
        ck_assert_int_eq(0, sts);

        // emits fields
        for (n = 0 ; data[i + n + 1] != NULL ; n++);
        put(prot, &data[i + 1], n);

        // check there is something to write
        sts = prot_should_write(prot);
        if (n == 0 && !allow_empty) {
            ck_assert_int_eq(0, sts);
            continue;
        }
        ck_assert_int_eq(1, sts);

        // write on the pipe
        sts = prot_write(prot, fds[1]);
        ck_assert_int_le(0, sts);
        ck_assert_int_eq(sts, (int)strlen(data[i]));

        // read the pipe
        nrd = (int)read(fds[0], buffer, sizeof buffer);
        ck_assert_int_le(0, nrd);
        ck_assert_int_eq(nrd, (int)strlen(data[i]));
        ck_assert_int_eq(0, memcmp(buffer, data[i], (unsigned)nrd));
    }
    // cleaning
    close(fds[0]);
    close(fds[1]);
    prot_destroy(prot);
}


static void test_reading(int allow_empty) {

    const char **fields;
    int i, j, n, sts, fds[2];
    prot_t *prot = NULL;

    // create the prot object
    prot_create(&prot);
    ck_assert_ptr_ne(NULL, prot);

    // set and check allow_empty
    ck_assert_int_eq(0, prot_is_empty_allowed(prot));
    prot_set_allow_empty(prot, allow_empty);
    ck_assert_int_eq(!!allow_empty, prot_is_empty_allowed(prot));

    // creates the pipe
    sts = pipe(fds);
    ck_assert_int_eq(0, sts);

    // iterate over samples
    for (i = 0; data[i] != NULL ; i = i + n + 2) {

        // write the pipe
        n = (int)strlen(data[i]);
        sts = (int)write(fds[1], data[i], (unsigned)n);
        ck_assert_int_eq(sts, n);

        // read the buffer
        sts = prot_read(prot, fds[0]);
        ck_assert_int_eq(sts, n);

        // receive fields
        n = prot_get(prot, &fields);
        if (!allow_empty && !strcmp(data[i], "\n")) {
            ck_assert_int_le(-EAGAIN, n);
            n = 0;
        }
        ck_assert_int_le(0, n);

        // check each field
        for (j = 0 ; j < n ; j++)
            ck_assert_str_eq(data[i + j + 1], fields[j]);

        // check count
        ck_assert_ptr_eq(NULL, data[i + n + 1]);

        // check it remains
        if (n > 0 || allow_empty) {
            j = prot_get(prot, NULL);
            ck_assert_int_eq(j, n);
        }

        // check it is cleaned
        prot_next(prot);
        j = prot_get(prot, NULL);
        ck_assert_int_eq(-EAGAIN, j);
    }
    // cleaning
    close(fds[0]);
    close(fds[1]);
    prot_destroy(prot);
}

static void test_write_read(int allow_empty, int urge) {

    const char **fields;
    int i, j, n, sts, fds[2];
    prot_t *prot = NULL;

    // create the prot object
    prot_create(&prot);
    ck_assert_ptr_ne(NULL, prot);

    // set and check allow_empty
    ck_assert_int_eq(0, prot_is_empty_allowed(prot));
    prot_set_allow_empty(prot, allow_empty);
    ck_assert_int_eq(!!allow_empty, prot_is_empty_allowed(prot));

    // creates the pipe
    sts = pipe(fds);
    ck_assert_int_eq(0, sts);

    // send all
    for (i = 0; data[i] != NULL ; i = i + n + 2) {

        // emits field by field
        for (n = 0 ; data[i + n + 1] != NULL ; n++);
        sts = prot_put(prot, (unsigned)n, &data[i + 1]);
        ck_assert_int_eq(0, sts);

        // write if required
        if (urge && prot_should_write(prot)) {
            sts = prot_write(prot, fds[1]);
            ck_assert_int_le(0, sts);
            ck_assert_int_eq(sts, (int)strlen(data[i]));
        }
    }

    // write all if required
    while (prot_should_write(prot)) {
        sts = prot_write(prot, fds[1]);
        ck_assert_int_le(0, sts);
    }

    // receive all
    for (i = 0; data[i] != NULL ; i = i + n + 2) {

        // skip empty line if not received nor transmitted
        if (data[i + 1] == NULL && !allow_empty) {
            n = 0;
            continue;
        }

        // read the record
        n = prot_get(prot, &fields);
        if (n == -EAGAIN) {
            // read the buffer
            sts = prot_read(prot, fds[0]);
            ck_assert_int_gt(sts, 0);
            n = prot_get(prot, &fields);
        }
        ck_assert_int_ge(n, 0);

        // check each field
        for (j = 0 ; j < n ; j++)
            ck_assert_str_eq(data[i + j + 1], fields[j]);

        // check count
        ck_assert_ptr_eq(NULL, data[i + n + 1]);

        // check it remains
        if (n > 0 || allow_empty) {
            j = prot_get(prot, NULL);
            ck_assert_int_eq(j, n);
        }

        // check it is cleaned
        prot_next(prot);
    }

    // cleaning
    close(fds[0]);
    close(fds[1]);
    prot_destroy(prot);
}


START_TEST(test_prot_create) {
    prot_t *prot = NULL;
    prot_create(&prot);
    ck_assert_ptr_ne(NULL, prot);
    ck_assert_int_eq(1, is_reset_state(prot));
    prot_destroy(prot);
}
END_TEST

START_TEST(test_prot_put_field_by_field) {
    test_writing(0, put_field_by_field);
    test_writing(1, put_field_by_field);
}
END_TEST

START_TEST(test_prot_put_all_fields) {
    test_writing(0, put_all_fields);
    test_writing(1, put_all_fields);
}
END_TEST

START_TEST(test_prot_put_all) {
    test_writing(0, put_all);
    test_writing(1, put_all);
}
END_TEST

START_TEST(test_prot_read) {
    test_reading(0);
    test_reading(1);
}
END_TEST

START_TEST(test_prot_write_read) {
    test_write_read(0, 0);
    test_write_read(0, 1);
    test_write_read(1, 0);
    test_write_read(1, 1);
}
END_TEST

void test_prot(void) {
    addtest(test_prot_create);
    addtest(test_prot_put_field_by_field);
    addtest(test_prot_put_all_fields);
    addtest(test_prot_put_all);
    addtest(test_prot_read);
    addtest(test_prot_write_read);
}






