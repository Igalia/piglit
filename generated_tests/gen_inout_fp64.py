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
import argparse
import os
import itertools

from templates import template_dir
from modules import utils

TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))

def get_dir_name(ver, test_type):
    """Returns the directory name to save tests given a GLSL version."""

    assert isinstance(ver, str)
    assert isinstance(test_type, str)
    if ver == '150':
        feature_dir = 'arb_gpu_shader_fp64'
    else:
        feature_dir = 'glsl-' + ver[0] + '.' + ver[1:]

    return os.path.join('spec', feature_dir, test_type,
                        'inout')


def generate_compilation_tests(type_name, shader, ver, names_only):
    """Generate in/out GLSL compilation tests."""

    assert isinstance(type_name, str)
    assert shader in ('vert', 'frag')
    assert isinstance(ver, str)
    assert isinstance(names_only, bool)

    filename = os.path.join(
        get_dir_name(ver, 'compiler'),
        '{0}-{1}put-{2}.{3}'.format('fs' if shader == 'frag' else 'vs',
                                    'out' if shader == 'frag' else 'in',
                                    type_name, shader))

    print(filename)

    if not names_only:
        with open(filename, 'w') as test_file:
            test_file.write(TEMPLATES.get_template(
                'template.{0}.mako'.format(shader)).render_unicode(
                    glsl_version='{}.{}'.format(ver[0], ver[1:]),
                    glsl_version_int=ver,
                    type_name=type_name,
                    extra_params=',0.0' if type_name in ['dvec2', 'dvec3'] else ''))


def generate_execution_tests(type_name, ver, names_only):
    """Generate in/out shader runner tests."""

    assert isinstance(type_name, str)
    assert isinstance(ver, str)
    assert isinstance(names_only, bool)

    filename = os.path.join(
        get_dir_name(ver, 'execution'),
        'vs-out-fs-in-{0}.shader_test'.format(type_name))

    print(filename)

    if not names_only:
        with open(filename, 'w') as test_file:
            test_file.write(TEMPLATES.get_template(
                'template.shader_test.mako').render_unicode(
                    glsl_version='{}.{}'.format(ver[0], ver[1:]),
                    glsl_version_int=ver,
                    type_name=type_name))


def all_compilation_tests(names_only):
    """Creates all the combinations for in/out compilation tests."""

    assert isinstance(names_only, bool)
    type_names = ['double', 'dvec2', 'dvec3', 'dvec4',
                  'dmat2', 'dmat2x3', 'dmat2x4',
                  'dmat3x2', 'dmat3', 'dmat3x4',
                  'dmat4x2', 'dmat4x3', 'dmat4']
    shaders = ['frag', 'vert']
    glsl_ver = ['150', '400']
    if not names_only:
        for ver in glsl_ver:
            utils.safe_makedirs(get_dir_name(ver, 'compiler'))

    for t_name, shader, ver in itertools.product(type_names, shaders, glsl_ver):
        yield t_name, shader, ver, names_only


def all_execution_tests(names_only):
    """Creates all the combinations for in/out shader runner tests."""

    assert isinstance(names_only, bool)
    type_names = ['double', 'dvec2', 'dvec3', 'dvec4']
    glsl_ver = ['150', '400']
    if not names_only:
        for ver in glsl_ver:
            utils.safe_makedirs(get_dir_name(ver, 'execution'))

    for t_name, ver in itertools.product(type_names, glsl_ver):
        yield t_name, ver, names_only


def main():
    """Main function."""

    parser = argparse.ArgumentParser(
        description="Generate in/out tests for fp64")
    parser.add_argument(
        '--names-only',
        dest='names_only',
        action='store_true',
        default=False,
        help="Don't output files, just generate a list of filenames to stdout")
    args = parser.parse_args()

    for test_args in all_compilation_tests(args.names_only):
        generate_compilation_tests(*test_args)

    for test_args in all_execution_tests(args.names_only):
        generate_execution_tests(*test_args)


if __name__ == '__main__':
    main()
