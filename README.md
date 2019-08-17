ircmsg
======

A dynamic-allocation free C99-compatible IRC message library.

Why use this library
====================

* It's C99

This means that it can be used on a variety of platforms both old and
new.  Moreover, because it is C, it is very easy to write bindings for
with various FFI systems.

* It doesn't use heap allocation

This means that the control over allocations is left entirely to the
user of the library. No library-defined data structures to clear.

This also means that the library doesn't impose upon the user
restrictions of which sorts of data structures they use for their IRC
messages.

* It's IRCv3 compliant

One of the design goals of `ircmsg` was for it to be able to be used
for parsing and serializing modern IRC messages with tags. By having
this feature, higher-level libraries can support modern features such
as batched messages and even getting the message's arrival time from
the server.

* It's free software

`ircmsg` is licensed under the MIT license. This means that the library
can be used both commercially and for FLOSS projects.

Usage
=====

To find out how to use this library, look either at your software
distribution's documentation directory or the `ircmsg` GIT repository's
`docs`-directory.
