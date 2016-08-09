# Copyright (C) 2012, 2014-2016 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# This permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR(S) BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

""" This module enables running shader tests. """

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import io
import re

from framework import exceptions
from .opengl import FastSkipMixin
from .piglit_test import PiglitBaseTest

__all__ = [
    'ShaderTest',
]


class ShaderTest(FastSkipMixin, PiglitBaseTest):
    """ Parse a shader test file and return a PiglitTest instance

    This function parses a shader test to determine if it's a GL, GLES2 or
    GLES3 test, and then returns a PiglitTest setup properly.

    """
    _is_gl = re.compile(r'GL (<|<=|=|>=|>) \d\.\d')
    _match_gl_version = re.compile(
        r'^GL\s+(?P<es>ES)?\s*(?P<op>(<|<=|=|>=|>))\s*(?P<ver>\d\.\d)')
    _match_glsl_version = re.compile(
        r'^GLSL\s+(?P<es>ES)?\s*(?P<op>(<|<=|=|>=|>))\s*(?P<ver>\d\.\d+)')

    def __init__(self, filename):
        gl_required = set()
        gl_version = None
        gles_version = None
        glsl_version = None
        glsl_es_version = None
        op = None
        sl_op = None

        # Iterate over the lines in shader file looking for the config section.
        # By using a generator this can be split into two for loops at minimal
        # cost. The first one looks for the start of the config block or raises
        # an exception. The second looks for the GL version or raises an
        # exception
        with io.open(filename, mode='r', encoding='utf-8') as shader_file:
            # The mock in python 3.3 doesn't support readlines(), so use
            # read().split() as a workaround
            lines = (l for l in shader_file.read().split('\n'))

            # Find the config section
            for line in lines:
                # We need to find the first line of the configuration file, as
                # soon as we do then we can move on to geting the
                # configuration. The first line needs to be parsed by the next
                # block.
                if line.startswith('[require]'):
                    break
            else:
                raise exceptions.PiglitFatalError(
                    "In file {}: Config block not found".format(filename))

        for line in lines:
            if line.startswith('GL_') and not line.startswith('GL_MAX'):
                gl_required.add(line.strip())
                continue

            # Find any GLES requirements.
            if not (gl_version or gles_version):
                m = self._match_gl_version.match(line)
                if m:
                    op = m.group('op')
                    if m.group('es'):
                        gles_version = float(m.group('ver'))
                    else:
                        gl_version = float(m.group('ver'))
                    continue

            if not (glsl_version or glsl_es_version):
                # Find any GLSL requirements
                m = self._match_glsl_version.match(line)
                if m:
                    sl_op = m.group('op')
                    if m.group('es'):
                        glsl_es_version = float(m.group('ver'))
                    else:
                        glsl_version = float(m.group('ver'))
                    continue

            if line.startswith('['):
                break

        # Select the correct binary to run the test, but be as conservative as
        # possible by always selecting the lowest version that meets the
        # criteria.
        if gles_version:
            if op in ['<', '<='] or op in ['=', '>='] and gles_version < 3:
                prog = 'shader_runner_gles2'
            else:
                prog = 'shader_runner_gles3'
        else:
            prog = 'shader_runner'

        super(ShaderTest, self).__init__(
            [prog, filename],
            run_concurrent=True,
            gl_required=gl_required,
            # FIXME: the if here is related to an incomplete feature in the
            # FastSkipMixin
            gl_version=gl_version if op not in ['<', '<='] else None,
            gles_version=gles_version if op not in ['<', '<='] else None,
            glsl_version=glsl_version if sl_op not in ['<', '<='] else None,
            glsl_es_version=glsl_es_version if sl_op not in ['<', '<='] else None)

    @PiglitBaseTest.command.getter
    def command(self):
        """ Add -auto to the test command """
        return self._command + ['-auto']
