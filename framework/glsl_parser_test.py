#!/usr/bin/env python
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

"""This module enables the running of GLSL parser tests.

This module can be used to add parser tests to a Piglit test group or to run
standalone tests on the command line. To add a test to a Piglit group, us
``add_glsl_parser_test()``. To run a single standalone test, execute
``glsl_parser_test.py TEST_FILE``.
"""

usage_message = "usage: glsl_parser_test.py TEST_FILE"

import ConfigParser
import os
import os.path as path
import re
import subprocess
import sys

from ConfigParser import SafeConfigParser
from core import Test, testBinDir, TestResult
from cStringIO import StringIO
from exectest import PlainExecTest

def add_glsl_parser_test(group, filepath, test_name):
    """Add an instance of GLSLParserTest to the given group."""
    group[test_name] = GLSLParserTest(filepath)

def import_glsl_parser_tests(group, basepath, subdirectories):
    """
    Recursively register each shader source file in the given
    ``subdirectories`` as a GLSLParserTest .

    :subdirectories: A list of subdirectories under the basepath.

    The name with which each test is registered into the given group is
    the shader source file's path relative to ``basepath``. For example,
    if::
            import_glsl_parser_tests(group, 'a', ['b1', 'b2'])
    is called and the file 'a/b1/c/d.frag' exists, then the test is
    registered into the group as ``group['b1/c/d.frag']``.
    """
    for d in subdirectories:
        walk_dir = path.join(basepath, d)
        for (dirpath, dirnames, filenames) in os.walk(walk_dir):
            # Ignore dirnames.
            for f in filenames:
                # Add f as a test if its file extension is good.
                ext = f.rsplit('.')[-1]
                if ext in ['vert', 'geom', 'frag']:
                    filepath = path.join(dirpath, f)
                    # testname := filepath relative to
                    # basepath.
                    testname = os.path.relpath(
                            filepath, basepath)
                    if os.path.sep != '/':
                        testname = testname.replace(os.path.sep, '/', -1)
                    assert isinstance(testname, basestring)
                    add_glsl_parser_test(
                            group,
                            filepath,
                            testname)

