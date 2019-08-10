Parsing IRC messages with ircmsg
================================

The main entrypoint to message parsing is `ircmsg_parse`, found in
`ircmsg/parser.h`. It looks like this:

```c
size_t
ircmsg_parse(const uint8_t *buf,
             size_t buf_size,
             const ircmsg_parser_callbacks *cbs,
             void *user_data);
```

* The return value indicates how many bytes were consumed, or 0 in case of an
  error.
* `buf` is the buffer wherein the IRC message is contained, and `buf_size` is
  the length of said buffer.
* `cbs` is a pointer to a struct of user-supplied parsing callbacks, and
  `user_data` is a pointer passed to said callbacks, which is used to point to
  user-defined parsing state.

Parsing callbacks
=================

The parsing callbacks are passed to `ircmsg_parse` in a struct that looks like
this:

```c
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
```

start_message
-------------

This callback is called when the start of the message is detected. This is
useful for instance with a message builder, wherein you want to allocate the
builder at this stage.

start_tags
----------

This callback is called when IRCv3 tags are encountered. This is useful for
instance with aforementioned message builder, where you want to create a data
structure in the builder where it is easy to append tag structures to.

on_tag
------

This callback is called when a tag has been found. The `name`-field is a pointer
to the name of the tag, and `name_len` is its length, so the name in all is
between `[name, name + name_len)`.

Note: Wrt. tag names, if the name of the tag is a duplicate (i.e. you have seen
this particular name before), the latest value is to be considered. It can be
dealth with either here or `end_tags` (see below).

`esc_value` and `esc_value_len` tell you where and how long the tag's value
is. If there is no value, or if the value is empty, `esc_value` will be `NULL`
and `esc_value_len` is 0. Otherwise the value is found between `[esc_value,
esc_value + esc_value_len)`.

Note that this value will be escaped according to the rules specified
[here](https://ircv3.net/specs/extensions/message-tags.html). To unescape the
value, you can use `ircmsg_tag_value_unescaped_size` and
`ircmsg_tag_value_unescape`, which are documented later.

end_tags
--------

This callback is called when all message tags are parsed. This is useful for
instance when you want to turn your easy-to-append data structure to a data
structure where it is easy to search for a specific value (e.g a hashmap from
the name to the value). This is also a good place to deduplicate the tag names
(see above) if it wasn't done in `on_tag`.

on_prefix
---------

This callback is called when a message prefix is encountered. The `prefix` is a
pointer to the beginning of said prefix, and `prefix_len` is its length. Thus
the whole prefix can be found within `[prefix, prefix + prefix_len)`.

on_command
----------

This callback is called when a message command is encountered. The `command` is
a pointer to the beginning of said prefix, and `command_len` is its length. Thus
the whole prefix can be found within `[command, command + command_len)`.

start_params
------------

This callback is called when any arguments are encountered. This is useful for
instance so that you can allocate a data structure where you can easily append
structures representing the parameters of the message.

on_param
--------

This callback is called when an argument is parsed. `param` points to the
beginning of the parameter and `param_len` is its length, and thus the parameter
is found in `[param, param + param_len)`. The parameter may have the length of
zero or contain ASCII spaces if it's the last parameter of the message.


end_params
----------

This callback is called when all parameters have been parsed. This is useful for
instance to turn a deque of parameters into a data structure that is easy to
index into to get a particular parameter.

end_message
-----------

This callback is called when the whole message has been parsed. This is useful
for instance with aforementioned message builders where you want to consume the
builder and produce an immutable message.

on_error
--------

This callback is called when the parser encounters an error. `error` describes
the type of error. If an error is encountered, the parser stops, and doesn't
consume any data.

Parsing helpers
===============

These are helpful functions that may be used to aid the user in parsing certain
things.

ircmsg_tag_value_unescaped_size
-------------------------------

The function:

```c
size_t
ircmsg_tag_value_unescaped_size(const uint8_t *esc_value,
                                size_t esc_value_len);
```

This function is used to figure out how long a buffer has to be in order to hold
an unescaped version of the escaped tag value `esc_value` of length
`esc_value_len`.

This function doesn't include room for a NUL-byte, so for that you want to add
one to the return value.

ircmsg_tag_value_unescape
-------------------------

The function:

```c
uint8_t *
ircmsg_tag_value_unescape(const uint8_t *esc_value,
                          size_t esc_value_len,
                          uint8_t *buf,
                          size_t buf_len);
```

This function unescapes an escaped tag value in `esc_value` of length
`esc_value_len` into a buffer `buf` of length `buf_len`. The length of the
buffer has to be at least as long as what `ircmsg_tag_value_unescaped_size`
would give for the values of `esc_value` and `esc_value_len` passed.

The function returns `buf` for convenience.
