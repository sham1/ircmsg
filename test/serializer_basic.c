// Copyright (c) 2019 Jani Juhani Sinervo
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <stdbool.h>
#include <ircmsg/serializer.h>
#include <stdio.h>
#include "serializer_test.h"

static int
serializer_basic_setup (void **state)
{
	return 0;
}

static int
serializer_basic_teardown (void **state)
{
	return 0;
}

static void
test_simple (void **state)
{
	char *params[] = {
		"#test",
		"This is the message",
		NULL,
	};
	struct irc_msg msg = {
		.tags = NULL,
		.prefix = NULL,
		.command = "PRIVMSG",
		.params = params,
	};

	const char *expected = "PRIVMSG #test :This is the message\r\n";
	size_t expected_length = strlen(expected);
	size_t serialized_length =
		ircmsg_serialize_buffer_len(&serializer_test_cbs,
					    &msg);

	assert_int_equal(expected_length, serialized_length);

	uint8_t *serialize_buf = calloc(serialized_length + 1,
					sizeof(*serialize_buf));

	ircmsg_serialize(serialize_buf, serialized_length,
			 &serializer_test_cbs,
			 &msg);

	assert_string_equal(expected, serialize_buf);

	free(serialize_buf);
}

static void
test_all (void **state)
{
	struct irc_tag tag1 = {
		.name = "foo",
		.value = "bar  ",
	};
	struct irc_tag *tags[] = {
		&tag1,
		NULL,
	};
	char *params[] = {
		"#test",
		"This is the message",
		NULL,
	};
	struct irc_msg msg = {
		.tags = tags,
		.prefix = "test!test@example.org",
		.command = "PRIVMSG",
		.params = params,
	};

	const char *expected = "@foo=bar\\s\\s :test!test@example.org PRIVMSG #test :This is the message\r\n";
	size_t expected_length = strlen(expected);
	size_t serialized_length =
		ircmsg_serialize_buffer_len(&serializer_test_cbs,
					    &msg);

	assert_int_equal(expected_length, serialized_length);

	uint8_t *serialize_buf = calloc(serialized_length + 1,
					sizeof(*serialize_buf));

	ircmsg_serialize(serialize_buf, serialized_length,
			 &serializer_test_cbs,
			 &msg);

	assert_string_equal(expected, serialize_buf);

	free(serialize_buf);
}

int
main (int argc, char **argv)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_simple,
						serializer_basic_setup,
						serializer_basic_teardown),
		cmocka_unit_test_setup_teardown(test_all,
						serializer_basic_setup,
						serializer_basic_teardown),
	};

	return cmocka_run_group_tests_name("serialize_basic_test", tests, NULL, NULL);
}
