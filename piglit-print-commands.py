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


import argparse
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

def main():
	parser = argparse.ArgumentParser(sys.argv)

	parser.add_argument("-t", "--include-tests",
			default = [],
			action  = "append",
			metavar = "<regex>",
			help    = "Run only matching tests (can be used more than once)")
	parser.add_argument("--tests",
			default = [],
			action  = "append",
			metavar = "<regex>",
			help    = "Run only matching tests (can be used more than once)" \
					  "Deprecated")
	parser.add_argument("-x", "--exclude-tests",
			default = [],
			action  = "append",
			metavar = "<regex>",
			help    = "Exclude matching tests (can be used more than once)")
	parser.add_argument("testProfile",
			metavar = "<Path to testfile>",
			help    = "Path to results folder")

	args = parser.parse_args()

	env = core.Environment()

	# If --tests is called warn that it is deprecated
	if args.tests != []:
		print "Warning: Option --tests is deprecated. Use --include-tests"

	# Append includes and excludes to env
	for each in args.include_tests:
		env.filter.append(re.compile(each))
	for each in args.tests:
		env.filter.append(re.compile(each))
	for each in args.exclude_tests:
		env.exclude_filter.append(re.compile(each))

	# Change to the piglit's path
	piglit_dir = path.dirname(path.realpath(sys.argv[0]))
	os.chdir(piglit_dir)

	profile = core.loadTestProfile(args.testFile)

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
