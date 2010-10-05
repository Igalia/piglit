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

import os
import subprocess

from core import checkDir, testBinDir, Test, TestResult

#############################################################################
##### GleanTest: Execute a sub-test of Glean
#############################################################################
def gleanExecutable():
	return testBinDir + 'glean'

def gleanResultDir():
	return os.path.join('.', 'results', 'glean')

class GleanTest(Test):
	globalParams = []

	def __init__(self, name):
		Test.__init__(self)
		self.name = name
		self.env = {}

	def run(self):
		results = TestResult()

		fullenv = os.environ.copy()
		for e in self.env:
			fullenv[e] = str(self.env[e])

		checkDir(os.path.join(gleanResultDir(), self.name), False)

		glean = subprocess.Popen(
			[gleanExecutable(), "-r", os.path.join(gleanResultDir(), self.name),
			"-o",
			"-v", "-v", "-v",
			"-t", "+"+self.name] + GleanTest.globalParams,
			stdout=subprocess.PIPE,
			stderr=subprocess.PIPE,
			env=fullenv,
			universal_newlines=True
		)

		out, err = glean.communicate()

		results['result'] = 'pass'
		if glean.returncode != 0 or out.find('FAIL') >= 0:
			results['result'] = 'fail'

		results['returncode'] = glean.returncode

		self.handleErr(results, err)

		results['info'] = "@@@Returncode: %d\n\nErrors:\n%s\n\nOutput:\n%s" % (glean.returncode, err, out)

		return results

