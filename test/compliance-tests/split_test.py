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

# These are only the cases needed for the test
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

def tag_to_test(tag_num, name, value):
    if len(value) == 0:
        value = 'NULL'
    else:
        value = f'\"{c_escape(value)}\"'
    return (
        f'struct irc_tag tag_{tag_num} = {{\n'
        f'\t\t.name = \"{name}\", .value = {value},\n'
        f'\t}};'
        )

def tags_to_test(tags):
    return '\n\t'.join(tag_to_test(i, name, value) for i, (name, value)
                       in enumerate(tags.items()))

def tags_to_arr_test(tags):
    ret = 'struct irc_tag *tags_arr[] = {\n'
    for i, _ in enumerate(tags):
        ret += f'\t\t&tag_{i},\n'
    ret += '\t\tNULL,\n\t}'
    return ret

def params_to_arr_test(params):
    ret = 'char *params[] = {\n'
    for param in params:
        ret += f'\t\t\"{c_escape(param)}\",\n'
    ret += '\t\tNULL,\n\t}'
    return ret

def expected_test(atoms):
    tags_arr = 'tags_arr' if "tags" in atoms else 'NULL'
    params_arr = 'params' if "params" in atoms else 'NULL'
    prefix = f'\"{c_escape(atoms["source"])}\"' if "source" in atoms else 'NULL'
    command = f'\"{c_escape(atoms["verb"])}\"'

    return (
        f'struct irc_msg expected = {{\n'
        f'\t\t.tags = {tags_arr},\n'
        f'\t\t.prefix = {prefix},\n'
        f'\t\t.command = {command},\n'
        f'\t\t.params = {params_arr},\n'
        f'\t}}'
        )

def test_to_function(i, input, atoms):
    template = """
    static void
    test_{i}(void **state)
    {{
        {tags}
        {tags_arr};
        {params_arr};
        {expected};

        struct irc_test *test = *state;
        const char *input = \"{input}\\r\\n\";
        size_t consumed = ircmsg_parse((const uint8_t *)input,
                                       strlen(input),
                                       &test_cbs,
                                       test);
        assert_false(test->failed);
        assert_int_equal(consumed, strlen(input));
        assert_true(are_msgs_equal(&expected, test->msg));
    }}
    """
    tags = tags_to_test(atoms["tags"]) if "tags" in atoms else ''
    tags_arr = tags_to_arr_test(atoms["tags"]) if "tags" in atoms else ''
    params_arr = params_to_arr_test(atoms["params"]) if "params" in atoms else ''
    expected = expected_test(atoms)
    vals = {'input': c_escape(input),
            'i': i,
            'tags': tags,
            'tags_arr': tags_arr,
            'params_arr': params_arr,
            'expected': expected,}
    return template.format(**vals)

tests = None

with open(sys.argv[1], 'r') as test_file:
    tests = yaml.safe_load(test_file)['tests']

def generate_funcs():
    f = []
    for i, test in enumerate(tests):
        input = test["input"]
        atoms = test["atoms"]
        f.append(test_to_function(i, input, atoms))
    return '\n'.join(f)

def generate_func_setups():
    f = []
    for i, _ in enumerate(tests):
        f.append(
            f'cmocka_unit_test_setup_teardown(test_{i},\n'
            f'                                success_setup,\n'
            f'                                success_teardown)'
        )
    return ',\n'.join(f)

funcs = generate_funcs()
func_setups = generate_func_setups()

with open(sys.argv[2], 'r') as template:
    values = {'funcs': funcs,
              'func_setups': func_setups,}
    contents = template.read().format(**values)
    with open(sys.argv[3], 'w') as output:
        output.write(contents)
