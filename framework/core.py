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
import json
import os
import platform
import stat
import subprocess
import sys
import time
import traceback
from log import log
from cStringIO import StringIO
from textwrap import dedent
from threads import ConcurrentTestPool
from threads import synchronized_self
import threading

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
	'testBinDir',
	'ResultFileInOldFormatError',
]

class JSONWriter:
	'''
	Writes to a JSON file stream

	JSONWriter is threadsafe.

	Example
	-------

	This call to ``json.dump``::
	    json.dump(
		{
		    'a': [1, 2, 3],
		    'b': 4,
		    'c': {
		        'x': 100,
		    },
		}
		file,
		indent=JSONWriter.INDENT)

	is equivalent to::
	    w = JSONWriter(file)
	    w.open_dict()
	    w.write_dict_item('a', [1, 2, 3])
	    w.write_dict_item('b', 4)
	    w.write_dict_item('c', {'x': 100})
	    w.close_dict()

	which is also equivalent to::
	    w = JSONWriter(file)
	    w.open_dict()
	    w.write_dict_item('a', [1, 2, 3])
	    w.write_dict_item('b', 4)

	    w.write_dict_key('c')
	    w.open_dict()
	    w.write_dict_item('x', 100)
	    w.close_dict()

	    w.close_dict()
	'''

	INDENT = 4

	def __init__(self, file):
		self.file = file
		self.__indent_level = 0
		self.__inhibit_next_indent = False
		self.__encoder = json.JSONEncoder(indent=self.INDENT)

		# self.__is_collection_empty
		#
		# A stack that indicates if the currect collection is empty
		#
		# When open_dict is called, True is pushed onto the
		# stack. When the first element is written to the newly
		# opened dict, the top of the stack is set to False.
		# When the close_dict is called, the stack is popped.
		#
		# The top of the stack is element -1.
		#
		# XXX: How does one attach docstrings to member variables?
		#
		self.__is_collection_empty = []

	@synchronized_self
	def __write_indent(self):
		if self.__inhibit_next_indent:
			self.__inhibit_next_indent = False
			return
		else:
			i = ' ' * self.__indent_level * self.INDENT
			self.file.write(i)

	@synchronized_self
	def __write(self, obj):
		lines = list(self.__encoder.encode(obj).split('\n'))
		n = len(lines)
		for i in range(n):
			self.__write_indent()
			self.file.write(lines[i])
			if i != n - 1:
				self.file.write('\n')

	@synchronized_self
	def open_dict(self):
		self.__write_indent()
		self.file.write('{')

		self.__indent_level += 1
		self.__is_collection_empty.append(True)

	@synchronized_self
	def close_dict(self, comma=True):
		self.__indent_level -= 1
		self.__is_collection_empty.pop()

		self.file.write('\n')
		self.__write_indent()
		self.file.write('}')

	@synchronized_self
	def write_dict_item(self, key, value):
		# Write key.
		self.write_dict_key(key)

		# Write value.
		self.__indent_level += 1
		self.__write(value)
		self.__indent_level -= 1

	@synchronized_self
	def write_dict_key(self, key):
		# Write comma if this is not the initial item in the dict.
		if self.__is_collection_empty[-1]:
			self.__is_collection_empty[-1] = False
		else:
			self.file.write(',')

		self.file.write('\n')
		self.__write(key)
		self.file.write(': ')

		self.__inhibit_next_indent = True

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

if 'PIGLIT_BUILD_DIR' in os.environ:
    testBinDir = os.environ['PIGLIT_BUILD_DIR'] + '/bin/'
else:
    testBinDir = os.path.dirname(__file__) + '/../bin/'


#############################################################################
##### Result classes
#############################################################################

class TestResult(dict):
	pass

class GroupResult(dict):
	def get_subgroup(self, path, create=True):
		'''
		Retrieve subgroup specified by path

		For example, ``self.get_subgroup('a/b/c')`` will attempt to
		return ``self['a']['b']['c']``. If any subgroup along ``path``
		does not exist, then it will be created if ``create`` is true;
		otherwise, ``None`` is returned.
		'''
		group = self
		for subname in path.split('/'):
			if subname not in group:
				if create:
					group[subname] = GroupResult()
				else:
					return None
			group = group[subname]
			assert(isinstance(group, GroupResult))
		return group

	@staticmethod
	def make_tree(tests):
		'''
		Convert a flat dict of test results to a hierarchical tree

		``tests`` is a dict whose items have form ``(path, TestResult)``,
		where path is a string with form ``group1/group2/.../test_name``.

		Return a tree whose leaves are the values of ``tests`` and
		whose nodes, which have type ``GroupResult``, reflect the
		paths in ``tests``.
		'''
		root = GroupResult()

		for (path, result) in tests.items():
			group_path = os.path.dirname(path)
			test_name = os.path.basename(path)

			group = root.get_subgroup(group_path)
			group[test_name] = TestResult(result)

		return root

