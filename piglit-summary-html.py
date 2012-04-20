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
import cgi
import os, os.path
import sys

sys.path.append(os.path.dirname(os.path.realpath(sys.argv[0])))
import framework.core as core
import framework.summary


#############################################################################
##### Auxiliary functions
#############################################################################

def testPathToHtmlFilename(path):
	return 'test_' + filter(lambda s: s.isalnum() or s == '_', path.replace('/', '__')) + '.html'


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
	f.write(text.encode('utf-8'))
	f.close()

templatedir = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'templates')
Result = readfile(os.path.join(templatedir, 'result.html'))
ResultDetail = readfile(os.path.join(templatedir, 'result_detail.html'))
ResultList = readfile(os.path.join(templatedir, 'result_list.html'))
ResultListItem = readfile(os.path.join(templatedir, 'result_listitem.html'))
ResultMString = readfile(os.path.join(templatedir, 'result_mstring.html'))

Index = readfile(os.path.join(templatedir, 'index.html'))
IndexTestrun = readfile(os.path.join(templatedir, 'index_testrun.html'))
IndexTestrunB = readfile(os.path.join(templatedir, 'index_testrunb.html'))
IndexTestrunBHref = readfile(os.path.join(templatedir, 'index_testrunb_href.html'))
IndexGroup = readfile(os.path.join(templatedir, 'index_group.html'))
IndexGroupTestrun = readfile(os.path.join(templatedir, 'index_group_testrun.html'))
IndexGroupGroup = readfile(os.path.join(templatedir, 'index_groupgroup.html'))
IndexTest = readfile(os.path.join(templatedir, 'index_test.html'))
IndexTestTestrun = readfile(os.path.join(templatedir, 'index_test_testrun.html'))

Testrun = readfile(os.path.join(templatedir, 'testrun.html'))

SummaryPages = {
	'all': 'index.html',
	'changes': 'changes.html',
	'problems': 'problems.html',
	'regressions': 'regressions.html',
	'fixes': 'fixes.html'
}

def buildResultListItem(detail):
	return ResultListItem % { 'detail': buildDetailValue(detail) }

def buildDetailValue(detail):
	if type(detail) == list:
		items = map(buildResultListItem, detail)
		return ResultList % { 'items': "".join(items) }

	elif isinstance(detail, basestring):
		return ResultMString % { 'detail': cgi.escape(detail) }

	return cgi.escape(str(detail))


def buildDetails(testResult):
	details = []
	for name in testResult:
		assert(isinstance(name, basestring))

		if name == 'result':
			continue

		value = buildDetailValue(testResult[name])
		details += [(name, value)]

	details.sort(lambda a, b: len(a[1])-len(b[1]))

	text = ''
	alternate = 'a'
	for name, value in details:
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

def writeTestrunHtml(testrun, filename):
	detail_keys = [
		key
		for key in testrun.__dict__.keys()
		if key in testrun.serialized_keys
		if key != 'tests'
		]
	detaildict = dict([(k, testrun.__dict__[k]) for k in detail_keys])
	details = buildDetails(detaildict)
	name = testrun.name
	codename = testrun.codename

	writefile(filename, Testrun % locals())

def hrefFromParts(codename, path):
	outStr = codename + '/' + testPathToHtmlFilename(path)
	if outStr[0] == '/':
		outStr = outStr[1:]
	return outStr

def buildTestSummary(indent, alternate, testsummary):
	tenindent = 10 - indent
	path = testsummary.path
	name = testsummary.name
	testruns = "".join([IndexTestTestrun % {
		'alternate': alternate,
		'status': result.status,
		'link': hrefFromParts(result.testrun.codename, path)
	} for result in testsummary.results])

	return IndexTest % locals()


def buildGroupSummaryTestrun(groupresult):
	passnr = groupresult.passvector.passnr
	warnnr = groupresult.passvector.warnnr
	failnr = groupresult.passvector.failnr
	skipnr = groupresult.passvector.skipnr
	crashnr = groupresult.passvector.crashnr
	totalnr = passnr + warnnr + failnr + crashnr # do not count skips

	if crashnr > 0:
		status = 'crash'
	elif failnr > 0:
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
	elif showcurrent == 'regressions':
		names = filter(lambda n: groupsummary.children[n].regressions, names)
	elif showcurrent == 'fixes':
		names = filter(lambda n: groupsummary.children[n].fixes, names)

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

	def testrunb(tr):
		if 'href' in tr.__dict__:
			return IndexTestrunBHref % tr.__dict__
		else:
			return IndexTestrunB % tr.__dict__

	group = buildGroupSummary(1, summary.root, showcurrent)
	testruns = "".join([IndexTestrun % tr.__dict__ for tr in summary.testruns])
	testrunsb = "".join([testrunb(tr) for tr in summary.testruns])

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
  -l, --list=listfile   Use test results from a list file

Example:
  %(progName)s summary/mysum results/all.results

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
	code = "".join([s for s in file])
	file.close()
	return eval(code)

def loadresult(descr):
	result = core.loadTestResults(descr[0])
	if len(descr) > 1:
		result.__dict__.update(descr[1])
	return result

def main():
	try:
		options, args = getopt(sys.argv[1:], "hofl:", [ "help", "overwrite", "list" ])
	except GetoptError:
		usage()

	OptionOverwrite = False
	OptionList = []
	for name, value in options:
		if name == "-h" or name == "--help":
			usage()
		elif name == "-o" or name == "--overwrite":
			OptionOverwrite = True
		elif name == "-l" or name == "--list":
			OptionList += parse_listfile(value)

	OptionList += [[name] for name in args[1:]]

	if len(args) < 1 or len(OptionList) == 0:
		usage()

	summaryDir = args[0]
	core.checkDir(summaryDir, not OptionOverwrite)

	results = []
	for result_dir in OptionList:
		results.append(loadresult(result_dir))

	summary = framework.summary.Summary(results)
	for j in range(len(summary.testruns)):
		tr = summary.testruns[j]
		tr.codename = filter(lambda s: s.isalnum(), tr.name)
		dirname = summaryDir + '/' + tr.codename
		core.checkDir(dirname, False)
		writeTestrunHtml(tr, dirname + '/index.html')
		for test in summary.allTests():
			filename = dirname + '/' + testPathToHtmlFilename(test.path)
			writeResultHtml(test, test.results[j], filename)

	writefile(os.path.join(summaryDir, 'result.css'), readfile(os.path.join(templatedir, 'result.css')))
	writefile(os.path.join(summaryDir, 'index.css'), readfile(os.path.join(templatedir, 'index.css')))
	writeSummaryHtml(summary, summaryDir, 'all')
	writeSummaryHtml(summary, summaryDir, 'problems')
	writeSummaryHtml(summary, summaryDir, 'changes')
	writeSummaryHtml(summary, summaryDir, 'regressions')
	writeSummaryHtml(summary, summaryDir, 'fixes')


if __name__ == "__main__":
	main()
