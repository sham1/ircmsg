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

#include "parser_test.h"
#include <string.h>
#include <stdlib.h>

static char *
alloc_strlen(const char *str, size_t len)
{
	if (str == NULL || len == 0) return NULL;
	char *ret = calloc(len + 1, sizeof(*ret));
	memcpy(ret, str, len);
	return ret;
}

void
test_start_message(void *user_data)
{
	struct irc_test *test_data = user_data;
	test_data->msg = alloc_msg();
}

void
test_start_tags(void *user_data)
{
	struct irc_test *test_data = user_data;
	struct irc_msg *msg = test_data->msg;
	msg->tags = allocate_tags();
}

void
test_on_tag(const uint8_t *name, size_t name_len,
	    const uint8_t *esc_value, size_t esc_value_len,
	    void *user_data)
{
	struct irc_test *test_data = user_data;
	struct irc_msg *msg = test_data->msg;

	struct irc_tag *tag = calloc(1, sizeof(*tag));
	tag->name = alloc_strlen((const char *) name, name_len);

	size_t value_len = ircmsg_tag_value_unescaped_size(esc_value, esc_value_len);
	tag->value = calloc(value_len + 1, sizeof(*tag->value));

	ircmsg_tag_value_unescape(esc_value, esc_value_len,
				  (uint8_t *) tag->value, value_len);

	append_tag(tag, &msg->tags);
}

void
test_end_tags(void *user_data)
{
	(void) user_data;
}

void
test_on_prefix(const uint8_t *prefix, size_t prefix_len,
	       void *user_data)
{
	struct irc_test *test_data = user_data;
	struct irc_msg *msg = test_data->msg;

	msg->prefix = alloc_strlen((const char *) prefix, prefix_len);
}

void
test_on_command(const uint8_t *command, size_t command_len,
		     void *user_data)
{
	struct irc_test *test_data = user_data;
	struct irc_msg *msg = test_data->msg;

	msg->command = alloc_strlen((const char *) command, command_len);
}

void
test_start_params(void *user_data)
{
	struct irc_test *test_data = user_data;
	struct irc_msg *msg = test_data->msg;
	msg->params = allocate_params();
}

void
test_on_param(const uint8_t *param, size_t param_len,
	      void *user_data)
{
	struct irc_test *test_data = user_data;
	struct irc_msg *msg = test_data->msg;
        char *captured_param = alloc_strlen((const char *) param, param_len);

	append_param(captured_param, &msg->params);
}

void
test_end_params(void *user_data)
{
	(void) user_data;
}

void
test_end_message(void *user_data)
{
	(void) user_data;
}

void
test_on_error(ircmsg_parser_err_code error, void *user_data)
{
	struct irc_test *test_data = user_data;

	test_data->failed = true;
	test_data->code = error;

	free_msg(test_data->msg);
	test_data->msg = NULL;
}

ircmsg_parser_callbacks test_cbs = {
	.start_message = test_start_message,

	.start_tags = test_start_tags,
	.on_tag = test_on_tag,
	.end_tags = test_end_tags,

	.on_prefix = test_on_prefix,

	.on_command = test_on_command,

	.start_params = test_start_params,
	.on_param = test_on_param,
	.end_params = test_end_params,

	.end_message = test_end_message,

	.on_error = test_on_error,
};

static size_t
ptrarr_len(const void **ptrarr)
{
	size_t count = 0;
	for (const void **iter = ptrarr; *iter != NULL; ++iter) {
		++count;
	}
	return count;
}

static void
ptrarr_resize(void ***ptrarr, size_t new_size)
{
	*ptrarr = realloc(*ptrarr, (new_size + 1) * sizeof(**ptrarr));
	(*ptrarr)[new_size] = NULL;
}

bool
are_tags_equal(const struct irc_tag *tag1, const struct irc_tag *tag2)
{
	if (strcmp(tag1->name, tag2->name) != 0) return false;
	if (tag1->value != NULL) {
		if (tag2->value == NULL) return false;
		if (strcmp(tag1->value, tag2->value) != 0) return false;
	} else if (tag1->value == NULL && tag2->value != NULL) {
		return false;
	}
	return true;
}

bool
are_msgs_equal(const struct irc_msg *msg1, const struct irc_msg *msg2)
{
        if (msg1->tags != NULL) {
		if (msg2->tags == NULL) return false;
		size_t msg1_tags_len = ptrarr_len((const void **) msg1->tags);
		size_t msg2_tags_len = ptrarr_len((const void **) msg2->tags);
		if (msg1_tags_len != msg2_tags_len) return false;

		for (size_t i = 0; i < msg1_tags_len; ++i) {
			struct irc_tag *tag1 = msg1->tags[i];
			struct irc_tag *tag2 = msg2->tags[i];
			if (!are_tags_equal(tag1, tag2)) return false;
		}
	} else if (msg1->tags == NULL && msg2->tags != NULL) {
		return false;
	}

	if (msg1->prefix != NULL) {
		if (msg2->prefix == NULL) return false;
		if (strcmp(msg1->prefix, msg2->prefix) != 0) return false;
	} else if (msg1->prefix == NULL && msg2->prefix != NULL) {
		return false;
	}

	if (strcmp(msg1->command, msg2->command) != 0) return false;

	if (msg1->params != NULL) {
		if (msg2->params == NULL) return false;
		size_t msg1_params_len = ptrarr_len((const void **) msg1->params);
		size_t msg2_params_len = ptrarr_len((const void **) msg2->params);
		if (msg1_params_len != msg2_params_len) return false;

		for (size_t i = 0; i < msg1_params_len; ++i) {
		        char *param1 = msg1->params[i];
		        char *param2 = msg2->params[i];
			if (strcmp(param1, param2) != 0) return false;
		}
	} else if ((msg1->params == NULL) && (msg2->params != NULL)) {
		return false;
	}

	return true;
}

struct irc_tag **
allocate_tags(void)
{
	struct irc_tag **ret = calloc(1, sizeof(*ret));
	return ret;
}

char **
allocate_params(void)
{
	char **ret = calloc(1, sizeof(*ret));
	return ret;
}

void
append_tag(struct irc_tag *tag, struct irc_tag ***tags)
{
	size_t old_len = ptrarr_len((const void **) *tags);
	size_t new_len = old_len + 1;
        ptrarr_resize((void ***) tags, new_len);
	(*tags)[old_len] = tag;
}

void
append_param(char *param, char ***params)
{
	size_t old_len = ptrarr_len((const void **) *params);
	size_t new_len = old_len + 1;
        ptrarr_resize((void ***) params, new_len);
	*params[new_len] = param;
}

void
free_tags(struct irc_tag **tags)
{
	if (tags == NULL) return;

	size_t tags_len = ptrarr_len((const void **) tags);
	for (size_t i = 0; i < tags_len; ++i) {
		free(tags[i]->name);
		free(tags[i]->value);
		free(tags[i]);
	}
	free(tags);
}

void
free_params(char **params)
{
	if (params == NULL) return;

	size_t params_len = ptrarr_len((const void **) params);
	for (size_t i = 0; i < params_len; ++i) {
		free(params[i]);
	}
	free(params);
}

struct irc_msg *
alloc_msg(void)
{
	struct irc_msg *ret = calloc(1, sizeof(*ret));
	return ret;
}

void
free_msg(struct irc_msg *msg)
{
	if (msg == NULL) return;
	free_tags(msg->tags);
	free(msg->prefix);
	free(msg->command);
	free_params(msg->params);
	free(msg);
}
