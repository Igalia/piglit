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

import framework.core as core
from framework.threads import synchronized_self

class SyncFileWriter:
	'''
		Using the 'print' syntax to write to an instance of this class
		may have unexpected results in a multithreaded program.  For example:
			print >> file, "a", "b", "c"
		will call write() to write "a", then call write() to write "b", and so on...
		This type of execution allows for another thread to call write() before
		the original statement completes its execution.
		To avoid this behavior, call file.write() explicitly.  For example:
			file.write("a", "b", "c", "\n")
		will ensure that "a b c" gets written to the file before any other thread
		is given write access.
	'''
	def __init__(self, filename):
		self.file = open(filename, 'w')

	@synchronized_self
	def write(self, *args):
		[self.file.write(str(a)) for a in args]
		self.file.flush()
		os.fsync(self.file.fileno())

	@synchronized_self
	def writeLine(self, *args):
		self.write(*args)
		self.write('\n')

	@synchronized_self
	def close(self):
		self.file.close()

#############################################################################
##### Main program
#############################################################################
def usage():
	USAGE = """\
Usage: %(progName)s [options] [profile.tests] [results]

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
Example:
  %(progName)s tests/all.tests results/all
         Run all tests, store the results in the directory results/all

  %(progName)s -t basic tests/all.tests results/all
         Run all tests whose path contains the word 'basic'

  %(progName)s -t ^glean/ -t tex tests/all.tests results/all
         Run all tests that are in the 'glean' group or whose path contains
		 the substring 'tex'
"""
	print USAGE % {'progName': sys.argv[0]}
	sys.exit(1)

def main():
	env = core.Environment()

	try:
		option_list = [
			 "help",
			 "dry-run",
			 "tests=",
			 "name=",
			 "exclude-tests=",
			 "concurrent=",
			 ]
		options, args = getopt(sys.argv[1:], "hdt:n:x:c:", option_list)
	except GetoptError:
		usage()

	OptionName = ''

	for name, value in options:
		if name in ('-h', '--help'):
			usage()
		elif name in ('-d', '--dry-run'):
			env.execute = False
		elif name in ('-t', '--tests'):
			env.filter[:0] = [re.compile(value)]
		elif name in ('-x', '--exclude-tests'):
			env.exclude_filter[:0] = [re.compile(value)]
		elif name in ('-n', '--name'):
			OptionName = value
		elif name in ('-c, --concurrent'):
			if value in ('1', 'on'):
				env.concurrent = True
			elif value in ('0', 'off'):
				env.concurrent = False
			else:
				usage()

	if len(args) != 2:
		usage()

	profileFilename = args[0]
	resultsDir = args[1]

	core.checkDir(resultsDir, False)

	results = core.TestrunResult()

	# Set results.name
	if OptionName is '':
		results.name = path.basename(resultsDir)
	else:
		results.name = OptionName

	# Begin json.
	result_filepath = os.path.join(resultsDir, 'main')
	json_writer = core.JSONWriter(SyncFileWriter(result_filepath))
	json_writer.open_dict()

	json_writer.write_dict_item('name', results.name)
	for (key, value) in env.collectData().items():
		json_writer.write_dict_item(key, value)

	profile = core.loadTestProfile(profileFilename)
	time_start = time.time()

	profile.run(env, json_writer)

	time_end = time.time()
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
