#
# Copyright (c) 2010 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#

from threadpool import ThreadPool, WorkRequest
from patterns import Singleton
from threading import RLock
from weakref import WeakKeyDictionary

def synchronized_self(function):
	'''
		A decorator function for providing multithreaded, synchronized access
		amongst one or more functions within a class instance.
	'''
	def wrapper(self, *args, **kwargs):
		synchronized_self.locks.setdefault(self, RLock()).acquire()
		try:
			return function(self, *args, **kwargs)
		finally:
			synchronized_self.locks[self].release()
	return wrapper

synchronized_self.locks = WeakKeyDictionary() # track the locks for each instance

class ThreadPools(Singleton):
	@synchronized_self
	def init(self):
		self.threadpools = dict() # {name : threadpool}
		self.create("base", 1)

	@synchronized_self
	def lookup(self, name):
		return self.threadpools.get(name, None)

	@synchronized_self
	def create(self, name, count = 2):
		if self.lookup(name) is None:
			self.threadpools[name] = ThreadPool(count)
		else:
			raise RuntimeWarning, "ThreadPool '%s' has already been created." % name

	@synchronized_self
	def put(self, callable_, args = None, kwds = None, name = "base"):
		pool = self.lookup(name)
		if pool is None:
			raise RuntimeError, "ThreadPool '%s' does not exist." % name
		pool.putRequest(
			WorkRequest(
				callable_, args = args, kwds = kwds
			)
		)

	def joinAll(self):
		pools = list(self.threadpools.itervalues())
		try:
			for pool in pools:
				pool.wait()
		except KeyboardInterrupt:
			pools = list(self.threadpools.itervalues())
			for pool in pools:
				pool.dismissWorkers(len(pool.workers))
