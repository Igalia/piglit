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
import re

import six

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
        self.gl_required = set()

        # Iterate over the lines in shader file looking for the config section.
        # By using a generator this can be split into two for loops at minimal
        # cost. The first one looks for the start of the config block or raises
        # an exception. The second looks for the GL version or raises an
        # exception
        with open(filename, 'r') as shader_file:
            if six.PY3:
                lines = (l for l in shader_file.readlines())
            elif six.PY2:
                lines = (l.decode('utf-8') for l in shader_file.readlines())

            # Find the config section
            for line in lines:
                # We need to find the first line of the configuration file, as
                # soon as we do then we can move on to geting the
                # configuration. The first line needs to be parsed by the next
                # block.
                if line.lstrip().startswith('[require]'):
                    break
            else:
                raise exceptions.PiglitFatalError(
                    "In file {}: Config block not found".format(filename))

            lines = list(lines)

        prog = self.__find_gl(lines, filename)

        super(ShaderTest, self).__init__([prog, filename], run_concurrent=True)

        # This needs to be run after super or gl_required will be reset
        self.__find_requirements(lines)

    def __find_gl(self, lines, filename):
        """Find the OpenGL API to use."""
        for line in lines:
            line = line.strip()
            if line.startswith('GL ES'):
                if line.endswith('3.0'):
                    prog = 'shader_runner_gles3'
                elif line.endswith('2.0'):
                    prog = 'shader_runner_gles2'
                # If we don't set gles2 or gles3 continue the loop,
                # probably htting the exception in the for/else
                else:
                    raise exceptions.PiglitFatalError(
                        "In File {}: No GL ES version set".format(filename))
                break
            elif line.startswith('[') or self._is_gl.match(line):
                # In the event that we reach the end of the config black
                # and an API hasn't been found, it's an old test and uses
                # "GL"
                prog = 'shader_runner'
                break
        else:
            raise exceptions.PiglitFatalError(
                "In file {}: No GL version set".format(filename))

        return prog

    def __find_requirements(self, lines):
        """Find any requirements in the test and record them."""
        for line in lines:
            if line.startswith('GL_') and not line.startswith('GL_MAX'):
                self.gl_required.add(line.strip())
                continue

            if not (self.gl_version or self.gles_version):
                # Find any gles requirements
                m = self._match_gl_version.match(line)
                if m:
                    if m.group('op') not in ['<', '<=']:
                        if m.group('es'):
                            self.gles_version = float(m.group('ver'))
                        else:
                            self.gl_version = float(m.group('ver'))
                        continue

            if not (self.glsl_version or self.glsl_es_version):
                # Find any GLSL requirements
                m = self._match_glsl_version.match(line)
                if m:
                    if m.group('op') not in ['<', '<=']:
                        if m.group('es'):
                            self.glsl_es_version = float(m.group('ver'))
                        else:
                            self.glsl_version = float(m.group('ver'))
                        continue

            if line.startswith('['):
                break

    @PiglitBaseTest.command.getter
    def command(self):
        """ Add -auto to the test command """
        return self._command + ['-auto']
