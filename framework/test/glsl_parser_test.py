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

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os
import sys
import re
import io
import six

from framework import exceptions
from .base import TestIsSkip
from .opengl import FastSkipMixin
from .piglit_test import PiglitBaseTest, TEST_BIN_DIR, ROOT_DIR

__all__ = [
    'GLSLParserTest',
    'GLSLParserNoConfigError',
]

# In different configurations piglit may have one or both of these.
if sys.platform == 'win32':
    _HAS_GL_BIN = os.path.exists(os.path.join(TEST_BIN_DIR, 'glslparsertest.exe'))
    _HAS_GLES_BIN = os.path.exists(os.path.join(TEST_BIN_DIR, 'glslparsertest_gles2.exe'))
else:
    _HAS_GL_BIN = os.path.exists(os.path.join(TEST_BIN_DIR, 'glslparsertest'))
    _HAS_GLES_BIN = os.path.exists(os.path.join(TEST_BIN_DIR, 'glslparsertest_gles2'))

# This forces testing with compatibility extensions, even when GLES support is
# built
_FORCE_DESKTOP_VERSION = os.environ.get('PIGLIT_FORCE_GLSLPARSER_DESKTOP', False)


def _is_gles_version(version):
    """Return True if version is es, otherwsie false."""
    assert not isinstance(version, six.binary_type), \
        '{}({})'.format(version, type(version))

    if isinstance(version, six.text_type):
        # GLES 3+ versions should have "es" appended, even though
        # glslparsertest doesn't require them. If the version ends in "es" then
        # it is a GLES test for sure.
        if version.endswith('es'):
            return True

        if version.endswith('compatibility'):
            return False

        version = float(version)

    return version in [1.0, 3.0, 3.1, 3.2]


class GLSLParserNoConfigError(exceptions.PiglitInternalError):
    pass


class GLSLParserInternalError(exceptions.PiglitInternalError):
    pass


