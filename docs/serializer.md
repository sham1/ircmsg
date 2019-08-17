Serializing IRC messages with ircmsg
====================================

There are two entrypoints for message serialization,`ircmsg_serialize` and
`ircmsg_serialize_buffer_len`, both of which are found in `ircmsg/serializer.h`.
They look like this:

```c
void
ircmsg_serialize(uint8_t *buf,
                 size_t buf_size,
                 const ircmsg_serializer_callbacks *cbs,
                 void *user_data);
```

```c
size_t
ircmsg_serialize_buffer_len(const ircmsg_serializer_callbacks *cbs,
                            void *user_data);
```

`ircmsg_serialize` takes a buffer `buf` and its length `buf_size`, which has to
be equal to or larger than the length gotten from `ircmsg_serialize_buffer_len`
for the given `cbs` and `user_data` combination.

Serialization callbacks
=======================

The serialization callbacks are passed to `ircmsg_serialize` and
`ircmsg_serialize_buffer_len` in a struct that looks like this:

```c
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
```

tag_count
---------

This callback is called at the beginning of the serialization to get the amount
of tags intended to be in the message.

on_tag
------

This callback is called once per tag, where the tag's index is given in
`tag_idx`. The user is expected to return the tag's name and its length in `tag`
and `tag_len` respectively, and the possible value in `val_len` and `val`.

on_prefix
---------

This callback is called to receive the prefix. The user shall return `false` in
the case where the message doesn't have a prefix, or `true` and set `prefix_len`
and `prefix` to the prefix' position in memory and length respectively.

on_command
----------

This callback is called to receive the command. The user shall put the command's
position to `command` and length to `command_len`.

param_count
-----------

This callback is called to see how many parameters the message should have.

on_param
--------

This callback is called once per parameter, and the user gets the expected
parameter's index in `param_idx`. The user shall set `param_len` to be the
parameter's length and `param` to the location of said parameter.

The last parameter will always be treated like the trailing parameter and
it is up to the user to make sure that no parameters before the last have
spaces.
