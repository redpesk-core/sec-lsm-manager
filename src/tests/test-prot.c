/*
 * Copyright (C) 2020-2023 IoT.bzh Company
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

#include "prot.h"

/* the format is LINE [ FIELDS...] NULL until a NULL line */
const char *data[] = {
	/* empty lines are allowed */
	"\n",
	NULL,

	/* some arguments */
	"first second third forth fifth\n",
	"first", "second", "third", "forth", "fifth", NULL,

	/* empty arguments */
	" second third  fifth\n",
	"", "second", "third", "", "fifth", NULL,

	/* escaping chars */
	"\\  sec\\\\ond th\\\nird \\  fi\\ fth\n",
	" ", "sec\\ond", "th\nird", " ", "fi fth", NULL,

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

static void test_writing(void (*put)(prot_t *prot, const char **fields, int count)) {

    char buffer[1000];
    int i, n, sts, nrd, fds[2];
    prot_t *prot = NULL;

    // create the prot object
    prot_create(&prot);
    ck_assert_ptr_ne(NULL, prot);

    // creates the pipe
    sts = pipe(fds);
    ck_assert_int_eq(0, sts);

    // iterate over samples
    for (i = 0; data[i] != NULL ; i = i + n + 2) {

        // check it is clean
        sts = prot_should_write(prot);
        ck_assert_int_eq(0, sts);

        // emits field by field
	for (n = 0 ; data[i + n + 1] != NULL ; n++);
	put(prot, &data[i + 1], n);

        // check there is something to write
        sts = prot_should_write(prot);
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


START_TEST(test_prot_create) {
    prot_t *prot = NULL;
    prot_create(&prot);
    ck_assert_ptr_ne(NULL, prot);
    ck_assert_int_eq(1, is_reset_state(prot));
    prot_destroy(prot);
}
END_TEST

START_TEST(test_prot_put_field_by_field) {
    test_writing(put_field_by_field);
}
END_TEST

START_TEST(test_prot_put_all_fields) {
    test_writing(put_all_fields);
}
END_TEST

START_TEST(test_prot_put_all) {
    test_writing(put_all);
}
END_TEST

void test_prot(void) {
    addtest(test_prot_create);
    addtest(test_prot_put_field_by_field);
    addtest(test_prot_put_all_fields);
    addtest(test_prot_put_all);
}
