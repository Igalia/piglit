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
from framework.threads import synchronized_self

#############################################################################
##### Main program
#############################################################################
def usage():
	USAGE = """\
Usage: %(progName)s [options] [profile.tests] [results]
       %(progName)s [options] -r [results]

Options:
  -h, --help                Show this message
  -d, --dry-run             Do not execute the tests
  -t regexp, --tests=regexp Run only matching tests (can be used more
                            than once)
  -x regexp, --exclude-tests=regexp Excludey matching tests (can be used
                            more than once)
  -n name, --name=name      Name of the testrun
  -c bool, --concurrent=bool  Enable/disable concurrent test runs. Valid
			      option values are: 0, 1, on, off.  (default: on)
  --valgrind                Run tests in valgrind's memcheck.
  -p platform, --platform=platform  Name of the piglit platform to use.
Example:
  %(progName)s tests/all.tests results/all
         Run all tests, store the results in the directory results/all

  %(progName)s -t basic tests/all.tests results/all
         Run all tests whose path contains the word 'basic'

  %(progName)s -t ^glean/ -t tex tests/all.tests results/all
         Run all tests that are in the 'glean' group or whose path contains
		 the substring 'tex'

  %(progName)s -r -x bad-test results/all
         Resume an interrupted test run whose results are stored in the
         directory results/all, skipping bad-test.
"""
	print USAGE % {'progName': sys.argv[0]}
	sys.exit(1)

def main():
	env = core.Environment()

	try:
		option_list = [
			 "help",
			 "dry-run",
			 "resume",
			 "valgrind",
			 "tests=",
			 "name=",
			 "exclude-tests=",
			 "concurrent=",
			 "platform=",
			 ]
		options, args = getopt(sys.argv[1:], "hdrt:n:x:c:p:", option_list)
	except GetoptError:
		usage()

	OptionName = ''
	OptionResume = False
	test_filter = []
	exclude_filter = []
	platform = None

	for name, value in options:
		if name in ('-h', '--help'):
			usage()
		elif name in ('-d', '--dry-run'):
			env.execute = False
		elif name in ('-r', '--resume'):
			OptionResume = True
		elif name in ('--valgrind'):
			env.valgrind = True
		elif name in ('-t', '--tests'):
			test_filter.append(value)
			env.filter.append(re.compile(value))
		elif name in ('-x', '--exclude-tests'):
			exclude_filter.append(value)
			env.exclude_filter.append(re.compile(value))
		elif name in ('-n', '--name'):
			OptionName = value
		elif name in ('-c, --concurrent'):
			if value in ('1', 'on'):
				env.concurrent = True
			elif value in ('0', 'off'):
				env.concurrent = False
			else:
				usage()
		elif name in ('-p, --platform'):
			platform = value

	if platform is not None:
		os.environ['PIGLIT_PLATFORM'] = platform

	if OptionResume:
		if test_filter or OptionName:
			print "-r is not compatible with -t or -n."
			usage()
		if len(args) != 1:
			usage()
		resultsDir = args[0]

		# Load settings from the old results JSON
		old_results = core.loadTestResults(resultsDir)
		profileFilename = old_results.options['profile']
		for value in old_results.options['filter']:
			test_filter.append(value)
			env.filter.append(re.compile(value))
		for value in old_results.options['exclude_filter']:
			exclude_filter.append(value)
			env.exclude_filter.append(re.compile(value))
	else:
		if len(args) != 2:
			usage()

		profileFilename = args[0]
		resultsDir = path.realpath(args[1])

	# Change to the piglit's path
	piglit_dir = path.dirname(path.realpath(sys.argv[0]))
	os.chdir(piglit_dir)

	core.checkDir(resultsDir, False)

	results = core.TestrunResult()

	# Set results.name
	if OptionName is '':
		results.name = path.basename(resultsDir)
	else:
		results.name = OptionName

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
	result_file.write(json.dumps(test_filter))
	json_writer.write_dict_key('exclude_filter')
	result_file.write(json.dumps(exclude_filter))
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
	if OptionResume:
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
