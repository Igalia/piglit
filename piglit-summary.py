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

# Print a very simple summary of piglit results file(s).
# When multiple result files are specified, compare the results
# of each test run to look for differences/regressions.
#
# Brian Paul
# April 2013


from getopt import getopt, GetoptError
import cgi
import os, os.path
import sys
import string

sys.path.append(os.path.dirname(os.path.realpath(sys.argv[0])))
import framework.core as core
import framework.summary


#############################################################################
##### Main program
#############################################################################
def usage():
	USAGE = """\
Usage: %(progName)s [options] resultsfile [...]

Print path/name of each test and the result.
When multiple files are specified, count the number of differences in results.
Tests are sorted by name.

Options:
  -h, --help            Show this message
  -s, --summary         Only display pass/fail summary
  -d, --diff            Only display the differences between multiple result files
  -l, --list=listfile   Use test results from a list file

Example list file:
 [
   [ 'test.result', { name: 'override-name' } ],
   [ 'other.result' ]
 ]
"""
	print USAGE % {'progName': sys.argv[0]}
	sys.exit(1)


def parse_listfile(filename):
	file = open(filename, "r")
	code = file.read()
	file.close()
	return eval(code)

def loadresult(descr):
	result = core.loadTestResults(descr[0])
	if len(descr) > 1:
		result.__dict__.update(descr[1])
	return result

def main():
	try:
		options, args = getopt(sys.argv[1:], "hsdl:", [ "help", "summary", "diff", "list" ])
	except GetoptError:
		usage()

	OptionList = []
	CountsOnly = False
	DiffOnly = False
	for name, value in options:
		if name == "-h" or name == "--help":
			usage()
		elif name == "-s" or name == "--summary":
			CountsOnly = True
		elif name == "-d" or name == "--diff":
			DiffOnly = True
		elif name == "-l" or name == "--list":
			OptionList += parse_listfile(value)

	OptionList += [[name] for name in args[0:]]

	if len(args) == 0 and len(OptionList) == 0:
		usage()

	# make list of results
	results = []
	for result_dir in OptionList:
		results.append(loadresult(result_dir))

	summary = framework.summary.Summary(results)

	# possible test outcomes
	possible_results = [ "pass", "fail", "crash", "skip", "warn" ]
	if len(OptionList) > 1:
			possible_results.append("changes")

	# init the summary counters
	counts = {}
	for result in possible_results:
			counts[result] = 0

	# get all results
	all = summary.allTests()

	# sort the results list by path
	all = sorted(all, key=lambda test: test.path)

	# loop over the tests
	for test in all:
		results = []
		anyChange = False
		# loop over the results for multiple runs
		for j in range(len(summary.testruns)):
			outcome = test.results[j]['result'] # 'pass', 'fail', etc.
			# check for different results between multiple runs
			if len(results) >= 1 and not outcome in results:
				# something changed
				counts["changes"] += 1
				anyChange = True
			results.append(outcome)

		# if all test runs had the same outcome:
		if not anyChange:
			counts[outcome] += 1

		# print the individual test result line
		if DiffOnly:
			if anyChange:
				print "%s: %s" % (test.path, string.join(results," "))
		elif not CountsOnly:
			print "%s: %s" % (test.path, string.join(results," "))

	# print the summary info
	print "summary:"
	total = 0
	for result in possible_results:
		print " %7s: %5d" % (result, counts[result])
		total += counts[result]
	print "   total: %5d" % total


if __name__ == "__main__":
	main()
