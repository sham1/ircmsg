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

#include "ircmsg/parser.h"
#include <stdbool.h>

static bool
is_irc_whitespace(uint8_t byte)
{
	// TODO: Add more byte "characters" that could be considered
	// whitespace.
        switch (byte)
	{
	case 0x20: // ASCII space
		return true;
	default:
		return false;
	}
}

size_t
ircmsg_parse(const uint8_t *buf,
	     size_t buf_size,
	     const ircmsg_parser_callbacks *cbs,
	     void *user_data)
{
	size_t bytes_consumed = 0;

	bool hit_error = false;
	bool found_message_start = false;
	bool is_tail_param = false;

	const uint8_t *head = buf;
	for (const uint8_t *iter = buf; iter < buf + buf_size; ++iter, ++bytes_consumed) {
		// If we're not on the last parameter of a command,
		// which may contain whitespaces, jump to the next
		// character to seek for the next message token.
		if (!is_tail_param && is_irc_whitespace (*iter)) {
			head = iter;
		}

		// The IRC spec specifies that messages are terminated by
		// bytes 0x13 followed by 0x10 (CRLF or \r\n). However, the
		// other party might not be fully spec compliant and might
		// send either only one of them or first LF and then CR.
		//
		// We must be able to cope with all of these situations. In
		// particular, we want to consume both bytes, if they exist.
		if (*iter == '\r' || *iter == '\n') {
			bool was_cr = *iter == '\r';
			bool was_lf = *iter == '\n';
			if (iter != (buf + buf_size) - 1) {
				if (was_lf && *(iter + 1) == '\r') {
					++bytes_consumed;
					++iter;
				        if (found_message_start) {
						cbs->end_message(user_data);
					} else {
						cbs->on_error(IRCMSG_ERR_PARSER_MESSAGE_NOT_FOUND, user_data);
						hit_error = true;
					}
					break;
				} else if (was_cr && *(iter + 1) == '\n') {
					++bytes_consumed;
					++iter;
				        if (found_message_start) {
						cbs->end_message(user_data);
					} else {
						cbs->on_error(IRCMSG_ERR_PARSER_MESSAGE_NOT_FOUND, user_data);
						hit_error = true;
					}
					break;
				} else if (was_lf && *(iter + 1) == '\n') {
					cbs->on_error(IRCMSG_ERR_PARSER_INVALID_SENTINEL, user_data);
					hit_error = true;
					break;
				} else if (was_cr && *(iter + 1) == '\r') {
					cbs->on_error(IRCMSG_ERR_PARSER_INVALID_SENTINEL, user_data);
					hit_error = true;
					break;
				} else {
					if (found_message_start) {
						cbs->end_message(user_data);
					} else {
						cbs->on_error(IRCMSG_ERR_PARSER_MESSAGE_NOT_FOUND, user_data);
						hit_error = true;
					}
					break;
				}
			} else {
				if (found_message_start) {
					cbs->end_message(user_data);
					break;
				} else {
					cbs->on_error(IRCMSG_ERR_PARSER_MESSAGE_NOT_FOUND, user_data);
					hit_error = true;
				}
			}
		}
	}

	if (!found_message_start && !hit_error) {
		cbs->on_error(IRCMSG_ERR_PARSER_UNEXPECTED_END_OF_MESSAGE, user_data);
		hit_error = true;
	}

	return hit_error ? 0 : bytes_consumed;
}
