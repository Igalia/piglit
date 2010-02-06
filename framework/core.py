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

# Piglit core

import errno
import os
import platform
import stat
import subprocess
import sys
import time
import traceback

__all__ = [
	'Environment',
	'checkDir',
	'loadTestProfile',
	'TestrunResult',
	'GroupResult',
	'TestResult',
	'TestProfile',
	'Group',
	'Test',
	'testBinDir'
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

# Encode a string
def encode(text):
	return text.encode("string_escape")

def decode(text):
	return text.decode("string_escape")

testBinDir = os.path.dirname(__file__) + '/../bin/'


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

	def allTestResults(self, name):
		return {name: self}

	def write(self, file, path):
		print >>file, "@test: " + encode(path)
		for k in self:
			v = self[k]
			if type(v) == list:
				print >>file, k + "!"
				for s in v:
					print >>file, " " + encode(str(s))
				print >>file, "!"
			else:
				print >>file, k + ": " + encode(str(v))
		print >>file, "!"


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

	def allTestResults(self, groupName):
		collection = {}
		for name, sub in self.items():
			subfullname = name
			if len(groupName) > 0:
				subfullname = groupName + '/' + subfullname
			collection.update(sub.allTestResults(subfullname))
		return collection

	def write(self, file, groupName):
		for name, sub in self.items():
			subfullname = name
			if len(groupName) > 0:
				subfullname = groupName + '/' + subfullname
			sub.write(file, subfullname)


class TestrunResult:
	def __init__(self, *args):
		self.name = ''
		self.globalkeys = ['name', 'href', 'glxinfo', 'lspci', 'time']
		self.results = GroupResult()

	def allTestResults(self):
		'''Return a dictionary containing (name: TestResult) mappings.
		Note that writing to this dictionary has no effect.'''
		return self.results.allTestResults('')

	def write(self, file):
		for key in self.globalkeys:
			if key in self.__dict__:
				print >>file, "%s: %s" % (key, encode(self.__dict__[key]))

		self.results.write(file,'')

	def parseFile(self, file):
		def arrayparser(a):
			def cb(line):
				if line == '!':
					del stack[-1]
				else:
					a.append(line[1:])
			return cb

		def dictparser(d):
			def cb(line):
				if line == '!':
					del stack[-1]
					return

				colon = line.find(':')
				if colon < 0:
					excl = line.find('!')
					if excl < 0:
						raise Exception("Line %(linenr)d: Bad format" % locals())

					key = line[:excl]
					d[key] = []
					stack.append(arrayparser(d[key]))
					return

				key = line[:colon]
				value = decode(line[colon+2:])
				d[key] = value
			return cb

		def toplevel(line):
			colon = line.find(':')
			if colon < 0:
				raise Exception("Line %(linenr)d: Bad format" % locals())

			key = line[:colon]
			value = decode(line[colon+2:])
			if key in self.globalkeys:
				self.__dict__[key] = value
			elif key == '@test':
				comp = value.split('/')
				group = self.results
				for name in comp[:-1]:
					if name not in group:
						group[name] = GroupResult()
					group = group[name]

				result = TestResult()
				group[comp[-1]] = result

				stack.append(dictparser(result))
			else:
				raise Exception("Line %d: Unknown key %s" % (linenr, key))

		stack = [toplevel]
		linenr = 1
		for line in file:
			if line[-1] == '\n':
				stack[-1](line[0:-1])
			linenr = linenr + 1

	def parseDir(self, path, PreferSummary):
		main = None
		filelist = [path + '/main', path + '/summary']
		if PreferSummary:
			filelist[:0] = [path + '/summary']
		for filename in filelist:
			try:
				main = open(filename, 'U')
				break
			except:
				pass
		if not main:
			raise Exception("Failed to open %(path)s" % locals())
		self.parseFile(main)
		main.close()


#############################################################################
##### Generic Test classes
#############################################################################

class Environment:
	def __init__(self):
		self.file = sys.stdout
		self.execute = True
		self.filter = []

	def run(self, command):
		try:
			p = subprocess.Popen(
				command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
			(stdout, stderr) = p.communicate()
		except:
			return "Failed to run " + command
		return stderr+stdout

	def collectData(self):
		if platform.system() != 'Windows':
			print >>self.file, "glxinfo:", '@@@' + encode(self.run('glxinfo'))
		if platform.system() == 'Linux':
			print >>self.file, "lspci:", '@@@' + encode(self.run('lspci'))


class Test:
	ignoreErrors = []
	sleep = 0

	def __init__(self):
		pass

	def run(self):
		raise NotImplementedError

	def doRun(self, env, path):
		# Filter
		if len(env.filter) > 0:
			if not True in map(lambda f: f.search(path) != None, env.filter):
				return None

		# Run the test
		if env.execute:
			try:
				print "Test: %(path)s" % locals()
				time_start = time.time()
				result = self.run()
				time_end = time.time()
				if 'time' not in result:
					result['time'] = time_end - time_start
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

			if result['result'] != 'pass':
				print "    result: %(result)s" % { 'result': result['result'] }

			result.write(env.file, path)
			if Test.sleep:
				time.sleep(Test.sleep)
		else:
			print "Dry-run: %(path)s" % locals()

	# Returns True iff the given error message should be ignored
	def isIgnored(self, error):
		for pattern in Test.ignoreErrors:
			if pattern.search(error):
				return True

		return False

	# Default handling for stderr messages
	def handleErr(self, results, err):
		errors = filter(lambda s: len(s) > 0, map(lambda s: s.strip(), err.split('\n')))

		ignored = [s for s in errors if self.isIgnored(s)]
		errors = [s for s in errors if s not in ignored]

		if len(errors) > 0:
			results['errors'] = errors

			if results['result'] == 'pass':
				results['result'] = 'warn'

		if len(ignored) > 0:
			results['errors_ignored'] = ignored


class Group(dict):
	def doRun(self, env, path):
		for sub in self:
			spath = sub
			if len(path) > 0:
				spath = path + '/' + spath
			self[sub].doRun(env, spath)


class TestProfile:
	def __init__(self):
		self.tests = Group()
		self.sleep = 0

	def run(self, env):
		time_start = time.time()
		self.tests.doRun(env, '')
		time_end = time.time()
		print >>env.file, "time:",(time_end-time_start)

#############################################################################
##### Loaders
#############################################################################

def loadTestProfile(filename):
	try:
		ns = {
			'__file__': filename
		}
		execfile(filename, ns)
		return ns['profile']
	except:
		traceback.print_exc()
		raise Exception('Could not read tests profile')

def loadTestResults(path, PreferSummary=False):
	try:
		mode = os.stat(path)[stat.ST_MODE]
		testrun = TestrunResult()
		if stat.S_ISDIR(mode):
			testrun.parseDir(path, PreferSummary)
		else:
			file = open(path, 'r')
			testrun.parseFile(file)
			file.close()

		if len(testrun.name) == 0:
			if path[-1] == '/':
				testrun.name = os.path.basename(path[0:-1])
			else:
				testrun.name = os.path.basename(path)

		return testrun
	except:
		traceback.print_exc()
		raise Exception('Could not read tests results')
