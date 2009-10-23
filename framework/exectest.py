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

import subprocess

from core import Test, testBinDir, TestResult

#############################################################################
##### PlainExecTest: Simply run an executable
##### Expect one line prefixed PIGLIT: in the output, which contains a
##### result dictionary. The plain output is appended to this dictionary
#############################################################################
class PlainExecTest(Test):
	def __init__(self, command):
		Test.__init__(self)
		self.command = command
		# Prepend testBinDir to the path.
		self.command[0] = testBinDir + self.command[0]

	def run(self):
		proc = subprocess.Popen(
			self.command,
			stdout=subprocess.PIPE,
			stderr=subprocess.PIPE,
			universal_newlines=True
		)
		out, err = proc.communicate()

		outlines = out.split('\n')
		outpiglit = map(lambda s: s[7:], filter(lambda s: s.startswith('PIGLIT:'), outlines))

		results = TestResult()

		if len(outpiglit) > 0:
			try:
				results.update(eval(''.join(outpiglit), {}))
				out = '\n'.join(filter(lambda s: not s.startswith('PIGLIT:'), outlines))
			except:
				results['result'] = 'fail'
				results['note'] = 'Failed to parse result string'

		if 'result' not in results:
			results['result'] = 'fail'

		if proc.returncode != 0:
			results['result'] = 'fail'
			results['note'] = 'Returncode was %d' % (proc.returncode)

		self.handleErr(results, err)

		results['info'] = "@@@Returncode: %d\n\nErrors:\n%s\n\nOutput:\n%s" % (proc.returncode, err, out)
		results['returncode'] = proc.returncode

		return results


