#!/usr/bin/python
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
import cgi
import errno
import os
import sys

import framework.core as core


#############################################################################
##### Auxiliary functions
#############################################################################

def testPathToHtmlFilename(path):
	return filter(lambda s: s.isalnum() or s == '_', path.replace('/', '__')) + '.html'


#############################################################################
##### Annotation preprocessing
#############################################################################

class PassVector:
	def __init__(self,p,w,f,s):
		self.passnr = p
		self.warnnr = w
		self.failnr = f
		self.skipnr = s

	def add(self,o):
		self.passnr += o.passnr
		self.warnnr += o.warnnr
		self.failnr += o.failnr
		self.skipnr += o.skipnr

def annotateOneTest(path, results):
	"""\
Result is an array containing one test result.
"""
	result = ''
	if 'result' in results:
		result = results['result']

	vectormap = {
		'pass': PassVector(1,0,0,0),
		'warn': PassVector(0,1,0,0),
		'fail': PassVector(0,0,1,0),
		'skip': PassVector(0,0,0,1)
	}

	if result not in vectormap:
		result = 'warn'

	results.status = result
	results.passvector = vectormap[result]
	results.path = path

	return results


def annotateTest(path, results):
	"""\
Result is an array containing corresponding test results, one per test run
"""
	for j in range(len(results)):
		results[j] = annotateOneTest(path, results[j])

	stati = set([r.status for r in results])
	changes = len(stati) > 1
	problems = len(stati - set(['pass', 'skip'])) > 0
	for r in results:
		r.changes = changes
		r.problems = problems


def annotateGroup(path, results):
	"""\
Result is an array containing corresponding GroupResults, one per test run
"""
	groupnames = set()
	testnames = set()

	changes = False
	problems = False

	for r in results:
		r.passvector = PassVector(0,0,0,0)
		r.path = path

		for name in r:
			if type(name) != str:
				continue

			if isinstance(r[name], core.GroupResult):
				groupnames.add(name)
			else:
				testnames.add(name)

	for name in groupnames:
		children = []

		for r in results:
			if name not in r:
				r[name] = core.GroupResult()

			children.append(r[name])

		spath = name
		if len(path) > 0:
			spath = path + '/' + spath

		annotateGroup(spath, children)

		changes = changes or results[0][name].changes
		problems = problems or results[0][name].problems

		for r in results:
			r.passvector.add(r[name].passvector)


	for name in testnames:
		children = []

		for r in results:
			if name not in r:
				r[name] = core.TestResult({}, { 'result': 'skip' })

			# BACKWARDS COMPATIBILITY
			if not isinstance(r[name], core.TestResult):
				r[name] = core.TestResult({}, r[name])
			# END BACKWARDS COMPATIBILITY

			children.append(r[name])

		spath = name
		if len(path) > 0:
			spath = path + '/' + spath

		annotateTest(spath, children)

		changes = changes or results[0][name].changes
		problems = problems or results[0][name].problems

		for r in results:
			r.passvector.add(r[name].passvector)

	for r in results:
		r.changes = changes
		r.problems = problems


#############################################################################
##### HTML output
#############################################################################

def readfile(filename):
	f = open(filename, "r")
	s = f.read()
	f.close()
	return s

def writefile(filename, text):
	f = open(filename, "w")
	f.write(text)
	f.close()

templatedir = os.path.dirname(__file__) + '/templates/'
Result = readfile(templatedir + 'result.html')
ResultDetail = readfile(templatedir + 'result_detail.html')
ResultList = readfile(templatedir + 'result_list.html')
ResultListItem = readfile(templatedir + 'result_listitem.html')
ResultMString = readfile(templatedir + 'result_mstring.html')

Index = readfile(templatedir + 'index.html')
IndexTestrun = readfile(templatedir + 'index_testrun.html')
IndexTestrunB = readfile(templatedir + 'index_testrunb.html')
IndexGroup = readfile(templatedir + 'index_group.html')
IndexGroupTestrun = readfile(templatedir + 'index_group_testrun.html')
IndexGroupGroup = readfile(templatedir + 'index_groupgroup.html')
IndexTest = readfile(templatedir + 'index_test.html')
IndexTestTestrun = readfile(templatedir + 'index_test_testrun.html')

SummaryPages = {
	'all': 'index.html',
	'changes': 'changes.html',
	'problems': 'problems.html'
}

def buildDetailValue(detail):
	if type(detail) == list:
		items = ''

		for d in detail:
			items = items + ResultListItem % { 'detail': buildDetailValue(d) }

		return ResultList % { 'items': items }
	elif type(detail) == str and detail[0:3] == '@@@':
		return ResultMString % { 'detail': cgi.escape(detail[3:]) }

	return cgi.escape(str(detail))


def buildDetails(testResult):
	details = []
	for name in testResult:
		if type(name) != str or name == 'result':
			continue

		value = buildDetailValue(testResult[name])
		details += [(name,value)]

	details.sort(lambda a,b: len(a[1])-len(b[1]))

	text = ''
	alternate = 'a'
	for name,value in details:
		text += ResultDetail % locals()

		if alternate == 'a':
			alternate = 'b'
		else:
			alternate = 'a'

	return text


