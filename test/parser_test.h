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

#ifndef __PARSER_TEST_H_
#define __PARSER_TEST_H_

#include <stdbool.h>
#include <ircmsg/parser.h>

struct irc_tag
{
	char *name;
	char *value;
};

bool are_tags_equal(const struct irc_tag *tag1, const struct irc_tag *tag2);

struct irc_msg
{
	struct irc_tag **tags;
	char *prefix;
	char *command;
	char **params;
};

bool are_msgs_equal(const struct irc_msg *msg1, const struct irc_msg *msg2);

struct irc_tag **allocate_tags(void);
char **allocate_params(void);

void append_tag(struct irc_tag *tag, struct irc_tag ***tags);
void append_param(char *param, char ***params);

void free_tags(struct irc_tag **tags);
void free_params(char **params);

struct irc_test
{
	struct irc_msg *msg;
	bool failed;
	ircmsg_parser_err_code code;
};

struct irc_msg *alloc_msg(void);
void free_msg(struct irc_msg *msg);

extern ircmsg_parser_callbacks test_cbs;

#endif /* parser_test.h */
