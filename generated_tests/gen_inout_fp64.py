# coding=utf-8
#
# Copyright © 2016 Intel Corporation
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

"""Generate in/out fp64 tests."""

from __future__ import print_function, division, absolute_import
import os
import itertools

from templates import template_dir
from modules import utils

TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))


def generate_inout(dirname, type_name, shader, direction, template):
    """Generate glsl parser tests."""
    filename = os.path.join(
        dirname,
        '{0}-{1}put-{2}.{3}'.format(shader, direction, type_name, template))

    print(filename)
    with open(filename, 'w') as f:
        f.write(TEMPLATES.get_template(
            'template.{0}.mako'.format(template)).render_unicode(
                type_name=type_name,
                extra_params=',0.0' if type_name in ['dvec2', 'dvec3'] else ''))

def generate_pipe(dirname, type_name):
    """Generate shader tests."""
    filename = os.path.join(
        dirname,
        'vs-out-fs-in-{0}.shader_test'.format(type_name))

    print(filename)
    with open(filename, 'w') as f:
        f.write(TEMPLATES.get_template(
            'vs-out-fs-in_template.mako').render_unicode(
                type_name=type_name))

def all_inout_tests():
    type_names = ['double', 'dvec2', 'dvec3', 'dvec4',
                  'dmat2', 'dmat2x3', 'dmat2x4',
                  'dmat3x2', 'dmat3', 'dmat3x4',
                  'dmat4x2', 'dmat4x3', 'dmat4']
    shaders = ['fs', 'vs']
    direction = {'fs': 'out',
                 'vs': 'in'}
    template = {'fs': 'frag',
                'vs': 'vert'}

    for t, s in itertools.product(type_names, shaders):
        yield t, s, direction[s], template[s]

def main():
    """Main function."""
    type_names = ['double', 'dvec2', 'dvec3', 'dvec4']
    dirname_pre = os.path.join('spec', 'arb_gpu_shader_fp64', 'preprocessor',
                               'inout')
    utils.safe_makedirs(dirname_pre)

    for args in all_inout_tests():
        generate_inout(dirname_pre, *args)


    dirname_exe = os.path.join('spec', 'arb_gpu_shader_fp64', 'execution',
                               'inout')
    utils.safe_makedirs(dirname_exe)

    for t in type_names:
        generate_pipe(dirname_exe, t)

if __name__ == '__main__':
    main()
