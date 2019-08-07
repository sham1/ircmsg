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

struct failure_test_struct
{
	bool failed;
	ircmsg_parser_err_code code;
};

static void
failure_test_on_error(ircmsg_parser_err_code error, void *user_data)
{
	struct failure_test_struct *test_struct = user_data;
	test_struct->failed = true;
	test_struct->code = error;
}

static ircmsg_parser_callbacks cbs = {
	.start_message = NULL,
	.start_tags = NULL,
	.on_tag = NULL,
	.end_tags = NULL,
	.on_prefix = NULL,
	.on_command = NULL,
	.start_params = NULL,
	.on_param = NULL,
	.end_params = NULL,
	.end_message = NULL,

	.on_error = failure_test_on_error,
};

static int
failure_setup (void **state)
{
	struct failure_test_struct *test_struct = calloc(1, sizeof(*test_struct));
	if (test_struct == NULL) {
		return -1;
	}
	*state = test_struct;
	return 0;
}

static int
failure_teardown (void **state)
{
	free(*state);
	return 0;
}

static void
test_empty (void **state)
{
	struct failure_test_struct *test_struct = *state;
        const char *empty_str = "";
	size_t consumed = ircmsg_parse((const uint8_t *) empty_str,
				       strlen(empty_str),
				       &cbs,
				       test_struct);
        assert_int_equal(consumed, 0);
	assert_true(test_struct->failed);
	assert_int_equal(test_struct->code, IRCMSG_ERR_PARSER_UNEXPECTED_END_OF_MESSAGE);
}

static void
test_linefeed (void **state)
{
	struct failure_test_struct *test_struct = *state;
        const char *linefeed_str = "\r\n";
	size_t consumed = ircmsg_parse((const uint8_t *) linefeed_str,
				       strlen(linefeed_str),
				       &cbs,
				       test_struct);
        assert_int_equal(consumed, 0);
	assert_true(test_struct->failed);
	assert_int_equal(test_struct->code, IRCMSG_ERR_PARSER_MESSAGE_NOT_FOUND);
}

static void
test_whitespace (void **state)
{
	struct failure_test_struct *test_struct = *state;
        const char *whitespace_str = "          \r\n";
	size_t consumed = ircmsg_parse((const uint8_t *) whitespace_str,
				       strlen(whitespace_str),
				       &cbs,
				       test_struct);
        assert_int_equal(consumed, 0);
	assert_true(test_struct->failed);
	assert_int_equal(test_struct->code, IRCMSG_ERR_PARSER_MESSAGE_NOT_FOUND);
}

int
main (int argc, char **argv)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_empty,
						failure_setup,
						failure_teardown),
		cmocka_unit_test_setup_teardown(test_linefeed,
						failure_setup,
						failure_teardown),
		cmocka_unit_test_setup_teardown(test_whitespace,
						failure_setup,
						failure_teardown),
	};

	return cmocka_run_group_tests_name("parse_fail_test", tests, NULL, NULL);
}
