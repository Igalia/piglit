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
import argparse
import os
import itertools

from templates import template_dir
from modules import utils

TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))

INT_TYPES = ['int', 'ivec2', 'ivec3', 'ivec4']

UINT_TYPES = ['uint', 'uvec2', 'uvec3', 'uvec4']

DOUBLE_TYPES = ['double', 'dvec2', 'dvec3', 'dvec4',
                'dmat2', 'dmat2x3', 'dmat2x4',
                'dmat3x2', 'dmat3', 'dmat3x4',
                'dmat4x2', 'dmat4x3', 'dmat4']

def get_dir_name(ver):
    """Returns the directory name to save tests given a GLSL version."""

    assert isinstance(ver, str)
    if ver.startswith('GL_'):
        feature_dir = ver[3:].lower()
    elif ver.endswith(' es'):
        feature_dir = 'glsl-es-{}.{}'.format(ver[0], ver[1:3])
    else:
        feature_dir = 'glsl-{}.{}'.format(ver[0], ver[1:])

    return os.path.join('spec', feature_dir, 'compiler',
                        'flat_interpolation')


def generate(type_name, mode, interface_block, struct, array, ver, names_only):
    """Generate GLSL parser tests."""

    assert isinstance(type_name, str)
    assert isinstance(mode, str)
    assert isinstance(interface_block, bool)
    assert isinstance(struct, bool)
    assert isinstance(array, bool)
    assert isinstance(ver, str)
    assert isinstance(names_only, bool)

    filename = os.path.join(
        get_dir_name(ver),
        '{}{}{}{}-{}{}.frag'.format(mode,
                                    '-interface_block' if interface_block else '',
                                    '-struct' if struct else '',
                                    '-array' if array else '',
                                    type_name,
                                    '-bad' if mode != 'flat' else ''))

    print(filename)

    if not names_only:
        with open(filename, 'w') as test_file:
            test_file.write(TEMPLATES.get_template(
                'template.frag.mako').render_unicode(
                    ver=ver,
                    mode=mode,
                    type_name=type_name,
                    interface_block=interface_block,
                    struct=struct,
                    array=array))


def create_tests(type_names, glsl_vers, names_only):
    """Creates combinations for flat qualifier tests."""

    assert isinstance(type_names, list)
    assert isinstance(glsl_vers, list)
    assert isinstance(names_only, bool)

    modes = ['flat', 'noperspective', 'smooth', 'default']
    interface_blocks = [True, False]
    structs = [True, False]
    arrays = [True, False]
    if not names_only:
        for ver in glsl_vers:
            utils.safe_makedirs(get_dir_name(ver))

    for t_name, mode, interface_block, struct, array, ver in itertools.product(type_names,
                                                                               modes,
                                                                               interface_blocks,
                                                                               structs,
                                                                               arrays,
                                                                               glsl_vers):
        if ver.endswith(' es'):
            # There is no "noperspective" interpolation mode in GLSL ES
            if mode == 'noperspective':
                continue
            # There is no support for arrays in input structs in GLSL ES
            if struct and array:
                continue
        # Input interface blocks didn't appear in GLSL until 1.50
        if interface_block and ver == '130':
            ver = '150'
        # Input interface blocks didn't appear in GLSL ES until 3.20
        if interface_block and ver == '300 es':
            ver = '320 es'
        # Input structs weren't allowed until 1.50
        if struct and ver == '130':
            ver = '150'
        yield t_name, mode, interface_block, struct, array, ver, names_only


def all_tests(names_only):
    """Creates all the combinations for flat qualifier tests."""

    assert isinstance(names_only, bool)

    # We need additional directories for GLSL 150 and GLSL ES 320
    if not names_only:
        utils.safe_makedirs(get_dir_name('150'))
        utils.safe_makedirs(get_dir_name('320 es'))
    for test_args in (list(create_tests(INT_TYPES + UINT_TYPES,
                                        ['130', '300 es'],
                                        names_only))
                      + list(create_tests(DOUBLE_TYPES,
                                          ['GL_ARB_gpu_shader_fp64', '400'],
                                          names_only))):
        yield test_args


def main():
    """Main function."""

    parser = argparse.ArgumentParser(
        description="Generate non-flat interpolation qualifier tests with fp64 types")
    parser.add_argument(
        '--names-only',
        dest='names_only',
        action='store_true',
        default=False,
        help="Don't output files, just generate a list of filenames to stdout")
    args = parser.parse_args()

    for test_args in all_tests(args.names_only):
        generate(*test_args)


if __name__ == '__main__':
    main()
