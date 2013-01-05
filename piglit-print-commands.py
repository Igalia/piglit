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


from getopt import getopt, GetoptError
import os.path as path
import re
import sys, os
import time
import traceback
import json

sys.path.append(path.dirname(path.realpath(sys.argv[0])))
import framework.core as core
from framework.exectest import ExecTest
from framework.gleantest import GleanTest

#############################################################################
##### Main program
#############################################################################
def usage():
	USAGE = """\
Usage: %(progName)s [options] [profile.tests]

Prints a list of all the tests and how to run them.  Ex:
   piglit test name ::: /path/to/piglit/bin/program <args>
   glean test name ::: PIGLIT_TEST='...' /path/to/piglit/bin/glean -v -v -v ...

Options:
  -h, --help                Show this message
  -t regexp, --tests=regexp Run only matching tests (can be used more
                            than once)
  -x regexp, --exclude-tests=regexp Exclude matching tests (can be used
                            more than once)
Example:
  %(progName)s tests/all.tests
  %(progName)s -t basic tests/all.tests
         Print tests whose path contains the word 'basic'

  %(progName)s -t ^glean/ -t tex tests/all.tests
         Print tests that are in the 'glean' group or whose path contains
         the substring 'tex'
"""
	print USAGE % {'progName': sys.argv[0]}
	sys.exit(1)

def main():
	env = core.Environment()

	try:
		option_list = [
			 "help",
			 "tests=",
			 "exclude-tests=",
			 ]
		options, args = getopt(sys.argv[1:], "ht:x:", option_list)
	except GetoptError:
		usage()

	OptionName = ''
	OptionResume = False
	test_filter = []
	exclude_filter = []

	for name, value in options:
		if name in ('-h', '--help'):
			usage()
		elif name in ('-t', '--tests'):
			test_filter.append(value)
			env.filter.append(re.compile(value))
		elif name in ('-x', '--exclude-tests'):
			exclude_filter.append(value)
			env.exclude_filter.append(re.compile(value))

	if len(args) != 1:
		usage()

	profileFilename = args[0]

	# Change to the piglit's path
	piglit_dir = path.dirname(path.realpath(sys.argv[0]))
	os.chdir(piglit_dir)

	profile = core.loadTestProfile(profileFilename)

	# If resuming an interrupted test run, re-write all of the existing
	# results since we clobbered the results file.  Also, exclude them
	# from being run again.
	if OptionResume:
		for (key, value) in old_results.tests.items():
			json_writer.write_dict_item(key, value)
			env.exclude_tests.add(key)

	def getCommand(test):
		command = ''
		if isinstance(test, GleanTest):
			for var, val in test.env.items():
				command += var + "='" + val + "' "
		command += ' '.join(test.command)
		return command

	profile.prepare_test_list(env)
	for name, test in profile.test_list.items():
		assert(isinstance(test, ExecTest))
		print name, ':::', getCommand(test)

if __name__ == "__main__":
	main()
