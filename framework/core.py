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
# PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

# Piglit core

import errno
import os
import re
import subprocess
import sys
import traceback

__all__ = [
	'Environment',
	'loadTestProfile',
	'testPathToResultName',
	'GroupResult',
	'TestResult'
]


#############################################################################
##### Helper functions
#############################################################################

# Ensure the given directory exists
def checkDir(dirname, failifexists):
	exists = True
	try:
		os.stat(dirname)
	except OSError, e:
		if e.errno == errno.ENOENT or e.errno == errno.ENOTDIR:
			exists = False

	if exists and failifexists:
		print >>sys.stderr, "%(dirname)s exists already.\nUse --overwrite if you want to overwrite it.\n" % locals()
		exit(1)

	try:
		os.makedirs(dirname)
	except OSError, e:
		if e.errno != errno.EEXIST:
			raise

def testPathToResultName(path):
	elems = filter(lambda s: len(s) > 0, path.split('/'))
	pyname = 'testrun.results' + "".join(map(lambda s: "['"+s+"']", elems))
	return pyname


#############################################################################
##### Result classes
#############################################################################


class TestResult(dict):
	def __init__(self, *args):
		dict.__init__(self)

		assert(len(args) == 0 or len(args) == 2)

		if len(args) == 2:
			for k in args[0]:
				self.__setattr__(k, args[0][k])

			self.update(args[1])

	def __repr__(self):
		attrnames = set(dir(self)) - set(dir(self.__class__()))
		return '%(class)s(%(dir)s,%(dict)s)' % {
			'class': self.__class__.__name__,
			'dir': dict([(k, self.__getattribute__(k)) for k in attrnames]),
			'dict': dict.__repr__(self)
		}


class GroupResult(dict):
	def __init__(self, *args):
		dict.__init__(self)

		assert(len(args) == 0 or len(args) == 2)

		if len(args) == 2:
			for k in args[0]:
				self.__setattr__(k, args[0][k])

			self.update(args[1])

	def __repr__(self):
		attrnames = set(dir(self)) - set(dir(self.__class__()))
		return '%(class)s(%(dir)s,%(dict)s)' % {
			'class': self.__class__.__name__,
			'dir': dict([(k, self.__getattribute__(k)) for k in attrnames]),
			'dict': dict.__repr__(self)
		}

class TestrunResult:
	def __init__(self, *args):
		self.name = ''
		self.results = GroupResult()



#############################################################################
##### Generic Test classes
#############################################################################

class Environment:
	def __init__(self):
		self.file = sys.stdout
		self.execute = True
		self.filter = []

class Test:
	ignoreErrors = []

	def doRun(self, env, path):
		# Filter
		if len(env.filter) > 0:
			if not True in map(lambda f: f.search(path) != None, env.filter):
				return None

		# Run the test
		if env.execute:
			try:
				print "Test: %(path)s" % locals()
				result = self.run()
				if 'result' not in result:
					result['result'] = 'fail'
				if not isinstance(result, TestResult):
					result = TestResult({}, result)
					result['result'] = 'warn'
					result['note'] = 'Result not returned as an instance of TestResult'
			except:
				result = TestResult()
				result['result'] = 'fail'
				result['exception'] = str(sys.exc_info()[0]) + str(sys.exc_info()[1])
				result['traceback'] = '@@@' + "".join(traceback.format_tb(sys.exc_info()[2]))

			print "    result: %(result)s" % { 'result': result['result'] }

			varname = testPathToResultName(path)
			print >>env.file, "%(varname)s = %(result)s" % locals()
		else:
			print "Dry-run: %(path)s" % locals()

	# Default handling for stderr messages
	def handleErr(self, results, err):
		errors = filter(lambda s: len(s) > 0, map(lambda s: s.strip(), err.split('\n')))

		ignored = []
		for s in errors:
			ignore = False
			for pattern in Test.ignoreErrors:
				if type(pattern) == str:
					if s.find(pattern) >= 0:
						ignore = True
						break
				else:
					if pattern.search(s):
						ignore = True
						break
			if ignore:
				ignored.append(s)

		errors = [s for s in errors if s not in ignored]

		if len(errors) > 0:
			results['errors'] = errors

			if results['result'] == 'pass':
				results['result'] = 'warn'

		if len(ignored) > 0:
			results['errors_ignored'] = ignored


class Group(dict):
	def doRun(self, env, path):
		print >>env.file, "%s = GroupResult()" % (testPathToResultName(path))
		for sub in self:
			spath = sub
			if len(path) > 0:
				spath = path + '/' + spath
			self[sub].doRun(env, spath)

#############################################################################
##### PlainExecTest: Simply run an executable
##### Expect one line prefixed PIGLIT: in the output, which contains a
##### result dictionary. The plain output is appended to this dictionary
#############################################################################
class PlainExecTest(Test):
	def __init__(self, command):
		self.command = command

	def run(self):
		proc = subprocess.Popen(
			self.command,
			stdout=subprocess.PIPE,
			stderr=subprocess.PIPE
		)
		out,err = proc.communicate()

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



#############################################################################
##### GleanTest: Execute a sub-test of Glean
#############################################################################
def gleanExecutable():
	return "./tests/glean/glean"

def gleanResultDir():
	return "./results/glean/"

class GleanTest(Test):
	globalParams = []

	def __init__(self, name):
		self.name = name
		self.env = {}

	def run(self):
		results = TestResult()

		fullenv = os.environ.copy()
		for e in self.env:
			fullenv[e] = str(self.env[e])

		checkDir(gleanResultDir()+self.name, False)

		glean = subprocess.Popen(
			[gleanExecutable(), "-o", "-r", gleanResultDir()+self.name,
			"--ignore-prereqs",
			"-v", "-v", "-v",
			"-t", "+"+self.name] + GleanTest.globalParams,
			stdout=subprocess.PIPE,
			stderr=subprocess.PIPE,
			env=fullenv
		)

		out, err = glean.communicate()

		results['result'] = 'pass'
		if glean.returncode != 0 or out.find('FAIL') >= 0:
			results['result'] = 'fail'

		results['returncode'] = glean.returncode

		self.handleErr(results, err)

		results['info'] = "@@@Returncode: %d\n\nErrors:\n%s\n\nOutput:\n%s" % (glean.returncode, err, out)

		return results


#############################################################################
##### Loaders
#############################################################################

def loadTestProfile(filename):
	try:
		ns = {
			'__file__': filename,
			'__dir__': os.path.dirname(filename),
			'Test': Test,
			'Group': Group,
			'GleanTest': GleanTest,
			'gleanExecutable': gleanExecutable,
			'PlainExecTest': PlainExecTest
		}
		execfile(filename, ns)
		return ns['tests']
	except:
		traceback.print_exc()
		raise FatalError('Could not read tests profile')

def loadTestResults(filename):
	try:
		ns = {
			'__file__': filename,
			'GroupResult': GroupResult,
			'TestResult': TestResult,
			'TestrunResult': TestrunResult
		}
		execfile(filename, ns)

		# BACKWARDS COMPATIBILITY
		if 'testrun' not in ns:
			testrun = TestrunResult()
			testrun.results.update(ns['results'])
			if 'name' in ns:
				testrun.name = ns['name']
			ns['testrun'] = testrun
		# END BACKWARDS COMPATIBILITY

		testrun = ns['testrun']
		if len(testrun.name) == 0:
			testrun.name = filename

		return testrun
	except:
		traceback.print_exc()
		raise FatalError('Could not read tests results')
