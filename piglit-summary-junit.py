#!/usr/bin/env python
#
# Copyright 2010-2011 VMware, Inc.
# All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sub license, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice (including the
# next paragraph) shall be included in all copies or substantial portions
# of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


import optparse
import os
import sys

import framework.core
import framework.summary
from framework import junit


class Writer:

	def __init__(self, filename):
		self.report = junit.Report(filename)
		self.path = []

	def write(self, arg):
		results = [framework.core.loadTestResults(arg)]
		summary = framework.summary.Summary(results)

		self.report.start()
		self.report.startSuite('piglit')
		try:
			for test in summary.allTests():
				self.write_test(summary, test)
		finally:
			self.enter_path([])
			self.report.stopSuite()
			self.report.stop()

	def write_test(self, summary, test):
		test_path = test.path.split('/')
		test_name = test_path.pop()
		self.enter_path(test_path)

		assert len(summary.testruns) == 1
		tr = summary.testruns[0]
		result = test.results[0]

		self.report.startCase(test_name)
		duration = None
		try:
			try:
				self.report.addStdout(result['command'] + '\n')
			except KeyError:
				pass

			try:
				self.report.addStderr(result['info'])
			except KeyError:
				pass

			success = result.get('result')
			if success in ('pass', 'warn'):
				pass
			elif success == 'skip':
				self.report.addSkipped()
			else:
				self.report.addFailure(success)

			try:
				duration = float(result['time'])
			except KeyError:
				pass
		finally:
			self.report.stopCase(duration)

	def enter_path(self, path):
		ancestor = 0
		try:
			while self.path[ancestor] == path[ancestor]:
				ancestor += 1
		except IndexError:
			pass

		for dirname in self.path[ancestor:]:
			self.report.stopSuite()

		for dirname in path[ancestor:]:
			self.report.startSuite(dirname)

		self.path = path


def main():
	optparser = optparse.OptionParser(
		usage="\n\t%prog [options] test.results",
		version="%%prog")
	optparser.add_option(
		'-o', '--output', metavar='FILE',
		type="string", dest="output", default='piglit.xml',
		help="output filename")
	(options, args) = optparser.parse_args(sys.argv[1:])

	if len(args) != 1:
		optparser.error('need to specify one test result')
		usage()

	writer = Writer(options.output)
	writer.write(args[0])


if __name__ == "__main__":
	main()


# vim:set sw=4 ts=4 noet:
