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

usage_message = "uasge: glsl_parser_test.py TEST_FILE"

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

def import_glsl_parser_tests(group, filepath, subdirectories):
	# Register each shader source file in the directories below as
	# a GLSLParserTest.
	for d in subdirectories:
		walk_dir = path.join(filepath, d)
		for (dirpath, dirnames, filenames) in os.walk(walk_dir):
			# Ignore dirnames.
			for f in filenames:
				# Add f as a test if its file extension is good.
				ext = f.rsplit('.')[-1]
				if ext in ['vert', 'geom', 'frag']:
					filepath = path.join(dirpath, f)
					# testname := filepath with initial
					#     'tests/spec/glsl-1.30' removed
					sep = os.sep
					testname = sep.join(filepath.split(sep)[3:])
					assert(type(testname) is str)
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
	* Additional options are currenly ignored.

	Examples
	--------
	::
		// [config]
		// # Comments have this form
		// glsl_version: 1.30
		// expect_result: pass
		// [end config]

	::
		/* [config]
		 * glsl_version: 1.30
		 * expect_result: pass
		 * [end config]
		 */

	::
		/*
		[config]
		glsl_version: 1.30
		expect_result: pass
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

	def __init__(self, filepath):
		"""
		:filepath: Must end in one '.vert', '.geom', or '.frag'.
		"""
		Test.__init__(self)
		self.filepath = filepath
		self.env = dict()
		self._cached_command = None

	def _get_config(self):
		"""Extract the config section from the test file.

		If there are no parsing errors in extracting the config, then
		return a tuple ``(config, None)``, where ``config`` is
		a ``SafeConfigPaser``. If the config section is missing or ill
		formed, or any other errors occur, then return ``(None,
		result)``, where ``result`` is a ``TestResult`` that reports
		failure.

		:return: (SafeConfigParser, None) or (None, TestResult)
		"""

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
			f = open(self.filepath, 'r')
		except IOError:
			result = TestResult()
			result['result'] = 'fail'
			result['errors'] = ["Failed to open test file '{0}'".format(self.filepath)]
			return (None, result)
		for line in f:
			if parse_state == PARSE_FIND_START:
				m = start.match(line)
				if m is not None:
					parse_state = PARSE_IN_CONFIG
					text_io.write(m.group('content'))
					indent = '.' * len(m.group('indent'))
					empty = re.compile(r'\A\s*(|//|/\*|\*)\s*\n\Z')
					internal = re.compile(r'\A{indent}(?P<content>.*\n)\Z'.format(indent=indent))
					end = re.compile(r'\A{indent}\[end config\]\s*\n\Z'.format(indent=indent))
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
			result = TestResult()
			result['result'] = 'fail'
			result['errors'] = ["Config section of test file '{0}' is missing".format(self.filepath)]
			result['errors'] += ["Failed to find initial line of config section '// [config]'"]
			result['note'] = "See the docstring in file '{0}'".format(__file__)
			return (None, result)
		elif parse_state == PARSE_IN_CONFIG:
			result = TestResult()
			result['result'] = 'fail'
			result['errors'] = ["Config section of test file '{0}' does not terminate".format(self.filepath)]
			result['errors'] += ["Failed to find terminal line of config section '// [end config]'"]
			result['note'] = "See the docstring in file '{0}'".format(__file__)
			return (None, result)
		elif parse_state == PARSE_ERROR:
			result = TestResult()
			result['result'] = 'fail'
			result['errors'] = ["Config section of test file '{0}' is ill formed, most likely due to whitespace".format(self.filepath)]
			result['note'] = "See the docstring in file '{0}'".format(__file__)
			return (None, result)
		else:
			assert(False)

		config = ConfigParser.SafeConfigParser()
		try:
			text = text_io.getvalue()
			text_io.close()
			config.readfp(StringIO(text))
		except ConfigParser.Error as e:
			result = TestResult()
			result['result'] = 'fail'
			result['errors'] = ['Errors exist in config section of test file']
			result['errors'] += [e.message]
			result['note'] = "See the docstring in file '{0}'".format(__file__)
			return (None, result)

		return (config, None)

	def _validate_config(self, config):
		"""Validate config.

		Check that that all required options are present. If validation
		succeeds, return ``None``. Else, return a ``TestResult`` that
		reports failure. (Currently, this function does not validate the
		options' values.)

		Required Options: expect_result, glsl_version

		:return: TestResult or None
		"""
		# Check that all required options are present.
		required_opts = ['expect_result', 'glsl_version']
		for o in required_opts:
			if not config.has_option('config', o):
				result = TestResult()
				result['result'] = 'fail'
				result['errors'] = ['Errors exist in config section of test file']
				result['errors'] += ["Option '{0}' is required".format(o)]
				result['note'] = "See the docstring in file '{0}'".format(__file__)
				return result
		return None

	def _make_command(self, config):
		"""Construct command line arguments for glslparsertest.

		If construction is successful, return ``(command, None)``. Else,
		return ``(None, result)`` where ``result`` is a ``TestResult``
		that reports failure.

		:precondition: config has already been validated
		:return: ([str], None) or (None, TestResult)
		"""

		command = [
			path.join(testBinDir, 'glslparsertest'),
			self.filepath,
			config.get('config', 'expect_result'),
			config.get('config', 'glsl_version'),
			]
		# There is no case in which the function returns (None, result).
		# However, the function may have a future need to do so.
		return (command, None)

	def run_standalone(self):
		"""Run the test as a standalone process outside of Piglit."""
		command = self.command
		if command is None:
			assert(self.result is not None)
			sys.stderr.write(repr(self.result))
			sys.exit(1)
		p = subprocess.Popen(command)
		p.communicate()

	@property
	def command(self):
		"""Command line arguments for 'glslparsertest'.

		The command line arguments are constructed by parsing the
		config section of the test file. If any errors occur, return
		None and set ``self.result`` to a ``TestResult`` that reports
		failure.

		:return: [str] or None
		"""
		if self._cached_command is None:
			(config, self.result) = self._get_config()
			if self.result is not None: return None
			self.result = self._validate_config(config)
			if self.result is not None: return None
			self.result = self._validate_config(config)
			if self.result is not None: return None
			(self._cached_command, self.result) = \
					self._make_command(config)
		return self._cached_command

if __name__ == '__main__':
	if len(sys.argv) != 2:
		sys.stderr.write("{0}: usage error\n\n".format(sys.argv[0]))
		sys.stderr.write(usage_message)
	test_file = sys.argv[1]
	test = GLSLParserTest(test_file)
	test.run_standalone()

# vim: noet ts=8 sw=8:
