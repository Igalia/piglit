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
from framework.threads import synchronized_self

#############################################################################
##### Main program
#############################################################################

def main():
	env = core.Environment()

	parser = argparse.ArgumentParser(sys.argv)


	# Either require that a name for the test is passed or that
	# resume is requested
	excGroup1 = parser.add_mutually_exclusive_group()
	excGroup1.add_argument("-n", "--name",
			metavar = "<test name>",
			default = None,
			help    = "Name of this test run")
	excGroup1.add_argument("-r", "--resume",
			action  = "store_true",
			help    = "Resume an interupted test run")

	parser.add_argument("-d", "--dry-run",
			action  = "store_true",
			help    = "Do not execute the tests")
	parser.add_argument("-t", "--include-tests",
			default = [],
			action  = "append",
			metavar = "<regex>",
			help    = "Run only matching tests (can be used more than once)")
	parser.add_argument("--tests",
			default = [],
			action  = "append",
			metavar = "<regex>",
			help    = "Run only matching tests (can be used more than once) " \
			          "DEPRECATED: use --include-tests instead")
	parser.add_argument("-x", "--exclude-tests",
			default = [],
			action  = "append",
			metavar = "<regex>",
			help    = "Exclude matching tests (can be used more than once)")

	# The new option going forward should be --no-concurrency, but to
	# maintain backwards compatability the --c, --concurrent option should
	# also be maintained. This code allows only one of the two options to be
	# supplied, or it throws an error
	excGroup2 = parser.add_mutually_exclusive_group()
	excGroup2.add_argument("--no-concurrency",
			action  = "store_true",
			help    = "Disable concurrent test runs")
	excGroup2.add_argument("-c", "--concurrent",
			action  = "store",
			metavar = "<boolean>",
			choices = ["1", "0", "on", "off"],
			help    = "Deprecated: Turn concrrent runs on or off")

	parser.add_argument("-p", "--platform",
			choices = ["glx", "x11_egl", "wayland", "gbm"],
			help    = "Name of windows system passed to waffle")
	parser.add_argument("--valgrind",
			action  =  "store_true",
			help    = "Run tests in valgrind's memcheck")
	parser.add_argument("testProfile",
			metavar = "<Path to test profile>",
			help    = "Path to testfile to run")
	parser.add_argument("resultsPath",
			metavar = "<Results Path>",
			help    = "Path to results folder")

	args = parser.parse_args()


	# Set the platform to pass to waffle
	if args.platform is not None:
		os.environ['PIGLIT_PLATFORM'] = args.platform

	# Set dry-run
	if args.dry_run is True:
		env.execute = False

	# Set valgrind
	if args.valgrind is True:
		env.valgrind = True

	# Turn concurency off if requested
	# Deprecated
	if args.concurrent is not None:
		if (args.concurrent == '1' or args.concurrent == 'on'):
			env.concurrent = True
			print "Warning: Option -c, --concurrent is deprecated, " \
					"concurrent test runs are on by default"
		elif (args.concurrent == '0' or args.concurrent == 'off'):
			env.concurrent = False
			print "Warning: Option -c, --concurrent is deprecated, " \
					"use --no-concurrency for non-concurrent test runs"
		# Ne need for else, since argparse restricts the arguments allowed

	# Not deprecated
	elif args.no_concurrency is True:
		env.concurrent = False

	# If the deprecated tests option was passed print a warning
	if args.tests != []:
		print "Warning: Option --tests is deprecated, use " \
				"--include-tests instead"

	# If resume is requested attempt to load the results file
	# in the specified path
	if args.resume is True:
		resultsDir = path.realpath(args.resultsPath)

		# Load settings from the old results JSON
		old_results = core.loadTestResults(resultsDir)
		profileFilename = old_results.options['profile']
		for value in old_results.options['filter']:
			test_filter.append(value)
			env.filter.append(re.compile(value))
		for value in old_results.options['exclude_filter']:
			exclude_filter.append(value)
			env.exclude_filter.append(re.compile(value))

	# Otherwise parse additional settings from the command line
	else:
		profileFilename = args.testProfile
		resultsDir = args.resultsPath

		# Set the excluded and included tests regex
		for each in args.include_tests:
			env.filter.append(re.compile(each))
		for each in args.tests:
			env.filter.append(re.compile(each))
		for each in args.exclude_tests:
			env.exclude_filter.append(re.compile(each))

	# Change working directory to the root of the piglit directory
	piglit_dir = path.dirname(path.realpath(sys.argv[0]))
	os.chdir(piglit_dir)

	core.checkDir(resultsDir, False)

	results = core.TestrunResult()

	# Set results.name
	if args.name is not None:
		results.name = args.name
	else:
		results.name = path.basename(resultsDir)

	# Begin json.
	result_filepath = os.path.join(resultsDir, 'main')
	result_file = open(result_filepath, 'w')
	json_writer = core.JSONWriter(result_file)
	json_writer.open_dict()

	# Write out command line options for use in resuming.
	json_writer.write_dict_key('options')
	json_writer.open_dict()
	json_writer.write_dict_item('profile', profileFilename)
	json_writer.write_dict_key('filter')
	result_file.write(json.dumps(args.include_tests))
	json_writer.write_dict_key('exclude_filter')
	result_file.write(json.dumps(args.exclude_tests))
	json_writer.close_dict()

	json_writer.write_dict_item('name', results.name)
	for (key, value) in env.collectData().items():
		json_writer.write_dict_item(key, value)

	profile = core.loadTestProfile(profileFilename)

	json_writer.write_dict_key('tests')
	json_writer.open_dict()
	# If resuming an interrupted test run, re-write all of the existing
	# results since we clobbered the results file.  Also, exclude them
	# from being run again.
	if args.resume is True:
		for (key, value) in old_results.tests.items():
			if os.path.sep != '/':
				key = key.replace(os.path.sep, '/', -1)
			json_writer.write_dict_item(key, value)
			env.exclude_tests.add(key)

	time_start = time.time()
	profile.run(env, json_writer)
	time_end = time.time()

	json_writer.close_dict()

	results.time_elapsed = time_end - time_start
	json_writer.write_dict_item('time_elapsed', results.time_elapsed)

	# End json.
	json_writer.close_dict()
	json_writer.file.close()

	print
	print 'Thank you for running Piglit!'
	print 'Results have been written to ' + result_filepath


if __name__ == "__main__":
	main()
