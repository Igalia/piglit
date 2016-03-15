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
import optparse
import os
import sys
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


def get_config(ver):
    """Returns the config string for compiler tests given a
       GLSL version
    """
    assert isinstance(ver, str)
    config = (' * expect_result: fail\n'
              ' * glsl_version: {0}\n').format(ver[0] + '.' + ver[1:])
    if ver == '150':
        config += ' * require_extensions: GL_ARB_gpu_shader_fp64\n'
    return config


def get_require(ver):
    """Returns the require string for shader runner tests
       given a GLSL version
    """
    assert isinstance(ver, str)
    require = 'GLSL >=  {0}\n'.format(ver[0] + '.' + ver[1:])
    if ver == '150':
        require += 'GL_ARB_gpu_shader_fp64\n'
    return require


def get_preprocessor(ver):
    """Returns the preprocessor string for a test given a
       GLSL version
    """
    assert isinstance(ver, str)
    preprocessor = '#version {0}\n'.format(ver)
    if ver == '150':
        preprocessor += '#extension GL_ARB_gpu_shader_fp64 : require\n'
    return preprocessor


def get_comments(ver, shader):
    """Returns the comments string for compiler tests
       given a GLSL version and shader stage
    """
    assert isinstance(ver, str)
    assert shader in ('vert', 'frag')
    if ver == '150':
        if shader == 'frag':
            return (
                ' *\n'
                ' * GL_ARB_gpu_shader_fp64 spec states:\n'
                ' *\n'
                ' *     "Fragment outputs can only be float, single-precision\n'
                ' *      floating-point vectors, signed or unsigned integers or\n'
                ' *      integer vectors, or arrays of these."\n')
        elif shader == 'vert':
            return (
                ' *\n'
                ' * GL_ARB_gpu_shader_fp64 spec states:\n'
                ' *\n'
                ' *     "Vertex shader inputs can only be single-precision\n'
                ' *      floating-point scalars, vectors, or matrices, or signed and\n'
                ' *      unsigned integers and integer vectors.  Vertex shader inputs\n'
                ' *      can also form arrays of these types, but not structures."\n')

    return ''


def generate_inout(type_name, shader, ver, names_only):
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
                    config=get_config(ver),
                    comments=get_comments(ver, shader),
                    preprocessor=get_preprocessor(ver),
                    type_name=type_name,
                    extra_params=',0.0' if type_name in ['dvec2', 'dvec3'] else ''))


def generate_pipe(type_name, ver, names_only):
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
                'vs-out-fs-in_template.mako').render_unicode(
                    require=get_require(ver),
                    preprocessor=get_preprocessor(ver),
                    type_name=type_name))


def all_inout_tests(names_only):
    """Creates all the combinations for in/out compilation tests."""

    assert isinstance(names_only, bool)
    type_names = ['double', 'dvec2', 'dvec3', 'dvec4',
                  'dmat2', 'dmat2x3', 'dmat2x4',
                  'dmat3x2', 'dmat3', 'dmat3x4',
                  'dmat4x2', 'dmat4x3', 'dmat4']
    shaders = ['frag', 'vert']
    glsl_ver = ['150', '400']
    for ver in glsl_ver:
        utils.safe_makedirs(get_dir_name(ver, 'compiler'))

    for t_name, shader, ver in itertools.product(type_names, shaders, glsl_ver):
        yield t_name, shader, ver, names_only


def all_pipe_tests(names_only):
    """Creates all the combinations for in/out shader runnertests."""

    assert isinstance(names_only, bool)
    type_names = ['double', 'dvec2', 'dvec3', 'dvec4']
    glsl_ver = ['150', '400']
    for ver in glsl_ver:
        utils.safe_makedirs(get_dir_name(ver, 'execution'))

    for t_name, ver in itertools.product(type_names, glsl_ver):
        yield t_name, ver, names_only


def main():
    """Main function."""

    parser = optparse.OptionParser(
        description="Generate in/out tests for fp64",
        usage="usage: %prog [-h] [--names-only]")
    parser.add_option(
        '--names-only',
        dest='names_only',
        action='store_true',
        help="Don't output files, just generate a list of filenames to stdout")

    (options, args) = parser.parse_args()

    if len(args) != 0:
        # User gave extra args.
        parser.print_help()
        sys.exit(1)

    for args in all_inout_tests(bool(options.names_only)):
        generate_inout(*args)

    for args in all_pipe_tests(bool(options.names_only)):
        generate_pipe(*args)


if __name__ == '__main__':
    main()
