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
import framework.summary


#############################################################################
##### Auxiliary functions
#############################################################################

def testPathToHtmlFilename(path):
	return filter(lambda s: s.isalnum() or s == '_', path.replace('/', '__')) + '.html'


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


def writeResultHtml(test, testResult, filename):
	path = test.path
	name = test.name
	status = testResult.status

	if 'result' in testResult:
		result = testResult['result']
	else:
		result = '?'

	details = buildDetails(testResult)

	writefile(filename, Result % locals())


def buildTestSummary(indent, alternate, testsummary):
	tenindent = 10 - indent
	path = testsummary.path
	name = testsummary.name
	testruns = "".join([IndexTestTestrun % {
		'alternate': alternate,
		'status': result.status,
		'link': result.testrun.codename + '/' + testPathToHtmlFilename(path)
	} for result in testsummary.results])

	return IndexTest % locals()


def buildGroupSummaryTestrun(groupresult):
	passnr = groupresult.passvector.passnr
	warnnr = groupresult.passvector.warnnr
	failnr = groupresult.passvector.failnr
	skipnr = groupresult.passvector.skipnr
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


def buildGroupSummary(indent, groupsummary, showcurrent):
	tenindent = 10 - indent

	items = ''
	alternate = 'a'
	path = groupsummary.path
	name = groupsummary.name
	names = groupsummary.children.keys()

	if showcurrent == 'changes':
		names = filter(lambda n: groupsummary.children[n].changes, names)
	elif showcurrent == 'problems':
		names = filter(lambda n: groupsummary.children[n].problems, names)

	names.sort()
	for n in names:
		child = groupsummary.children[n]
		if isinstance(child, framework.summary.GroupSummary):
			items = items + IndexGroupGroup % {
				'group': buildGroupSummary(indent+1, child, showcurrent)
			}
		else:
			items = items + buildTestSummary(indent+1, alternate, child)

		if alternate == 'a':
			alternate = 'b'
		else:
			alternate = 'a'

	testruns = "".join([buildGroupSummaryTestrun(result)
			for result in groupsummary.results])

	return IndexGroup % locals()


def writeSummaryHtml(summary, summaryDir, showcurrent):
	"""\
results is an array containing the top-level results dictionarys.
"""
	def link(to):
		if to == showcurrent:
			return to
		else:
			page = SummaryPages[to]
			return '<a href="%(page)s">%(to)s</a>' % locals()

	group = buildGroupSummary(1, summary.root, showcurrent)
	testruns = "".join([IndexTestrun % tr.__dict__ for tr in summary.testruns])
	testrunsb = "".join([IndexTestrunB % tr.__dict__ for tr in summary.testruns])

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

	summary = framework.summary.Summary(results)
	for j in range(len(summary.testruns)):
		tr = summary.testruns[j]
		tr.codename = filter(lambda s: s.isalnum(), tr.name)
		dirname = summaryDir + '/' + tr.codename
		core.checkDir(dirname, False)
		for test in summary.allTests():
			filename = dirname + testPathToHtmlFilename(test.path)
			writeResultHtml(test, test.results[j], filename)

	writefile(summaryDir + '/result.css', readfile(templatedir + 'result.css'))
	writefile(summaryDir + '/index.css', readfile(templatedir + 'index.css'))
	writeSummaryHtml(summary, summaryDir, 'all')
	writeSummaryHtml(summary, summaryDir, 'problems')
	writeSummaryHtml(summary, summaryDir, 'changes')


if __name__ == "__main__":
	main()
