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

#ifndef __PARSER_H_
#define __PARSER_H_

#include <stdlib.h>
#include <stdint.h>

typedef enum
{
	IRCMSG_ERR_PARSER_MESSAGE_NOT_FOUND,
	IRCMSG_ERR_PARSER_UNEXPECTED_END_OF_MESSAGE,
	IRCMSG_ERR_PARSER_INVALID_SENTINEL,
} ircmsg_parser_err_code;

typedef struct
{
	void (*const start_message)(void *user_data);

	void (*const start_tags)(void *user_data);
	void (*const on_tag)(const uint8_t *name, size_t name_len,
			     const uint8_t *esc_value, size_t esc_value_len,
			     void *user_data);
	void (*const end_tags)(void *user_data);

	void (*const on_prefix)(const uint8_t *prefix, size_t prefix_len,
				void *user_data);

	void (*const on_command)(const uint8_t *command, size_t command_len,
				 void *user_data);

	void (*const start_params)(void *user_data);
	void (*const on_param)(const uint8_t *param, size_t param_len,
			       void *user_data);
	void (*const end_params)(void *user_data);

	void (*const end_message)(void *user_data);

	void (*const on_error)(ircmsg_parser_err_code error, void *user_data);
} ircmsg_parser_callbacks;

/*
 * Parses a single IRC message in `buf`, in range
 * [`buf`, `buf+buf_size`).
 *
 * If parsing is successful, returns the number of bytes
 * consumed, or 0 in case of an error (see the error callback).
 */
size_t
ircmsg_parse(const uint8_t *buf,
	     size_t buf_size,
	     const ircmsg_parser_callbacks *cbs,
	     void *user_data);

#endif /* ircmsg/parser.h */
