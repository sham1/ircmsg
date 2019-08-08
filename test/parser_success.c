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
#include <ircmsg/parser.h>
#include <stdio.h>
#include "parser_test.h"

static int
success_setup (void **state)
{
	struct irc_test *test_struct = calloc(1, sizeof(*test_struct));
	if (test_struct == NULL) {
		return -1;
	}
	*state = test_struct;
	return 0;
}

static int
success_teardown (void **state)
{
	struct irc_test *test_struct = *state;
	free_msg(test_struct->msg);
	free(test_struct);
	return 0;
}

static void
test_command (void **state)
{
	struct irc_msg expected = {
		.tags = NULL,
		.prefix = NULL,
		.command = "PRIVMSG",
		.params = NULL,
	};

	struct irc_test *test_struct = *state;
        const char *command_str = "PRIVMSG\r\n";
	size_t consumed = ircmsg_parse((const uint8_t *) command_str,
				       strlen(command_str),
				       &test_cbs,
				       test_struct);
	assert_false(test_struct->failed);
        assert_int_equal(consumed, strlen(command_str));
        assert_true(are_msgs_equal(&expected, test_struct->msg));
}

static void
test_all (void **state)
{
	struct irc_tag tag1 = { .name = "foo", .value = "bar" };
	struct irc_tag tag2 = { .name = "baz", .value = "quux " };
	struct irc_tag *expected_tags_arr[] = {
	        &tag1,
		&tag2,
		NULL,
	};
	struct irc_msg expected = {
		.tags = expected_tags_arr,
		.prefix = "hello\tfoo@bar",
		.command = "PRIVMSG",
		.params = NULL,
	};

	struct irc_test *test_struct = *state;
        const char *command_str = "@foo=bar;baz=quux\\s\\ :hello\tfoo@bar PRIVMSG\r\n";
	size_t consumed = ircmsg_parse((const uint8_t *) command_str,
				       strlen(command_str),
				       &test_cbs,
				       test_struct);
	assert_false(test_struct->failed);
        assert_int_equal(consumed, strlen(command_str));
        assert_true(are_msgs_equal(&expected, test_struct->msg));
}

int
main (int argc, char **argv)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_command,
						success_setup,
						success_teardown),
		cmocka_unit_test_setup_teardown(test_all,
						success_setup,
						success_teardown),
	};

	return cmocka_run_group_tests_name("parse_success_test", tests, NULL, NULL);
}
