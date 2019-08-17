#!/usr/bin/env python3

# Copyright (c) 2019 Jani Juhani Sinervo
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import yaml
import pprint
import sys

def c_escape(string):
    ret = ''
    for c in string:
        if c == '\\':
            ret = ret + '\\\\'
        elif c == '\r':
            ret = ret + '\\r'
        elif c == '\n':
            ret = ret + '\\n'
        else:
            ret = ret + c
    return ret

def generate_match(match):
    return f'\"{c_escape(match)}\\r\\n\"'

def generate_matches(matches):
    return ',\n'.join((generate_match(match) for match in matches))

def generate_tag(i, name, value):
    name_ptr = 'NULL' if len(name) == 0 else f'\"{c_escape(name)}\"'
    return (
        f'struct irc_tag tag_{i} = {{\n'
        f'    .name = {name_ptr},\n'
        f'    .value = \"{c_escape(value)}\",\n'
        f'}};'
    )

def generate_tags(tags):
    return '\n'.join((generate_tag(i, name, value) for i, (name, value) in
                      enumerate(tags.items())))

def generate_tag_ptr(i):
    return f'&tag_{i}'

def generate_tags_arr(tags):
    template = """
    struct irc_tag *tags[] = {{
        {tags},
        NULL,
    }}
    """
    tags = ',\n        '.join((generate_tag_ptr(i) for i, _ in
                               enumerate(tags)))
    vals = {'tags': tags,}
    return template.format(**vals).strip()

def generate_params_arr(params):
    template = """
    char *params[] = {{
        {params},
        NULL,
    }}
    """
    params = ',\n        '.join((f'\"{c_escape(param)}\"' for param in params))
    vals = {'params': params,}
    return template.format(**vals).strip()

def generate_test_func(i, test):
    atoms = test["atoms"]

    template = """
    static void
    test_{i}(void **state)
    {{
        {tags}
        {tags_arr};
        {params_arr};

        struct irc_msg msg = {{
            .tags = {tags_arr_ptr},
            .prefix = {prefix},
            .command = {command},
            .params = {params_arr_ptr},
        }};

        const char *matches[] = {{
            {matches},
        }};

        size_t serialized_size =
            ircmsg_serialize_buffer_len(&serializer_test_cbs,
                                        &msg);

        assert_true(matches_any_length(matches, {matches_count},
                                       serialized_size));

        uint8_t *serialize_buf = calloc(serialized_size + 1,
                                        sizeof(*serialize_buf));

        ircmsg_serialize(serialize_buf, serialized_size,
                         &serializer_test_cbs, &msg);

        assert_true(matches_any(matches, {matches_count},
                                (char *) serialize_buf));
    }}
    """

    tags = generate_tags(atoms["tags"]) if "tags" in atoms else ''
    tags_arr = generate_tags_arr(atoms["tags"]) if "tags" in atoms else ''
    params_arr = generate_params_arr(atoms["params"]) if "params" in atoms \
                                     else ''
    tags_arr_ptr = 'tags' if "tags" in atoms else 'NULL'
    prefix = f'\"{c_escape(atoms["source"])}\"' \
        if "source" in atoms else 'NULL'
    command = f'\"{atoms["verb"]}\"'
    params_arr_ptr = 'params' if "params" in atoms else 'NULL'
    matches = generate_matches(test["matches"])
    matches_count = len(test["matches"])

    vals = {'i': i,
            'tags': tags,
            'tags_arr': tags_arr,
            'params_arr': params_arr,
            'tags_arr_ptr': tags_arr_ptr,
            'prefix': prefix,
            'command': command,
            'params_arr_ptr': params_arr_ptr,
            'matches': matches,
            'matches_count': matches_count,}

    return template.format(**vals)

def generate_funcs(tests):
    ret = []
    for i, test in enumerate(tests):
        ret.append(generate_test_func(i, test))
    return ret

def generate_func_setup(i):
    return (
        f'cmocka_unit_test_setup_teardown(test_{i},\n'
        f'                                serializer_basic_setup,\n'
        f'                                serializer_basic_teardown)'
    )

def generate_func_setups(tests):
    ret = []
    for i, _ in enumerate(tests):
        ret.append(generate_func_setup(i))
    return ret

tests = None

with open(sys.argv[1], 'r') as test_file:
    tests = yaml.safe_load(test_file)['tests']

funcs = '\n'.join(generate_funcs(tests))
func_setups = ',\n'.join(generate_func_setups(tests))

with open(sys.argv[2], 'r') as template:
    values = {'tests': funcs,
              'test_setups': func_setups,}
    contents = template.read().format(**values)
    with open(sys.argv[3], 'w') as output:
        output.write(contents)
