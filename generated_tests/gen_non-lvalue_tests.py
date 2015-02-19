# coding=utf-8
#
# Copyright © 2012, 2014 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

from __future__ import print_function
import os
import itertools

from templates import template_dir
from modules import utils

TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))

_NAMES = {
    "++t": "preincrement",
    "--t": "predecrement",
    "t++": "postincrement",
    "t--": "postdecrement",
}


def generate(dirname, type_name, op, usage, shader_target):
    """Generate glsl parser tests."""
    if type_name.endswith('2'):
        var_as_vec4 = 'vec4(t.xyxy)'
        components = '.xy'
    elif type_name.endswith('3'):
        var_as_vec4 = 'vec4(t.xyzx)'
        components = '.xyz'
    elif type_name.endswith('4'):
        var_as_vec4 = 'vec4(t)'
        components = ''
    else:
        var_as_vec4 = 'vec4(t)'
        components = '.x'

    if shader_target == 'vert':
        dest = "gl_Position"
        mode = 'attribute'
    else:
        mode = 'varying'
        dest = "gl_FragColor"

    filename = os.path.join(
        dirname,
        '{0}-{1}-non-lvalue-for-{2}.{3}'.format(
            _NAMES[op], type_name, usage, shader_target))

    print(filename)
    with open(filename, 'w') as f:
        f.write(TEMPLATES.get_template(
            '{0}.glsl_parser_test.mako'.format(usage)).render_unicode(
                type_name=type_name,
                mode=mode,
                dest=dest,
                components=components,
                var_as_vec4=var_as_vec4,
                op=op))


def all_tests():
    type_name = ['float', 'vec2', 'vec3', 'vec4', 'int', 'ivec2', 'ivec3',
                 'ivec4']
    op = ["++t", "--t", "t++", "t--"]
    usage = ['assignment', 'out-parameter']
    shader_target = ['vert', 'frag']

    for t, o, u, s in itertools.product(type_name, op, usage, shader_target):
        yield t, o, u, s


def main():
    dirname = os.path.join('spec', 'glsl-1.10', 'compiler', 'expressions')
    utils.safe_makedirs(dirname)

    for args in all_tests():
        generate(dirname, *args)


if __name__ == '__main__':
    main()
