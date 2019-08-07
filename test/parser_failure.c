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

struct failure_irc_tag
{
	char *name;
	char *value;
};

static struct failure_irc_tag **
reallocate_tags_arr(struct failure_irc_tag **arr, size_t count)
{
	struct failure_irc_tag **ret = realloc(arr, (count + 1) * sizeof(*arr));
	ret[count] = NULL;
	return ret;
}

static size_t
count_tags(struct failure_irc_tag **arr)
{
        size_t ret = 0;
	for (struct failure_irc_tag **iter = arr; *iter; ++iter) {
		++ret;
	}
	return ret;
}

static void
free_tags_arr(struct failure_irc_tag **arr)
{
	if (arr == NULL) return;
	for (struct failure_irc_tag **iter = arr; *iter; ++iter) {
		free((*iter)->name);
		free((*iter)->value);
		free(*iter);
	}
}

struct failure_irc_message
{
	struct failure_irc_tag **tags;
	char *prefix;
	char *command;
	char **params;
};

struct failure_test_struct
{
	struct failure_irc_message *msg;
	bool failed;
	ircmsg_parser_err_code code;
};

static void
failure_test_on_error(ircmsg_parser_err_code error, void *user_data)
{
	struct failure_test_struct *test_struct = user_data;
	test_struct->failed = true;
	test_struct->code = error;

	if (test_struct->msg != NULL) {
		free_tags_arr(test_struct->msg->tags);
		free(test_struct->msg);
	}
}

static void
failure_test_start_message(void *user_data)
{
	struct failure_test_struct *test_struct = user_data;
	// If this fails in this test, something is seriously wrong.
	// This is not recoverable.
	test_struct->msg = calloc(1, sizeof(*test_struct->msg));
}

static void
failure_test_end_message(void *user_data)
{
	struct failure_test_struct *test_struct = user_data;
	free(test_struct->msg);
}

static void
failure_test_start_tags(void *user_data)
{
	struct failure_test_struct *test_struct = user_data;
        struct failure_irc_message *msg = test_struct->msg;
	msg->tags = reallocate_tags_arr(NULL, 0);
}

static char *
allocate_len(const uint8_t *str, size_t len)
{
	if (str == NULL) return NULL;
        char *ret = calloc(len + 1, sizeof(*ret));
	memcpy(ret, str, len);
	return ret;
}

static void
failure_test_on_tag(const uint8_t *name, size_t name_len,
		    const uint8_t *esc_value, size_t esc_value_len,
		    void *user_data)
{
	struct failure_test_struct *test_struct = user_data;
        struct failure_irc_message *msg = test_struct->msg;
	size_t old_len = count_tags(msg->tags);

	msg->tags = reallocate_tags_arr(msg->tags, old_len + 1);
	msg->tags[old_len] = calloc(1, sizeof(*msg->tags[old_len]));
	msg->tags[old_len]->name = allocate_len(name, name_len);
	msg->tags[old_len]->value = allocate_len(esc_value, esc_value_len);
}

static void
failure_test_end_tags(void *user_data)
{
	struct failure_test_struct *test_struct = user_data;
        struct failure_irc_message *msg = test_struct->msg;

	size_t tags_count = count_tags(msg->tags);
	for (size_t i = 0; i < tags_count; ++i) {
		char *name = msg->tags[i]->name;
		char *value = msg->tags[i]->value;
		if (value) {
			printf("Tag %s with value %s\n", name, value);
		} else {
			printf("Tag %s without value\n", name);
		}
	}
}

static ircmsg_parser_callbacks cbs = {
	.start_message = failure_test_start_message,
	.start_tags = failure_test_start_tags,
	.on_tag = failure_test_on_tag,
	.end_tags = failure_test_end_tags,
	.on_prefix = NULL,
	.on_command = NULL,
	.start_params = NULL,
	.on_param = NULL,
	.end_params = NULL,
	.end_message = failure_test_end_message,

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
static void
test_tags_unexpected_end (void **state)
{
	struct failure_test_struct *test_struct = *state;
	const char *tags_str = "    @test;foo=bar\\s \r\n";
	size_t consumed = ircmsg_parse((const uint8_t *) tags_str,
				       strlen(tags_str),
				       &cbs,
				       test_struct);
        assert_int_equal(consumed, 0);
	assert_true(test_struct->failed);
	assert_int_equal(test_struct->code, IRCMSG_ERR_PARSER_UNEXPECTED_END_OF_MESSAGE);
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
		cmocka_unit_test_setup_teardown(test_tags_unexpected_end,
						failure_setup,
						failure_teardown),
	};

	return cmocka_run_group_tests_name("parse_fail_test", tests, NULL, NULL);
}