class Parser(object):
    """Find and parse the config block of a GLSLParserTest.

    Specifically it is necessary to parse a glsl_parser_test to get information
    about it before actually creating a PiglitTest. Even though this could
    be done with a function wrapper, making it a distinct class makes it easier
    to sort in the profile.
    """
    _CONFIG_KEYS = frozenset(['expect_result', 'glsl_version',
                              'require_extensions', 'check_link'])

    def __init__(self, filepath, installpath=None):
        # a set that stores a list of keys that have been found already
        self.__found_keys = set()
        self.gl_required = set()
        self.glsl_es_version = None
        self.glsl_version = None
        abs_filepath = os.path.join(ROOT_DIR, filepath)

        try:
            with io.open(abs_filepath, mode='r', encoding='utf-8') as testfile:
                testfile = testfile.read()
                self.config = self.parse(testfile, abs_filepath)
            self.command = self.get_command(filepath, installpath)
        except GLSLParserInternalError as e:
            raise exceptions.PiglitFatalError(
                'In file "{}":\n{}'.format(filepath, six.text_type(e)))

        self.set_skip_conditions()

    def set_skip_conditions(self):
        """Set OpenGL and OpenGL ES fast skipping conditions."""
        glsl = self.config['glsl_version']
        if _is_gles_version(glsl):
            self.glsl_es_version = float(glsl[:3])
        elif glsl.endswith('compatibility'):
            self.glsl_version = float(glsl[:3])
        else:
            self.glsl_version = float(glsl)

        req = self.config['require_extensions']
        self.gl_required = set(r for r in req.split() if not r.startswith('!'))

        # If GLES is requested, but piglit was not built with a gles version,
        # then ARB_ES3<ver>_compatibility is required. Add it to
        # self.gl_required
        if self.glsl_es_version and _FORCE_DESKTOP_VERSION:
            if self.glsl_es_version == 1.0:
                ver = '2'
            elif self.glsl_es_version == 3.0:
                ver = '3'
            elif self.glsl_es_version == 3.1:
                ver = '3_1'
            elif self.glsl_es_version == 3.2:
                ver = '3_2'
            ext = 'ARB_ES{}_compatibility'.format(ver)
            self.gl_required.add(ext)
            self.command.append(ext)

    @staticmethod
    def pick_binary(version):
        """Pick the correct version of glslparsertest to use.

        This will try to select glslparsertest_gles2 for OpenGL ES tests, and
        glslparsertest for desktop OpenGL tests. However, sometimes this isn't
        possible. In that case all tests will be assigned to the desktop
        version.

        If the test requires desktop OpenGL, but only OpenGL ES is available,
        then the test will be skipped in the python layer.

        """
        if _is_gles_version(version) and not _FORCE_DESKTOP_VERSION:
            return 'glslparsertest_gles2'
        else:
            return 'glslparsertest'

    def get_command(self, filepath, installpath):
        """ Create the command argument to pass to super()

        This private helper creates a configparser object, then reads in the
        provided config (from self.__parser), and tests for required options
        that must be provided. If it does not find them it raises an exception.
        It then crafts a command which is returned, and ultimately passed to
        super()

        """
        for opt in ['expect_result', 'glsl_version']:
            if not self.config.get(opt):
                raise GLSLParserInternalError("Missing required section {} "
                                              "from config".format(opt))

        # Create the command and pass it into a PiglitTest()
        glsl = self.config['glsl_version']
        command = [
            self.pick_binary(glsl),
            installpath or filepath,
            self.config['expect_result'],
            self.config['glsl_version']
        ]

        if self.config['check_link'].lower() == 'true':
            command.append('--check-link')
        command.extend(self.config['require_extensions'].split())

        return command

    def parse(self, testfile, filepath):
        """ Private helper that parses the config file

        This method parses the lines of text file, and then returns a
        StrinIO instance suitable to be parsed by a configparser class.

        It will raise GLSLParserInternalError if any part of the parsing
        fails.

        """
        keys = {'require_extensions': '', 'check_link': 'false'}

        # Text of config section.
        # Create a generator that iterates over the lines in the test file.
        # This allows us to run the loop until we find the header, stop and
        # then run again looking for the config sections.
        # This reduces the need for if statements substantially
        lines = (l.strip() for l in testfile.split('\n'))

        is_header = re.compile(r'(//|/\*|\*)\s*\[config\]')
        for line in lines:
            if is_header.match(line):
                break
        else:
            raise GLSLParserNoConfigError("No [config] section found!")

        is_header = re.compile(r'(//|/\*|\*)\s*\[end config\]')
        is_metadata = re.compile(
            r'(//|/\*|\*)\s*(?P<key>[a-z_]*)\:\s(?P<value>.*)')
        bad_values = re.compile(r'(?![\w\.\! ]).*')

        for line in lines:
            # If strip renendered '' that means we had a blank newline,
            # just go on
            if line in ['', '//', '*']:
                continue
            # If we get to the end of the config break
            elif is_header.match(line):
                break

            match = is_metadata.match(line)
            if match:
                if match.group('key') not in self._CONFIG_KEYS:
                    raise GLSLParserInternalError(
                        "Key {} is not a valid key for a "
                        "glslparser test config block".format(
                            match.group('key')))
                elif match.group('key') in self.__found_keys:
                    # If this key has already been encountered throw an error,
                    # there are no duplicate keys allows
                    raise GLSLParserInternalError(
                        'Duplicate entry for key {}'.format(
                            match.group('key')))
                else:
                    bad = bad_values.search(match.group('value'))
                    # XXX: this always seems to return a match object, even
                    # when the match is ''
                    if bad.group():
                        raise GLSLParserInternalError(
                            'Bad character "{}" at line: "{}". '
                            'Only alphanumerics, _, and space '
                            'are allowed'.format(
                                bad.group()[0], line))

                    # Otherwise add the key to the set of found keys, and add
                    # it to the dictionary that will be returned
                    self.__found_keys.add(match.group('key'))
                    keys[match.group('key')] = match.group('value')
            else:
                raise GLSLParserInternalError(
                    "The config section is malformed."
                    "Check file {0} for line {1}".format(filepath, line))
        else:
            raise GLSLParserInternalError("No [end config] section found!")

        return keys


class GLSLParserTest(FastSkipMixin, PiglitBaseTest):
    """A Test derived class specifically for glslparser.

    Arguments:
    filepath -- the path to a glsl_parser_test which must end in .vert,
                .tesc, .tese, .geom or .frag
    """

    def __init__(self, command, gl_required=set(), glsl_version=None,
                 glsl_es_version=None, **kwargs):
        super(GLSLParserTest, self).__init__(
            command, run_concurrent=True, gl_required=gl_required,
            glsl_version=glsl_version, glsl_es_version=glsl_es_version)

    @PiglitBaseTest.command.getter
    def command(self):
        command = super(GLSLParserTest, self).command
        glslfile = os.path.join(ROOT_DIR, command[1])
        return [command[0], glslfile] + command[2:]

    @classmethod
    def new(cls, filepath, installpath=None):
        """Parse a file and create an instance.

        :param str filepath: the file to parse
        :param Optional[str] installpath:
            The relative path the file will be isntalled to if different than
            filepath
        """
        parsed = Parser(filepath, installpath)
        return cls(
            parsed.command,
            gl_required=parsed.gl_required,
            glsl_version=parsed.glsl_version,
            glsl_es_version=parsed.glsl_es_version)

    def is_skip(self):
        if os.path.basename(self.command[0]) == 'glslparsertest' and not _HAS_GL_BIN:
            raise TestIsSkip('Test is for desktop OpenGL, but piglit was not '
                             'built with OpenGL support.')
        elif (os.path.basename(self.command[0]) == 'glslparsertest_gles2'
              and not _HAS_GLES_BIN):
            raise TestIsSkip('Test is for OpenGL ES, but piglit was not '
                             'built with OpenGL ES support.')

        super(GLSLParserTest, self).is_skip()
