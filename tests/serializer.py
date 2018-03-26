# encoding=utf-8
# Copyright © 2018 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

"""Script for taking profiles in python format and serializing them to XML."""

import argparse
import os
import sys
import xml.etree.cElementTree as et

import six

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))

from framework.test.piglit_test import (
    PiglitGLTest, PiglitCLTest, ASMParserTest, BuiltInConstantsTest,
    CLProgramTester,
)
from framework.test.shader_test import ShaderTest, MultiShaderTest
from framework.test.glsl_parser_test import GLSLParserTest
from framework.profile import load_test_profile
from framework.options import OPTIONS


def parser():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser()
    parser.add_argument('name')
    parser.add_argument('input')
    parser.add_argument('output')
    parser.add_argument('--no-process-isolation', action='store_true')
    parser.add_argument('--glsl-arb-compat', action='store_true')
    args = parser.parse_args()
    return args


def _serialize_skips(test, elem):
    if getattr(test, 'gl_required', None):
        et.SubElement(elem, 'option', name='gl_required',
                      value=repr(test.gl_required))
    if getattr(test, 'gl_version', None):
        et.SubElement(elem, 'option', name='gl_version',
                      value=repr(test.gl_version))
    if getattr(test, 'gles_version', None):
        et.SubElement(elem, 'option', name='gles_version',
                      value=repr(test.gles_version))
    if getattr(test, 'glsl_version', None):
        et.SubElement(elem, 'option', name='glsl_version',
                      value=repr(test.glsl_version))
    if getattr(test, 'glsl_es_version', None):
        et.SubElement(elem, 'option', name='glsl_es_version',
                      value=repr(test.glsl_es_version))


def serializer(name, profile, outfile):
    """Take each test in the profile and write it out into the xml."""
    # TODO: This is going to take a lot of memory
    root = et.Element('PiglitTestList', count=six.text_type(len(profile)),
                      name=name)
    for name, test in profile.itertests():
        if isinstance(test, PiglitGLTest):
            elem = et.SubElement(root, 'Test', type='gl', name=name)
            if test.require_platforms:
                et.SubElement(elem, 'option', name='require_platforms',
                              value=repr(test.require_platforms))
            if test.exclude_platforms:
                et.SubElement(elem, 'option', name='exclude_platforms',
                              value=repr(test.exclude_platforms))
            _serialize_skips(test, elem)
        elif isinstance(test, BuiltInConstantsTest):
            elem = et.SubElement(root, 'Test', type='gl_builtin', name=name)
        elif isinstance(test, GLSLParserTest):
            elem = et.SubElement(root, 'Test', type='glsl_parser', name=name)
            _serialize_skips(test, elem)
        elif isinstance(test, ASMParserTest):
            elem = et.SubElement(root, 'Test', type='asm_parser', name=name)
            et.SubElement(elem, 'option', name='type_',
                          value=repr(test.command[1]))
            et.SubElement(elem, 'option', name='filename',
                          value=repr(test.filename))
            continue
        elif isinstance(test, ShaderTest):
            elem = et.SubElement(root, 'Test', type='shader', name=name)
            _serialize_skips(test, elem)
        elif isinstance(test, MultiShaderTest):
            elem = et.SubElement(root, 'Test', type='multi_shader', name=name)
            et.SubElement(elem, 'option', name='prog', value=repr(test.prog))
            et.SubElement(elem, 'option', name='files', value=repr(test.files))
            et.SubElement(elem, 'option', name='subtests', value=repr(test.subtests))
            skips = et.SubElement(elem, 'Skips')
            for s in test.skips:
                skip = et.SubElement(skips, 'Skip')
                _serialize_skips(s, skip)
            continue
        elif isinstance(test, CLProgramTester):
            elem = et.SubElement(root, 'Test', type='cl_prog', name=name)
            et.SubElement(elem, 'option', name='filename',
                          value=repr(test.filename))
            continue
        elif isinstance(test, PiglitCLTest):
            elem = et.SubElement(root, 'Test', type='cl', name=name)
            et.SubElement(elem, 'option', name='command', value=repr(test._command))
            continue
        else:
            continue

        et.SubElement(elem, 'option', name='command', value=repr(test._command))
        et.SubElement(elem, 'option', name='run_concurrent',
                      value=repr(test.run_concurrent))
        if test.cwd:
            et.SubElement(elem, 'option', name='cwd', value=test.cwd)
        if test.env:
            env = et.SubElement(elem, 'environment')
            for k, v in six.iteritems(test.env):
                et.SubElement(env, 'env', name=k, value=v)

    tree = et.ElementTree(root)
    tree.write(outfile, encoding='utf-8', xml_declaration=True)


def main():
    args = parser()
    OPTIONS.process_isolation = not args.no_process_isolation
    if args.glsl_arb_compat:
        os.environ['PIGLIT_FORCE_GLSLPARSER_DESKTOP'] = 'true'
    profile = load_test_profile(args.input, python=True)
    serializer(args.name, profile, args.output)


if __name__ == '__main__':
    main()
