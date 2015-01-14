# Copyright (C) 2012, 2014 Intel Corporation
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

from __future__ import print_function, absolute_import
import os
import re

from .piglit_test import PiglitBaseTest
from framework import grouptools

__all__ = [
    'ShaderTest',
    'ShaderTestParserException',
    'add_shader_test_dir'
]


class ShaderTest(PiglitBaseTest):
    """ Parse a shader test file and return a PiglitTest instance

    This function parses a shader test to determine if it's a GL, GLES2 or
    GLES3 test, and then returns a PiglitTest setup properly.

    """
    def __init__(self, arguments):
        is_gl = re.compile(r'GL (<|<=|=|>=|>) \d\.\d')
        # Iterate over the lines in shader file looking for the config section.
        # By using a generator this can be split into two for loops at minimal
        # cost. The first one looks for the start of the config block or raises
        # an exception. The second looks for the GL version or raises an
        # exception
        with open(arguments, 'r') as shader_file:
            lines = (l for l in shader_file)

            # Find the config section
            for line in lines:
                # We need to find the first line of the configuration file, as
                # soon as we do then we can move on to geting the
                # configuration. The first line needs to be parsed by the next
                # block.
                if line.lstrip().startswith('[require]'):
                    break
            else:
                raise ShaderTestParserException("Config block not found")

            # Find the OpenGL API to use
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
                        raise ShaderTestParserException("No GL ES version set")
                    break
                elif line.startswith('[') or is_gl.match(line):
                    # In the event that we reach the end of the config black
                    # and an API hasn't been found, it's an old test and uses
                    # "GL"
                    prog = 'shader_runner'
                    break
            else:
                raise ShaderTestParserException("No GL version set")

        super(ShaderTest, self).__init__([prog, arguments], run_concurrent=True)

    @PiglitBaseTest.command.getter
    def command(self):
        """ Add -auto to the test command """
        return self._command + ['-auto']


class ShaderTestParserException(Exception):
    """ An excpetion to be raised for errors in the ShaderTest parser """
    pass


def add_shader_test_dir(group, startdir):
    """Add all shader tests in a directory to the given group."""
    for dirpath, _, filenames in os.walk(startdir):
        for filename in filenames:
            testname, ext = os.path.splitext(filename)
            if ext != '.shader_test':
                continue

            lgroup = grouptools.join(
                grouptools.from_path(os.path.relpath(dirpath, startdir)),
                testname)
            group[lgroup] = ShaderTest(os.path.join(dirpath, filename))