class GLSLParserTest(PlainExecTest):
    """Test for the GLSL parser (and more) on a GLSL source file.

    This test takes a GLSL source file and passes it to the executable
    ``glslparsertest``. The GLSL source file being tested must have a GLSL
    file extension: one of ``.vert``, ``.geom``, or ``.frag``. The test file
    must have a properly formatted comment section containing configuration
    data (see below).

    For example test files, see the directory
    'piglit.repo/examples/glsl_parser_text`.

    Quirks
    ------
    It is not completely corect to state that this is a test for the GLSL
    parser, because it also tests later compilation stages, such as AST
    construction and static type checking. Specifically, this tests the
    success of the executable ``glslparsertest``, which in turn tests the
    success of the native function ``glCompileShader()``.

    Config Section
    --------------
    The GLSL source file must contain a special config section in its
    comments. This section can appear anywhere in the file: above,
    within, or below the actual GLSL source code. The syntax of the config
    section is essentially the syntax of
    ``ConfigParser.SafeConfigParser``.

    The beginning of the config section is marked by a comment line that
    contains only '[config]'. The end of the config section is marked by
    a comment line that contains only '[end config]'. All intervening
    lines become the text of the config section.

    Whitespace is significant, because ``ConfigParser`` treats it so.  The
    config text of each non-empty line begins on the same column as the
    ``[`` in the ``[config]`` line.  (A line is considered empty if it
    contains whitespace and an optional C comment marker: ``//``, ``*``,
    ``/*``). Therefore, option names must be aligned on this column. Text
    that begins to the right of this column is considered to be a line
    continuation.


    Required Options
    ----------------
    * glsl_version: A valid GLSL version number, such as 1.10.
    * expect_result: Either ``pass`` or ``fail``.

    Nonrequired Options
    -------------------
    * require_extensions: List of GL extensions. If an extension is not
          supported, the test is skipped. Each extension name must begin
          with GL and elements are separated by whitespace.
    * check_link: Either ``true`` or ``false``.  A true value passes the
          --check-link option to be supplied to glslparsertest, which
          causes it to detect link failures as well as compilation
          failures.

    Examples
    --------
    ::
            // [config]
            // glsl_version: 1.30
            // expect_result: pass
            // # Lists may be single-line.
            // require_extensions: GL_ARB_fragment_coord_conventions GL_AMD_conservative_depth
            // [end config]

    ::
            /* [config]
             * glsl_version: 1.30
             * expect_result: pass
             * # Lists may be span multiple lines.
             * required_extensions:
             *     GL_ARB_fragment_coord_conventions
             *     GL_AMD_conservative_depth
             * [end config]
             */

    ::
            /*
            [config]
            glsl_version: 1.30
            expect_result: pass
            check_link: true
            [end config]
            */

    An incorrect example, where text is not properly aligned::
            /* [config]
            glsl_version: 1.30
            expect_result: pass
            [end config]
            */

    Another alignment problem::
            // [config]
            //   glsl_version: 1.30
            //   expect_result: pass
            // [end config]
    """

    __required_opts = [
            'expect_result',
            'glsl_version'
            ]

    __config_defaults = {
            'require_extensions' : '',
            'check_link' : 'false',
            }

    def __init__(self, filepath, runConcurrent = True):
        """
        :filepath: Must end in one '.vert', '.geom', or '.frag'.
        """
        Test.__init__(self, runConcurrent)
        self.__config = None
        self.__command = None
        self.__filepath = filepath
        self.result = None

    def __get_config(self):
        """Extract the config section from the test file.

        Set ``self.__cached_config``.  If the config section is missing
        or invalid, or any other errors occur, then set ``self.result``
        to failure.

        :return: None
        """

        cls = self.__class__

        # Text of config section.
        text_io = StringIO()

        # Parsing state.
        PARSE_FIND_START = 0
        PARSE_IN_CONFIG = 1
        PARSE_DONE = 2
        PARSE_ERROR = 3
        parse_state = PARSE_FIND_START

        # Regexen that change parser state.
        start = re.compile(r'\A(?P<indent>\s*(|//|/\*|\*)\s*)(?P<content>\[config\]\s*\n)\Z')
        empty = None # Empty line in config body.
        internal = None # Non-empty line in config body.
        end = None # Marks end of config body.

        try:
            f = open(self.__filepath, 'r')
        except IOError:
            self.result = TestResult()
            self.result['result'] = 'fail'
            self.result['errors'] = ["Failed to open test file '{0}'".format(self.__filepath)]
            return
        for line in f:
            if parse_state == PARSE_FIND_START:
                m = start.match(line)
                if m is not None:
                    parse_state = PARSE_IN_CONFIG
                    text_io.write(m.group('content'))
                    indent = '.' * len(m.group('indent'))
                    empty = re.compile(r'\A\s*(|//|/\*|\*)\s*\n\Z')
                    internal = re.compile(r'\A{indent}(?P<content>.*\n)\Z'.format(indent=indent))
                    end = re.compile(r'\A{indent}\[end( |_)config\]\s*\n\Z'.format(indent=indent))
            elif parse_state == PARSE_IN_CONFIG:
                if start.match(line) is not None:
                    parse_state = PARSE_ERROR
                    break
                if end.match(line) is not None:
                    parse_state = PARSE_DONE
                    break
                m = internal.match(line)
                if m is not None:
                    text_io.write(m.group('content'))
                    continue
                m = empty.match(line)
                if m is not None:
                    text_io.write('\n')
                    continue
                parse_state = PARSE_ERROR
                break
            else:
                assert(False)

        if parse_state == PARSE_DONE:
            pass
        elif parse_state == PARSE_FIND_START:
            self.result = TestResult()
            self.result['result'] = 'fail'
            self.result['errors'] = ["Config section of test file '{0}' is missing".format(self.__filepath)]
            self.result['errors'] += ["Failed to find initial line of config section '// [config]'"]
            self.result['note'] = "See the docstring in file '{0}'".format(__file__)
            return
        elif parse_state == PARSE_IN_CONFIG:
            self.result = TestResult()
            self.result['result'] = 'fail'
            self.result['errors'] = ["Config section of test file '{0}' does not terminate".format(self.__filepath)]
            self.result['errors'] += ["Failed to find terminal line of config section '// [end config]'"]
            self.result['note'] = "See the docstring in file '{0}'".format(__file__)
            return
        elif parse_state == PARSE_ERROR:
            self.result = TestResult()
            self.result['result'] = 'fail'
            self.result['errors'] = ["Config section of test file '{0}' is ill formed, most likely due to whitespace".format(self.__filepath)]
            self.result['note'] = "See the docstring in file '{0}'".format(__file__)
            return
        else:
            assert(False)

        config = ConfigParser.SafeConfigParser(cls.__config_defaults)
        try:
            text = text_io.getvalue()
            text_io.close()
            config.readfp(StringIO(text))
        except ConfigParser.Error as e:
            self.result = TestResult()
            self.result['result'] = 'fail'
            self.result['errors'] = ['Errors exist in config section of test file']
            self.result['errors'] += [e.message]
            self.result['note'] = "See the docstring in file '{0}'".format(__file__)
            return

        self.__config = config

    def __validate_config(self):
        """Validate config.

        Check that that all required options are present. If
        validation fails, set ``self.result`` to failure.

        Currently, this function does not validate the options'
        values.

        :return: None
        """
        cls = self.__class__

        if self.__config is None:
            return
        for o in cls.__required_opts:
            if not self.__config.has_option('config', o):
                self.result = TestResult()
                self.result['result'] = 'fail'
                self.result['errors'] = ['Errors exist in config section of test file']
                self.result['errors'] += ["Option '{0}' is required".format(o)]
                self.result['note'] = "See the docstring in file '{0}'".format(__file__)
                return

    def run_standalone(self):
        """Run the test as a standalone process outside of Piglit."""
        if self.result is not None:
            sys.stdout.write(self.result)
            sys.exit(1)

        assert(self.command is not None)
        env = os.environ.copy()
        for e in self.env:
            env[e] = str(self.env[e])
        p = subprocess.Popen(self.command, env=env)
        p.communicate()

    @property
    def config(self):
        if self.__config is None:
            self.__get_config()
            self.__validate_config()
        return self.__config

    @property
    def command(self):
        """Command line arguments for 'glslparsertest'.

        The command line arguments are constructed by parsing the
        config section of the test file. If any errors are present in
        the config section, then ``self.result`` is set to failure and
        this returns ``None``.

        :return: [str] or None
        """

        if self.result is not None:
            return None

        assert(self.config is not None)
        command = [
                path.join(testBinDir, 'glslparsertest'),
                self.__filepath,
                self.config.get('config', 'expect_result'),
                self.config.get('config', 'glsl_version')
                ]
        if self.config.get('config', 'check_link').lower() == 'true':
            command.append('--check-link')
        command += self.config.get('config', 'require_extensions').split()
        return command

    @property
    def env(self):
        return dict()

if __name__ == '__main__':
    if len(sys.argv) != 2:
        sys.stderr.write("{0}: usage error\n\n".format(sys.argv[0]))
        sys.stderr.write(usage_message)
    test_file = sys.argv[1]
    test = GLSLParserTest(test_file)
    test.run_standalone()
