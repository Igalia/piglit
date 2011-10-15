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
import shlex
import types

from core import Test, testBinDir, TestResult

#############################################################################
##### ExecTest: A shared base class for tests that simply run an executable.
#############################################################################

class ExecTest(Test):
	def __init__(self, command):
		Test.__init__(self)
		self.command = command
		self.env = {}

		if type(self.command) is types.StringType:
			self.command = shlex.split(self.command)

	def interpretResult(self, out, results):
		raise NotImplementedError
		return out

	def run(self):
		fullenv = os.environ.copy()
		for e in self.env:
			fullenv[e] = str(self.env[e])

		if self.command is not None:
			proc = subprocess.Popen(
				self.command,
				stdout=subprocess.PIPE,
				stderr=subprocess.PIPE,
				env=fullenv,
				universal_newlines=True
				)
			out, err = proc.communicate()

			# proc.communicate() returns 8-bit strings, but we need
			# unicode strings.  In Python 2.x, this is because we
			# will eventually be serializing the strings as JSON,
			# and the JSON library expects unicode.  In Python 3.x,
			# this is because all string operations require
			# unicode.  So translate the strings into unicode,
			# assuming they are using UTF-8 encoding.
			#
			# If the subprocess output wasn't properly UTF-8
			# encoded, we don't want to raise an exception, so
			# translate the strings using 'replace' mode, which
			# replaces erroneous charcters with the Unicode
			# "replacement character" (a white question mark inside
			# a black diamond).
			out = out.decode('utf-8', 'replace')
			err = err.decode('utf-8', 'replace')

			results = TestResult()

			out = self.interpretResult(out, results)

			if proc.returncode == -5:
				results['result'] = 'trap'
			elif proc.returncode == -6:
				results['result'] = 'abort'
			elif proc.returncode in (-8, -10, -11):
				results['result'] = 'crash'
			elif proc.returncode == -1073741819:
				# 0xc0000005
				# Windows EXCEPTION_ACCESS_VIOLATION
				results['result'] = 'crash'
			elif proc.returncode == -1073741676:
				# 0xc0000094
				# Windows EXCEPTION_INT_DIVIDE_BY_ZERO
				results['result'] = 'crash'
			elif proc.returncode != 0:
				results['result'] = 'fail'
				results['note'] = 'Returncode was %d' % (proc.returncode)

			env = ''
			for key in self.env:
				env = env + key + '="' + self.env[key] + '" '
			if env:
				results['environment'] = env
			results['info'] = "Returncode: %d\n\nErrors:\n%s\n\nOutput:\n%s" % (proc.returncode, err, out)
			results['returncode'] = proc.returncode
			results['command'] = ' '.join(self.command)

			self.handleErr(results, err)

		else:
			results = TestResult()
			if 'result' not in results:
				results['result'] = 'skip'

		return results



#############################################################################
##### PlainExecTest: Run a "native" piglit test executable
##### Expect one line prefixed PIGLIT: in the output, which contains a
##### result dictionary. The plain output is appended to this dictionary
#############################################################################
class PlainExecTest(ExecTest):
	def __init__(self, command):
		ExecTest.__init__(self, command)
		# Prepend testBinDir to the path.
		self.command[0] = testBinDir + self.command[0]

	def interpretResult(self, out, results):
		outlines = out.split('\n')
		outpiglit = map(lambda s: s[7:], filter(lambda s: s.startswith('PIGLIT:'), outlines))

		if len(outpiglit) > 0:
			try:
				results.update(eval(''.join(outpiglit), {}))
				out = '\n'.join(filter(lambda s: not s.startswith('PIGLIT:'), outlines))
			except:
				results['result'] = 'fail'
				results['note'] = 'Failed to parse result string'

		if 'result' not in results:
			results['result'] = 'fail'
		return out
