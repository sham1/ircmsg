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

#ifndef __SERIALIZER_H_
#define __SERIALIZER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	size_t (*const tag_count)(void *user_data);
        void (*const on_tag)(size_t tag_idx,
			     size_t * const tag_len, const uint8_t **tag,
			     size_t * const val_len, const uint8_t **val,
			     void *user_data);
        bool (*const on_prefix)(size_t * const prefix_len,
				const uint8_t **prefix,
				void *user_data);
	void (*const on_command)(size_t * const command_len,
				 const uint8_t **command,
				 void *user_data);
	size_t (*const param_count)(void *user_data);
        void (*const on_param)(size_t param_idx,
			       size_t * const param_len,
			       const uint8_t **param,
			       void *user_data);
} ircmsg_serializer_callbacks;

void
ircmsg_serialize(uint8_t *buf,
		 size_t buf_size,
		 const ircmsg_serializer_callbacks *cbs,
		 void *user_data);

size_t
ircmsg_serialize_buffer_len(const ircmsg_serializer_callbacks *cbs,
			    void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* ircmsg/serializer.h */
