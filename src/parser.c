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

static void
parse_tag(const uint8_t *head,
	  const uint8_t *tail,
	  const ircmsg_parser_callbacks *cbs,
	  void *user_data)
{
	const uint8_t *name_head = head;
	size_t name_len = 0;

	const uint8_t *value_head = NULL;
        size_t value_len = 0;

	const uint8_t *iter;
	for (iter = head; head < tail; ++iter) {
		// Values are split from names with 0x3D '='.
		if (*iter == 0x3D) {
			name_len = iter - name_head;
			value_head = iter + 1;
		}
	}

	if (value_head != NULL) {
		value_len = iter - value_head;
	}

	cbs->on_tag(name_head, name_len, value_head, value_len, user_data);
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

	bool found_tags = false;
	bool parsed_tags = false;

	const uint8_t *head = buf;
	for (const uint8_t *iter = buf; iter < buf + buf_size; ++iter, ++bytes_consumed) {
		// If we're not on the last parameter of a command,
		// which may contain whitespaces, jump to the next
		// character to seek for the next message token.
		if (!is_tail_param && is_irc_whitespace (*iter)) {
			// Special consideration is needed if we're currently
			// parsing tags.
			if (found_tags && (!parsed_tags)) {
				if (head != iter) {
					parse_tag(head, iter, cbs, user_data);
					head = iter + 1;
					parsed_tags = true;
				}
			} else {
				head = iter + 1;
			}
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
			if (found_tags && (!parsed_tags)) {
				cbs->on_error(IRCMSG_ERR_PARSER_UNEXPECTED_END_OF_MESSAGE, user_data);
				hit_error = true;
				break;
			}
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

		// IRCv3 introduced so-called "tags" to messages. These tags are optional,
		// but if they are present, the byte to indicate as much is 0x40 '@'.
		if (*iter == 0x40) {
			found_message_start = true;
			found_tags = true;
			cbs->start_message(user_data);
			cbs->start_tags(user_data);
			head = iter + 1;

			continue;
		}

		if (found_tags && (!parsed_tags)) {
			if (*iter == ';') {
				parse_tag(head, iter, cbs, user_data);
				head = iter + 1;
			}
		}
	}

	if (!found_message_start && !hit_error) {
		cbs->on_error(IRCMSG_ERR_PARSER_UNEXPECTED_END_OF_MESSAGE, user_data);
		hit_error = true;
	}

	return hit_error ? 0 : bytes_consumed;
}
