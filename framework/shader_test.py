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

import os
import os.path as path
import re

from .core import testBinDir, Group, Test, TestResult
from .exectest import PlainExecTest


def add_shader_test(group, testname, filepath):
    group[testname] = ShaderTest([filepath, '-auto'])


def add_shader_test_dir(group, dirpath, recursive=False):
    """Add all shader tests in a directory to the given group."""
    for filename in os.listdir(dirpath):
        filepath = path.join(dirpath, filename)
        if path.isdir(filepath):
            if not recursive:
                continue
            if not filename in group:
                group[filename] = Group()
            add_shader_test_dir(group[filename], filepath, recursive)
        else:
            ext = filename.rsplit('.')[-1]
            if ext != 'shader_test':
                continue
            testname = filename[0:-(len(ext) + 1)]  # +1 for '.'
            add_shader_test(group, testname, filepath)


class ShaderTest(PlainExecTest):
    API_ERROR = 0
    API_GL = 1
    API_GLES2 = 2
    API_GLES3 = 3

    __has_compiled_regexes = False
    __re_require_header = None
    __re_gl = None
    __re_gles2 = None
    __re_gles3 = None
    __re_gl_unknown = None

    @classmethod
    def __compile_regexes(cls):
        """Compile the regular expressions needed to parse shader tests.

        Hundreds, maybe thousands, of ShaderTests may be instantiated.  This
        function compiles the regular expressions only once, at class scope,
        and uses them for all instances.

        This function is idempotent."""

        if cls.__has_compiled_regexes:
            return

        common = {
            'cmp': r'(<|<=|=|>=|>)',
            'gl_version': r'(\d.\d)',
            'gles2_version': r'(2.\d\s)',
            'gles3_version': r'(3.\d\s)',
            'comment': r'(#.*)'
        }

        cls.__re_require_header = re.compile(r'^\s*\[require\]'
                                             '\s*{comment}?$'.format(**common))
        cls.__re_end_require_block = re.compile(r'^\s*\['.format(*common))
        cls.__re_gl = re.compile(r'^\s*GL\s*{cmp}\s*{gl_version}\s*{comment}'
                                 '?$'.format(**common))
        cls.__re_gles2 = re.compile(r'^\s*GL ES\s*{cmp}\s*{gles2_version}'
                                    '\s*{comment}?$'.format(**common))
        cls.__re_gles3 = re.compile(r'^\s*GL ES\s*{cmp}\s*{gles3_version}'
                                    '\s*{comment}?$'.format(**common))
        cls.__re_gl_unknown = re.compile(r'^\s*GL\s*{cmp}'.format(**common))

    def __init__(self, shader_runner_args):
        Test.__init__(self, runConcurrent=True)

        assert(isinstance(shader_runner_args, list))
        assert(isinstance(shader_runner_args[0], str) or
               isinstance(shader_runner_args[0], unicode))

        self.__shader_runner_args = shader_runner_args
        self.__test_filepath = shader_runner_args[0]
        self.__result = None
        self.__command = None
        self.__gl_api = None

        self.env = {}

    def __report_failure(self, message):
        assert(self.__result is None)
        self.__result = TestResult()
        self.__result["result"] = "fail"
        self.__result["errors"] = [message]

    def __parse_test_file(self):
        self.__set_gl_api()

    def __set_gl_api(self):
        """Set self.__gl_api by parsing the test's requirement block.

        This function is idempotent."""

        if self.__gl_api is not None:
            return

        cls = self.__class__
        cls.__compile_regexes()

        PARSE_FIND_REQUIRE_HEADER = 0
        PARSE_FIND_GL_REQUIREMENT = 1

        parse_state = PARSE_FIND_REQUIRE_HEADER

        try:
            with open(self.__test_filepath) as f:
                for line in f:
                    if parse_state == PARSE_FIND_REQUIRE_HEADER:
                        if cls.__re_require_header.match(line) is not None:
                            parse_state = PARSE_FIND_GL_REQUIREMENT
                        else:
                            continue
                    elif parse_state == PARSE_FIND_GL_REQUIREMENT:
                        if cls.__re_gl.match(line) is not None:
                            self.__gl_api = ShaderTest.API_GL
                            return
                        elif cls.__re_gles2.match(line) is not None:
                            self.__gl_api = ShaderTest.API_GLES2
                            return
                        elif cls.__re_gles3.match(line) is not None:
                            self.__gl_api = ShaderTest.API_GLES3
                            return
                        elif cls.__re_gl_unknown.match(line) is not None:
                            self.__report_failure("Failed to parse GL "
                                                  "requirement: " + line)
                            self.__gl_api = ShaderTest.API_ERROR
                            return
                        elif cls.__re_end_require_block.match(line):
                            # Default to GL if no API is given.
                            self.__gl_api = ShaderTest.API_GL
                            return
                        else:
                            continue
                    else:
                        assert(False)

                if parse_state == PARSE_FIND_REQUIRE_HEADER or \
                   parse_state == PARSE_FIND_GL_REQUIREMENT:
                    # If no requirements are found, then assume the required
                    # API is GL. This matches the behavior of the
                    # shader_runner executable, whose default requirements are
                    # GL >= 1.0 and GLSL >= 1.10.
                    self.__gl_api = ShaderTest.API_GL
                else:
                    assert(False)

        except IOError:
            self.__report_failure("Failed to read test file "
                                  "{0!r}".format(self.__test_filepath))
            return

    @property
    def command(self):
        if self.__command is not None:
            return self.__command

        self.__set_gl_api()

        if self.__result is not None:
            assert(self.__result["result"] == "fail")
            return ["/bin/false"]

        if self.__gl_api == ShaderTest.API_GL:
            runner = "shader_runner"
        elif self.__gl_api == ShaderTest.API_GLES2:
            runner = "shader_runner_gles2"
        elif self.__gl_api == ShaderTest.API_GLES3:
            runner = "shader_runner_gles3"
        else:
            assert(False)

        runner = os.path.join(testBinDir, runner)
        self.__command = [runner] + self.__shader_runner_args
        return self.__command

    def run(self, env):
        """ Parse the test file's [require] block to determine which
        executable is needed to run the test. Then run the executable on the
        test file."""

        # Parse the test file to discover any errors.
        self.__parse_test_file()

        if self.__result is not None:
            # We've already decided the test result, most likely because
            # parsing the test file discovered an error.
            return self.__result

        return PlainExecTest.run(self, env)
