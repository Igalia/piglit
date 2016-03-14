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
import optparse
import os
import sys
import itertools

from templates import template_dir
from modules import utils

TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))

def get_dir_name(ver):
    """Returns the directory name to save tests given a GLSL version."""

    assert isinstance(ver, str)
    if ver == '150':
        feature_dir = 'arb_gpu_shader_fp64'
    else:
        feature_dir = 'glsl-' + ver[0] + '.' + ver[1:]

    return os.path.join('spec', feature_dir, 'compiler',
                        'flat_interpolation')


def get_config(ver, mode):
    """Returns the config string for a test given a
       GLSL version and interpolation qualifier mode
    """
    assert isinstance(ver, str)
    config = (' * expect_result: {0}\n'
              ' * glsl_version: {1}\n'.format('pass' if mode == 'flat' else 'fail',
                                              ver[0] + '.' + ver[1:]))
    if ver == '150':
        config += ' * require_extensions: GL_ARB_gpu_shader_fp64\n'
    return config


def get_preprocessor(ver):
    """Returns the preprocessor string for a test given a
       GLSL version
    """
    assert isinstance(ver, str)
    preprocessor = '#version {0}\n'.format(ver)
    if ver == '150':
        preprocessor += '#extension GL_ARB_gpu_shader_fp64 : require\n'
    return preprocessor


def get_comments(ver):
    """Returns the comments string for a test
       given a GLSL version
    """
    assert isinstance(ver, str)
    if ver == '150':
        return (
            ' *\n'
            ' * GL_ARB_gpu_shader_fp64 spec states:\n'
            ' *\n'
            ' *    "Modifications to The OpenGL Shading Language Specification, Version 1.50\n'
            ' *     (Revision 09)\n'
            ' *    ...\n'
            ' *        Modify Section 4.3.4, Inputs, p. 31\n'
            ' *    ...\n'
            ' *        (modify third paragraph, p. 32, allowing doubles as inputs and disallowing\n'
            ' *        as non-flat fragment inputs) ... Fragment inputs can only be signed and\n'
            ' *        unsigned integers and integer vectors, float, floating-point vectors,\n'
            ' *        double, double-precision vectors, single- or double-precision matrices, or\n'
            ' *        arrays or structures of these. Fragment shader inputs that are signed or\n'
            ' *        unsigned integers, integer vectors, doubles, double-precision vectors, or\n'
            ' *        double-precision matrices must be qualified with the interpolation\n'
            ' *        qualifier flat.\n')

    return ''


def generate(type_name, mode, additional_type_qualifier, ver, names_only):
    """Generate GLSL parser tests."""

    assert isinstance(type_name, str)
    assert isinstance(mode, str)
    assert isinstance(additional_type_qualifier, str)
    assert isinstance(ver, str)
    assert isinstance(names_only, bool)
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
        get_dir_name(ver),
        '{0}-{1}-{2}{3}.frag'.format(mode, additional_type_qualifier, type_name,
                                     '' if mode == 'flat' else '-bad'))

    print(filename)

    if not names_only:
        with open(filename, 'w') as test_file:
            test_file.write(TEMPLATES.get_template(
                '{0}_template.frag.mako'.format(additional_type_qualifier)).render_unicode(
                    config=get_config(ver, mode),
                    mode=mode,
                    type_name=type_name,
                    comments=get_comments(ver),
                    preprocessor=get_preprocessor(ver),
                    interpolation_mode='' if mode == 'default' else mode + ' ',
                    var_name=var_name,
                    var_as_vec4=var_as_vec4))


def all_tests(names_only):
    """Creates all the combinations for flat qualifier tests."""

    assert isinstance(names_only, bool)
    type_names = ['double', 'dvec2', 'dvec3', 'dvec4',
                  'dmat2', 'dmat2x3', 'dmat2x4',
                  'dmat3x2', 'dmat3', 'dmat3x4',
                  'dmat4x2', 'dmat4x3', 'dmat4']
    modes = ['flat', 'noperspective', 'smooth', 'default']
    additional_type_qualifiers = ['scalar', 'array', 'struct']
    glsl_ver = ['150', '400']
    for ver in glsl_ver:
        utils.safe_makedirs(get_dir_name(ver))

    for t_name, mode, a_t_qualifier, ver in itertools.product(type_names,
                                                              modes,
                                                              additional_type_qualifiers,
                                                              glsl_ver):
        yield t_name, mode, a_t_qualifier, ver, names_only


def main():
    """Main function."""

    parser = optparse.OptionParser(
        description="Generate non-flat interpolation qualifier tests with fp64 types",
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

    for args in all_tests(bool(options.names_only)):
        generate(*args)


if __name__ == '__main__':
    main()
