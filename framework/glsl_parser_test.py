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

""" This module enables the running of GLSL parser tests. """

import ConfigParser
import os
import os.path as path
import re
from cStringIO import StringIO

from .exectest import PiglitTest


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
                if ext in ['vert', 'tesc', 'tese', 'geom', 'frag', 'comp']:
                    filepath = path.join(dirpath, f)
                    # testname := filepath relative to
                    # basepath.
                    testname = os.path.relpath(filepath, basepath)
                    if os.path.sep != '/':
                        testname = testname.replace(os.path.sep, '/', -1)
                    assert isinstance(testname, basestring)
                    add_glsl_parser_test(group, filepath, testname)


class GLSLParserTest(PiglitTest):
    """ Read the options in a glsl parser test and create a Test object

    Specifically it is necessary to parse a glsl_parser_test to get information
    about it before actually creating a PiglitTest. Even though this could
    be done with a funciton wrapper, making it a distinct class makes it easier
    to sort in the profile.

    Arguments:
    filepath -- the path to a glsl_parser_test which must end in .vert,
                .tesc, .tese, .geom or .frag

    """
    def __init__(self, filepath):
        os.stat(filepath)

        # Parse the config file and get the config section, then write this
        # section to a StringIO and pass that to ConfigParser
        with open(filepath, 'r') as testfile:
            text_io = self.__parser(testfile, filepath)

            config = ConfigParser.SafeConfigParser(
                defaults={'require_extensions': '', 'check_link': 'false'})

            # Verify that the config was valid
            text = text_io.getvalue()
            text_io.close()
            config.readfp(StringIO(text))

            for opt in ['expect_result', 'glsl_version']:
                if not config.has_option('config', opt):
                    raise GLSLParserException("Missing required section {} "
                                              "from config".format(opt))

            # Create the command and pass it into a PiglitTest()
            command = ['glslparsertest',
                       filepath,
                       config.get('config', 'expect_result'),
                       config.get('config', 'glsl_version')]
            if config.get('config', 'check_link').lower() == 'true':
                command.append('--check-link')
            command.extend(config.get('config', 'require_extensions').split())

            super(GLSLParserTest, self).__init__(command, run_concurrent=True)

    def __parser(self, testfile, filepath):
        """ Private helper that parses the config file

        This method parses the lines of text file, and then returns a
        StrinIO instance suitable to be parsed by a configparser class.
        
        It will raise GLSLParserExceptions if any part of the parsing
        fails.

        """
        # Text of config section.
        text_io = StringIO()
        text_io.write('[config]\n')

        # Create a generator that iterates over the lines in the test file.
        # This allows us to run the loop until we find the header, stop and
        # then run again looking for the config sections.
        # This reduces the need for if statements substantially
        lines = (l for l in testfile)

        is_header = re.compile(r'\s*(//|/\*|\*)\s*\[config\]')
        for line in lines:
            if is_header.match(line):
                break
        else:
            raise GLSLParserException("No [config] section found!")

        is_header = re.compile(r'\s*(//|/\*|\*)\s*\[end config\]')
        for line in lines:
            # Remove all leading whitespace
            line = line.strip()

            # If strip renendered '' that means we had a blank newline,
            # just go on
            if line == '':
                continue
            # If we get to the end of the config break
            elif is_header.match(line):
                break
            # If the starting character is a two character comment
            # remove that and any newly revealed whitespace, then write
            # it into the StringIO
            elif line[:2] in ['//', '/*', '*/']:
                text_io.write(line[2:].lstrip() + '\n')
            # If we have just * then we're in the middle of a C style
            # comment, do like above
            elif line[:1] == '*':
                text_io.write(line[1:].lstrip() + '\n')
            else:
                raise GLSLParserException(
                    "The config section is malformed."
                    "Check file {0}".format(filepath))
        else:
            raise GLSLParserException("No [end config] section found!")

        return text_io


class GLSLParserException(Exception):
    pass
