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
import sys, os.path

sys.path.append(os.path.dirname(os.path.realpath(sys.argv[0])))
import framework.core as core



#############################################################################
##### Main program
#############################################################################
def usage():
	USAGE = """\
Usage: %(progName)s [options] [main results file]

Options:
  -h, --help                Show this message

Example:
  %(progName)s results/main > results/summary
"""
	print USAGE % {'progName': sys.argv[0]}
	sys.exit(1)

def main():
	env = core.Environment()

	try:
		options, args = getopt(sys.argv[1:], "h", [ "help" ])
	except GetoptError:
		usage()

	OptionName = ''

	for name, value in options:
		if name in ('-h', '--help'):
			usage()

	if len(args) < 2:
		usage()

	combined = core.loadTestResults(args[0])
	del args[0]

	for resultsDir in args:
		results = core.loadTestResults(resultsDir)

		for testname, result in results.tests.items():
			combined.tests[testname] = result

	combined.write(sys.stdout)


if __name__ == "__main__":
	main()
