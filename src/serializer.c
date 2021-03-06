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

#include "ircmsg/serializer.h"
#include <string.h>

static size_t
get_tag_value_escaped_size(const uint8_t *unesc_val,
			   size_t unesc_val_len);

static bool
is_tag_escapable(uint8_t byte);

static uint8_t
get_tag_escape(uint8_t byte);

void
ircmsg_serialize(uint8_t *buf,
		 size_t buf_size,
		 const ircmsg_serializer_callbacks *cbs,
		 void *user_data)
{
	uint8_t *iter = buf;
	uint8_t *buf_end = buf + buf_size;
#define set_byte(byte)				\
	do {					\
		if (iter < buf_end) {		\
			*iter = (byte);		\
			++iter;			\
		} else {			\
			return;			\
		}				\
	} while (false)

        size_t tag_count = cbs->tag_count(user_data);
	size_t param_count = cbs->param_count(user_data);

	uint8_t tag_prefix = '@';
	bool had_tags = false;
	for (size_t tag_idx = 0; tag_idx < tag_count; ++tag_idx) {
		had_tags = true;
		set_byte(tag_prefix);
		if (tag_prefix == '@') tag_prefix = ';';
		size_t tag_len = 0;
		size_t val_len = 0;
		const uint8_t *tag = NULL;
		const uint8_t *val = NULL;
		cbs->on_tag(tag_idx,
			    &tag_len, &tag,
			    &val_len, &val,
			    user_data);
		if ((iter + tag_len) >= buf_end) return;
	        memcpy(iter, tag, tag_len);
		iter += tag_len;
		if (val_len > 0) {
			set_byte('=');
		        size_t esc_len =
				get_tag_value_escaped_size(val, val_len);
			if ((iter + esc_len) >= buf_end) return;
			for (const uint8_t *val_iter = val;
			     val_iter < (val + val_len);
			     ++val_iter) {
			        if (is_tag_escapable(*val_iter)) {
					set_byte('\\');
					set_byte(get_tag_escape(*val_iter));
					continue;
				}
				set_byte(*val_iter);
			}
		}
	}
	if (had_tags) {
		set_byte(' ');
	}

	{
		size_t prefix_len = 0;
		const uint8_t *prefix = NULL;
		bool has_prefix = cbs->on_prefix(&prefix_len, &prefix,
						 user_data);
		if (has_prefix) {
		        set_byte(':');

			if ((iter + prefix_len) >= buf_end) return;
			memcpy(iter, prefix, prefix_len);

			iter += prefix_len;

			set_byte(' ');
		}
	}

	{
		size_t command_len = 0;
		const uint8_t *command = NULL;

		cbs->on_command(&command_len, &command, user_data);

		if ((iter + command_len) >= buf_end) return;
		memcpy(iter, command, command_len);

		iter += command_len;
	}

	for (size_t param_idx = 0; param_idx < param_count; ++param_idx) {
		set_byte(' ');
		if (param_idx == (param_count - 1)) {
			// The last argument is always treated as trailing.
			set_byte(':');
		}

		size_t param_len = 0;
		const uint8_t *param = NULL;

		cbs->on_param(param_idx, &param_len, &param,
			      user_data);

		if ((iter + param_len) >= buf_end) return;
		memcpy(iter, param, param_len);

		iter += param_len;
	}

	set_byte('\r');
	set_byte('\n');
#undef set_byte
}

size_t
ircmsg_serialize_buffer_len(const ircmsg_serializer_callbacks *cbs,
			    void *user_data)
{
	size_t tag_idx = 0;
	size_t param_idx = 0;

	// Accounts for the \r\n at the end.
	size_t req_size = 2;

	size_t tag_count = cbs->tag_count(user_data);
        while (tag_idx < tag_count) {
		size_t tag_len = 0;
		size_t val_len = 0;

		const uint8_t *tag = NULL;
		const uint8_t *val = NULL;

	        cbs->on_tag(tag_idx++,
			    &tag_len, &tag,
			    &val_len, &val,
			    user_data);

		// Accounts for either the tag prefix '@',
		// or the tag separator.
		++req_size;
		req_size += tag_len;
		if (val_len != 0) {
			// Accounts for the '=' between tag name
			// and value.
			++req_size;
			size_t esc_size =
				get_tag_value_escaped_size(val, val_len);
			req_size += esc_size;
		}
	}
	if (tag_idx != 0) {
		// Accounts for the space that separates
		// message tags from the rest of the
		// messages.
		++req_size;
	}

	{
		size_t prefix_len = 0;
		const uint8_t *prefix = NULL;
		bool has_prefix = cbs->on_prefix(&prefix_len,
						 &prefix,
						 user_data);
		if (has_prefix) {
			// Accounts for the "prefix'" prefix,
			// ':'
			++req_size;
			req_size += prefix_len;

			// Accounts for the space after the prefix.
			++req_size;
		}
	}

	{
		size_t command_len = 0;
		const uint8_t *command = NULL;
		cbs->on_command(&command_len,
				&command,
				user_data);

		req_size += command_len;
	}

	size_t param_count = cbs->param_count(user_data);
	while (param_idx < param_count) {
		// Accounts for the space before every
		// parameter.
		++req_size;
		if (param_idx == (param_count - 1)) {
			// Accounts for the trailing parameter
			// prefix ':'
			++req_size;
		}
		size_t param_len = 0;
		const uint8_t *param = NULL;

		cbs->on_param(param_idx++,
			      &param_len, &param,
			      user_data);

		req_size += param_len;
	}

	return req_size;
}

static size_t
get_tag_value_escaped_size(const uint8_t *unesc_val,
			   size_t unesc_val_len)
{
	size_t esc_size = 0;
	for (const uint8_t *iter = unesc_val;
	     iter < (unesc_val + unesc_val_len);
	     ++iter) {
		switch(*iter) {
		case ';':
		case ' ':
		case '\\':
		case '\r':
		case '\n':
			esc_size += 2;
			break;
		default:
			++esc_size;
			break;
		}
	}
	return esc_size;
}

static bool
is_tag_escapable(uint8_t byte)
{
	switch(byte) {
	case ';':
	case ' ':
	case '\\':
	case '\r':
	case '\n':
	        return true;
	default:
	        return false;
	}
}

static uint8_t
get_tag_escape(uint8_t byte)
{
	switch(byte) {
	case ';':
		return ':';
	case ' ':
		return 's';
	case '\\':
		return '\\';
	case '\r':
		return 'r';
	case '\n':
		return 'n';
	default:
		// Shouldn't be reached.
		return '\0';
	}
}