def writeResultHtml(testResult, filename):
	path = testResult.path
	name = testResult.path.split('/')[-1]
	status = testResult.status

	if 'result' in testResult:
		result = testResult['result']
	else:
		result = '?'

	details = buildDetails(testResult)

	writefile(filename, Result % locals())


def recursiveWriteResultHtml(results, summaryDir):
	for n in results:
		if type(n) != str:
			continue

		if isinstance(results[n], core.GroupResult):
			recursiveWriteResultHtml(results[n], summaryDir)
		else:
			writeResultHtml(results[n], summaryDir + '/' + testPathToHtmlFilename(results[n].path))


def buildTestSummary(indent, alternate, name, test):
	tenindent = 10 - indent
	testruns = "".join([IndexTestTestrun % {
		'alternate': alternate,
		'status': t.status,
		'link': r.codename + '/' + testPathToHtmlFilename(t.path)
	} for r,t in test])

	return IndexTest % locals()


def buildGroupSummaryTestrun(results, group):
	passnr = group.passvector.passnr
	warnnr = group.passvector.warnnr
	failnr = group.passvector.failnr
	skipnr = group.passvector.skipnr
	totalnr = passnr + warnnr + failnr # do not count skips

	if failnr > 0:
		status = 'fail'
	elif warnnr > 0:
		status = 'warn'
	elif passnr > 0:
		status = 'pass'
	else:
		status = 'skip'

	return IndexGroupTestrun % locals()


def buildGroupSummary(indent, name, results, showcurrent):
	"""\
testruns is an array of pairs (results,group), where results is the
entire testrun record and group is the group we're currently printing.
"""
	tenindent = 10 - indent

	items = ''
	alternate = 'a'
	names = filter(lambda k: type(k) == str, results[0][1])

	if showcurrent == 'changes':
		names = filter(lambda n: results[0][1][n].changes, names)
	elif showcurrent == 'problems':
		names = filter(lambda n: results[0][1][n].problems, names)

	names.sort()
	for n in names:
		if isinstance(results[0][1][n], core.GroupResult):
			items = items + IndexGroupGroup % {
				'group': buildGroupSummary(indent+1, n, [(r,g[n]) for r,g in results], showcurrent)
			}
		else:
			items = items + buildTestSummary(indent+1, alternate, n, [(r,g[n]) for r,g in results])

		if alternate == 'a':
			alternate = 'b'
		else:
			alternate = 'a'

	testruns = "".join([buildGroupSummaryTestrun(r,g) for r,g in results])

	return IndexGroup % locals()


def writeSummaryHtml(results, summaryDir, showcurrent):
	"""\
results is an array containing the top-level results dictionarys.
"""
	def link(to):
		if to == showcurrent:
			return to
		else:
			page = SummaryPages[to]
			return '<a href="%(page)s">%(to)s</a>' % locals()

	group = buildGroupSummary(1, 'Total', [(r,r.results) for r in results], showcurrent)
	testruns = "".join([IndexTestrun % r.__dict__ for r in results])
	testrunsb = "".join([IndexTestrunB % r.__dict__ for r in results])

	tolist = SummaryPages.keys()
	tolist.sort()
	showlinks = " | ".join([link(to) for to in tolist])

	writefile(summaryDir + '/' + SummaryPages[showcurrent], Index % locals())


#############################################################################
##### Main program
#############################################################################
def usage():
	USAGE = """\
Usage: %(progName)s [options] [summary-dir] [test.results]...

Options:
  -h, --help            Show this message
  -o, --overwrite       Overwrite existing directories

Example:
  %(progName)s summary/mysum results/all.results
"""
	print USAGE % {'progName': sys.argv[0]}
	sys.exit(1)


def main():
	try:
		options, args = getopt(sys.argv[1:], "ho", [ "help", "overwrite" ])
	except GetoptError:
		usage()

	OptionOverwrite = False
	for name,value in options:
		if name == "-h" or name == "--help":
			usage()
		elif name == "-o" or name == "--overwrite":
			OptionOverwrite = True

	if len(args) < 2:
		usage()

	summaryDir = args[0]
	resultFilenames = args[1:]

	core.checkDir(summaryDir, not OptionOverwrite)

	results = [core.loadTestResults(name) for name in resultFilenames]

	annotateGroup('', [r.results for r in results])
	for r in results:
		r.codename = filter(lambda s: s.isalnum(), r.name)
		core.checkDir(summaryDir + '/' + r.codename, False)
		recursiveWriteResultHtml(r.results, summaryDir + '/' + r.codename)

	writefile(summaryDir + '/result.css', readfile(templatedir + 'result.css'))
	writefile(summaryDir + '/index.css', readfile(templatedir + 'index.css'))
	writeSummaryHtml(results, summaryDir, 'all')
	writeSummaryHtml(results, summaryDir, 'problems')
	writeSummaryHtml(results, summaryDir, 'changes')


if __name__ == "__main__":
	main()
