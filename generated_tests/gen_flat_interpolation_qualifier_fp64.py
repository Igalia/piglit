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

"""Generate non-flat interpolation qualifier tests."""

from __future__ import print_function, division, absolute_import
import os
import itertools

from templates import template_dir
from modules import utils

TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))


def generate(dirname, type_name, mode, additional_type_qualifier):
    """Generate GLSL parser tests."""
    var_name = 'u'
    var_as_vec4 = 's.' + var_name if additional_type_qualifier == 'struct' else var_name
    if additional_type_qualifier == 'array':
        var_as_vec4 += '[3]'
    if type_name.startswith('dmat'):
        var_as_vec4 += '[0]'

    if type_name.endswith('2'):
        var_as_vec4 += '.xyxy'
    elif type_name.endswith('3'):
        var_as_vec4 += '.xyzx'

    filename = os.path.join(
        dirname,
        '{0}-{1}-{2}{3}.frag'.format(mode, additional_type_qualifier, type_name,
                                     '' if mode == 'flat' else '-bad'))

    print(filename)
    with open(filename, 'w') as test_file:
        test_file.write(TEMPLATES.get_template(
            '{0}_template.frag.mako'.format(additional_type_qualifier)).render_unicode(
                expect_result='pass' if mode == 'flat' else 'fail',
                type_name=type_name,
                mode=mode,
                interpolation_mode='' if mode == 'default' else mode + ' ',
                var_name=var_name,
                var_as_vec4=var_as_vec4))


def all_tests():
    """Creates all the combinations for flat qualifier tests."""
    type_names = ['double', 'dvec2', 'dvec3', 'dvec4',
                  'dmat2', 'dmat2x3', 'dmat2x4',
                  'dmat3x2', 'dmat3', 'dmat3x4',
                  'dmat4x2', 'dmat4x3', 'dmat4']
    modes = ['flat', 'noperspective', 'smooth', 'default']
    additional_type_qualifiers = ['scalar', 'array', 'struct']

    for t_name, mode, a_t_qualifier in itertools.product(type_names,
                                                         modes,
                                                         additional_type_qualifiers):
        yield t_name, mode, a_t_qualifier


def main():
    """Main function."""
    dirname = os.path.join('spec', 'arb_gpu_shader_fp64', 'compiler',
                           'flat_interpolation')
    utils.safe_makedirs(dirname)

    for args in all_tests():
        generate(dirname, *args)


if __name__ == '__main__':
    main()