class ResultFileInOldFormatError(Exception):
	def __init__(self, filepath):
		super(Exception, self).__init__(filepath)
		self.filepath = filepath

class TestrunResult:
	def __init__(self):
		self.serialized_keys = [
			'name',
			'tests',
			'glxinfo',
			'lspci',
			'time_elapsed',
			]
		self.name = None
		self.glxinfo = None
		self.lspci = None
		self.tests = {}

	def __checkFileIsNotInOldFormat(self, file):
		'''
		If file contains the old, custom format, then raise
		``ResultFileInOldFormatError``.

		:return: None
		'''
		saved_position = file.tell()
		first_line = file.readline()
		if first_line.startswith('name:'):
			raise ResultFileInOldFormatError(file.name)
		file.seek(saved_position)

	def __repairFile(self, file):
		'''
		Reapair JSON file if necessary

                If the JSON file is not closed properly, perhaps due a system
                crash during a test run, then the JSON is repaired by
                discarding the trailing, incomplete item and appending braces
                to the file to close the JSON object.

                The repair is performed on a string buffer, and the given file
                is never written to. This allows the file to be safely read
                during a test run.

                :return: If no repair occured, then ``file`` is returned.
                    Otherwise, a new file object containing the repaired JSON
                    is returned.
		'''

                saved_position = file.tell()
		lines = file.readlines()
                file.seek(saved_position)

		if lines[-1] == '}':
			# JSON object was closed properly. No repair is
			# necessary.
			return file

		# JSON object was not closed properly.
		#
		# To repair the file, we execute these steps:
		#   1. Find the closing brace of the last, properly written
		#      test result.
		#   2. Discard all subsequent lines.
		#   3. Remove the trailing comma of that test result.
		#   4. Append enough closing braces to close the json object.
		#   5. Return a file object containing the repaired JSON.

		# Each non-terminal test result ends with this line:
		safe_line =  3 * JSONWriter.INDENT * ' ' + '},\n'

		# Search for the last occurence of safe_line.
		safe_line_num = None
		for i in range(-1, - len(lines), -1):
			if lines[i] == safe_line:
				safe_line_num = i
				break

		if safe_line_num is None:
			raise Exception('failed to repair corrupt result file: ' + file.name)

		# Remove corrupt lines.
		lines = lines[0:(safe_line_num + 1)]

		# Remove trailing comma.
		lines[-1] = 3 * JSONWriter.INDENT * ' ' + '}\n'

		# Close json object.
		lines.append(JSONWriter.INDENT * ' ' + '}\n')
		lines.append('}')

                # Return new file object containing the repaired JSON.
                new_file = StringIO()
                new_file.writelines(lines)
                new_file.flush()
                new_file.seek(0)
                return new_file

	def write(self, file):
		# Serialize only the keys in serialized_keys.
		keys = set(self.__dict__.keys()).intersection(self.serialized_keys)
		raw_dict = dict([(k, self.__dict__[k]) for k in keys])
		json.dump(raw_dict, file, indent=JSONWriter.INDENT)

	def parseFile(self, file):
		self.__checkFileIsNotInOldFormat(file)
		file = self.__repairFile(file)
		raw_dict = json.load(file)

		# Check that only expected keys were unserialized.
		for key in raw_dict:
			if key not in self.serialized_keys:
				raise Exception('unexpected key in results file: ' + str(key))

		self.__dict__.update(raw_dict)

		# Replace each raw dict in self.tests with a TestResult.
		for (path, result) in self.tests.items():
			self.tests[path] = TestResult(result)

#############################################################################
##### Generic Test classes
#############################################################################

