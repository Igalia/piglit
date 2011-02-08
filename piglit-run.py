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
import re
import sys, os

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
		options, args = getopt(sys.argv[1:], "hdt:n:x:", [ "help", "dry-run", "tests=", "name=", "exclude-tests=" ])
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

	if len(args) != 2:
		usage()

	profileFilename = args[0]
	resultsDir = args[1]

	core.checkDir(resultsDir, False)

	profile = core.loadTestProfile(profileFilename)
	env.file = SyncFileWriter(resultsDir + '/main')
	env.file.writeLine("name: %(name)s" % { 'name': core.encode(OptionName) })
	env.collectData()
	profile.run(env)
	env.file.close()

	print "Writing summary file..."
	results = core.loadTestResults(resultsDir)
	for testname, result in results.allTestResults().items():
		if 'info' in result:
			if len(result['info']) > 4096:
				result['info'] = result['info'][0:4096]
	file = open(resultsDir + '/summary', "w")
	results.write(file)
	file.close()

	print
	print 'Thank you for running Piglit!'
	print 'Summary for submission has been written to ' + resultsDir + '/summary'


if __name__ == "__main__":
	main()
