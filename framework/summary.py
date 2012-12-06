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

import core


#############################################################################
##### Vector indicating the number of subtests that have passed/failed/etc.
#############################################################################
class PassVector:
	def __init__(self, p, w, f, s, c):
		self.passnr = p
		self.warnnr = w
		self.failnr = f
		self.skipnr = s
		self.crashnr = c

	def add(self, o):
		self.passnr += o.passnr
		self.warnnr += o.warnnr
		self.failnr += o.failnr
		self.skipnr += o.skipnr
		self.crashnr += o.crashnr


#############################################################################
##### TestSummary: Summarize the results for one test across a
##### number of testruns
#############################################################################
class TestSummary:
	def isRegression(self, statiList):
		# Regression is:
		# - if an item is neither 'pass' nor 'skip'
		# - and if any item on the left side thereof is 'pass' or 'skip'
		for i in range(1, len(statiList)):
			if statiList[i-1] == 'pass' or statiList[i-1] == 'skip':
				for j in range(i, len(statiList)):
					if statiList[j] != 'pass' and statiList[j] != 'skip':
						return True
		return False

	def __init__(self, summary, path, name, results):
		"""\
summary is the root summary object
path is the path to the group (e.g. shaders/glean-fragProg1)
name is the display name of the group (e.g. glean-fragProg1)
results is an array of TestResult instances, one per testrun
"""
		self.summary = summary
		self.path = path
		self.name = name
		self.results = results[:]

		for j in range(len(self.results)):
			result = self.results[j]
			result.testrun = self.summary.testruns[j]
			result.status = ''
			if 'result' in result:
				result.status = result['result']

			vectormap = {
				'pass': PassVector(1,0,0,0,0),
				'warn': PassVector(0,1,0,0,0),
				'fail': PassVector(0,0,1,0,0),
				'skip': PassVector(0,0,0,1,0),
				'crash': PassVector(0,0,0,0,1)
			}

			if result.status not in vectormap:
				result.status = 'warn'

			result.passvector = vectormap[result.status]

		statiList = [result.status for result in results]
		statiSet = set(statiList)
		self.changes = len(statiSet) > 1
		self.problems = len(statiSet - set(['pass', 'skip'])) > 0
		self.regressions = self.isRegression(statiList)
		statiList.reverse()
		self.fixes = self.isRegression(statiList)
		self.skipped = 'skip' in statiSet

	def allTests(self):
		return [self]

#############################################################################
##### GroupSummary: Summarize a group of tests
#############################################################################
class GroupSummary:
	def createDummyGroup(self, result, test_name):
		new_group = core.GroupResult()
		new_group[' ' + test_name + '(All Tests)'] = result[test_name]
		result[test_name] = new_group

	def __init__(self, summary, path, name, results):
		"""\
summary is the root summary object
path is the path to the group (e.g. shaders/glean-fragProg1)
name is the display name of the group (e.g. glean-fragProg1)
results is an array of GroupResult instances, one per testrun
"""
		self.summary = summary
		self.path = path
		self.name = name
		self.results = results[:]
		self.changes = False
		self.problems = False
		self.regressions = False
		self.fixes = False
		self.skipped = False
		self.children = {}

		# Perform some initial annotations
		for j in range(len(self.results)):
			result = self.results[j]
			result.passvector = PassVector(0, 0, 0, 0, 0)
			result.testrun = self.summary.testruns[j]

		# Collect, create and annotate children
		for result in self.results:
			for name in result:
				if name in self.children:
					continue

				childpath = name
				if len(self.path) > 0:
					childpath = self.path + '/' + childpath

				if isinstance(result[name], core.GroupResult):
					# This loop checks to make sure that all results
					# with the same 'name' are of the same type.
					# This is necessary to handle the case where
					# a testname in an earlier result (an earlier
					# result in this case means a result that
					# comes before the current result in self.results)
					# denotes a test group but in a later
					# result it denotes a single test case, for example:
					#
					# result 0:
					#	test/group/a PASS
					#	test/group/b PASS
					#	test/group/c PASS
					# result 1:
					#	test/group PASS
					for r in self.results:
						if r.has_key(name) and not isinstance(r[name], core.GroupResult):
							self.createDummyGroup(r, name)

					childresults = [r.get(name, core.GroupResult())
							for r in self.results]

					self.children[name] = GroupSummary(
						summary,
						childpath,
						name,
						childresults
					)
				else:
					# We need to check and handle the reversed case
					# described in the above comment e.g.:
					# result 0:
					#	test/group PASS
					# result 1:
					#	test/group/a PASS
					#	test/group/b PASS
					#	test/group/c PASS
					need_group = 0
					for r in self.results:
						if r.has_key(name) and not isinstance(r[name], core.TestResult):
							need_group = 1
					if need_group:
						for r in self.results:
							if r.has_key(name) and isinstance(r[name], core.TestResult):
								self.createDummyGroup(r, name)
						childresults = [r.get(name, core.GroupResult())
								for r in self.results]

						self.children[name] = GroupSummary(
							summary,
							childpath,
							name,
							childresults
						)
					else:
						childresults = [r.get(name, core.TestResult({ 'result': 'skip' }))
								for r in self.results]
						self.children[name] = TestSummary(
							summary,
							childpath,
							name,
							childresults
						)

				for j in range(len(self.results)):
					self.results[j].passvector.add(childresults[j].passvector)

				self.changes = self.changes or self.children[name].changes
				self.problems = self.problems or self.children[name].problems
				self.regressions = self.regressions or self.children[name].regressions
				self.fixes = self.fixes or self.children[name].fixes
				self.skipped = self.skipped or self.children[name].skipped

	def allTests(self):
		"""\
Returns an array of all child TestSummary instances.
"""
		return [t for name in self.children for t in self.children[name].allTests()]

#############################################################################
##### Summary: Summarize an array of testruns
#############################################################################
class Summary:
	def __init__(self, testruns):
		"""\
testruns is an array of TestrunResult instances
"""
		groups = [
			core.GroupResult.make_tree(testrun.tests)
			for testrun in testruns
			]
		self.testruns = testruns
		self.root = GroupSummary(self, '', 'All', groups)

	def allTests(self):
		"""\
Returns an array of all child TestSummary instances.
"""
		return self.root.allTests()