class Environment:
	def __init__(self):
		# If disabled, runs all tests serially from the main thread.
		self.concurrent = True
		# Set when we want doRun to only run a test if it's concurrent.
		self.run_concurrent = True
		self.execute = True
		self.filter = []
		self.exclude_filter = []

	def run(self, command):
		try:
			p = subprocess.Popen(
				command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
			(stdout, stderr) = p.communicate()
		except:
			return "Failed to run " + command
		return stderr+stdout

	def collectData(self):
		result = {}
		if platform.system() != 'Windows':
			result['glxinfo'] = self.run('glxinfo')
		if platform.system() == 'Linux':
			result['lspci'] = self.run('lspci')
		return result

class Test:
	ignoreErrors = []
	sleep = 0

	def __init__(self, runConcurrent = False):
		'''
			'runConcurrent' controls whether this test will
			execute it's work (i.e. __doRunWork) on the calling thread
			(i.e. the main thread) or from the ConcurrentTestPool threads.
		'''
		self.runConcurrent = runConcurrent

	def run(self):
		raise NotImplementedError

	def doRun(self, env, path, json_writer):
		'''
		Schedule test to be run

		:path:
		    Fully qualified test name as a string.  For example,
		    ``spec/glsl-1.30/preprocessor/compiler/keywords/void.frag``.

		:testrun:
		    A TestrunResult object that accumulates test results.
		    After this test has executed, the test's ``TestResult`` is
		    assigned to ``testrun.tests[path]``
		'''
		args = (env, path, json_writer)
		if env.run_concurrent:
			if self.runConcurrent:
				ConcurrentTestPool().put(self.__doRunWork, args=args)
		else:
			if not env.concurrent or not self.runConcurrent:
				self.__doRunWork(*args)

	def __doRunWork(self, env, path, json_writer):
		# Exclude tests that don't match the filter regexp
		if len(env.filter) > 0:
			if not True in map(lambda f: f.search(path) != None, env.filter):
				return None

		# And exclude tests that do match the exclude_filter regexp
		if len(env.exclude_filter) > 0:
			if True in map(lambda f: f.search(path) != None, env.exclude_filter):
				return None

		def status(msg):
			log(msg = msg, channel = path)

		# Run the test
		if env.execute:
			try:
				status("running")
				time_start = time.time()
				result = self.run()
				time_end = time.time()
				if 'time' not in result:
					result['time'] = time_end - time_start
				if 'result' not in result:
					result['result'] = 'fail'
				if not isinstance(result, TestResult):
					result = TestResult(result)
					result['result'] = 'warn'
					result['note'] = 'Result not returned as an instance of TestResult'
			except:
				result = TestResult()
				result['result'] = 'fail'
				result['exception'] = str(sys.exc_info()[0]) + str(sys.exc_info()[1])
				result['traceback'] = "".join(traceback.format_tb(sys.exc_info()[2]))

			status(result['result'])

			json_writer.write_dict_item(path, result)
			if Test.sleep:
				time.sleep(Test.sleep)
		else:
			status("dry-run")

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
	def doRun(self, env, path, json_writer):
		'''
		Schedule all tests in group for execution.

		See ``Test.doRun``.
		'''
		for sub in sorted(self):
			spath = sub
			if len(path) > 0:
				spath = path + '/' + spath
			self[sub].doRun(env, spath, json_writer)


class TestProfile:
	def __init__(self):
		self.tests = Group()
		self.sleep = 0

	def run(self, env, json_writer):
		'''
		Schedule all tests in profile for execution.

		See ``Test.doRun``.
		'''
		json_writer.write_dict_key('tests')
		json_writer.open_dict()
		# queue up the concurrent tests up front, so the pool
		# is filled from the start of the test.
		if env.concurrent:
			env.run_concurrent = True
			self.tests.doRun(env, '', json_writer)
		# Run any remaining non-concurrent tests serially from this
		# thread, while the concurrent tests 
		env.run_concurrent = False
		self.tests.doRun(env, '', json_writer)
		ConcurrentTestPool().join()
		json_writer.close_dict()

	def remove_test(self, test_path):
		"""Remove a fully qualified test from the profile.

		``test_path`` is a string with slash ('/') separated
		components. It has no leading slash. For example::
			test_path = 'spec/glsl-1.30/linker/do-stuff'
		"""

		l = test_path.split('/')
		group = self.tests[l[0]]
		for group_name in l[1:-2]:
			group = group[group_name]
		del group[l[-1]]

#############################################################################
##### Loaders
#############################################################################

def loadTestProfile(filename, resdir):
	ns = {
		'__file__': filename,
		'res_dir': resdir
	}
	try:
		execfile(filename, ns)
	except:
		traceback.print_exc()
		raise Exception('Could not read tests profile')
	return ns['profile']

def loadTestResults(path):
	if os.path.isdir(path):
		filepath = os.path.join(path, 'main')
	else:
		filepath = path

	testrun = TestrunResult()
	try:
		with open(filepath, 'r') as file:
			testrun.parseFile(file)
	except OSError:
		traceback.print_exc()
		raise Exception('Could not read tests results')

	assert(testrun.name is not None)
	return testrun
