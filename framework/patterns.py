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

import threading

class Singleton(object):
	'''
		Modeled after http://www.python.org/download/releases/2.2.3/descrintro/*__new__

		A thread-safe (mostly -- see NOTE) Singleton class pattern.

		NOTE: deleting a singleton instance (i.e. Singleton::delInstance) does not guarantee that something
		else is currently using it. To reduce this risk, a program should not hold a reference to the
		instance.  Rather, use the create/construct syntax (see example below) to access the instance.  Yet,
		this still does not guarantee that this type of usage will result in a desired effect in a
		multithreaded program.
		You've been warned so use the singleton pattern wisely!

		Example:

		class MySingletonClass(Singleton):
			def init(self):
				print "in MySingletonClass::init()", self

			def foo(self):
				print "in MySingletonClass::foo()", self

		MySingletonClass().foo()
		MySingletonClass().foo()
		MySingletonClass().foo()

		---> output will look something like this:
		in MySingletonClass::init() <__main__.MySingletonClass object at 0x7ff5b322f3d0>
		in MySingletonClass::foo() <__main__.MySingletonClass object at 0x7ff5b322f3d0>
		in MySingletonClass::foo() <__main__.MySingletonClass object at 0x7ff5b322f3d0>
		in MySingletonClass::foo() <__main__.MySingletonClass object at 0x7ff5b322f3d0>
	'''

	lock = threading.RLock()

	def __new__(cls, *args, **kwargs):
		try:
			cls.lock.acquire()
			it = cls.__dict__.get('__it__')
			if it is not None:
				return it
			cls.__it__ = it = object.__new__(cls)
			it.init(*args, **kwargs)
			return it
		finally: # this always gets called, even when returning from within the try block
			cls.lock.release()

	def init(self, *args, **kwargs):
		'''
			Derived classes should override this method to do its initializations
			The derived class should not implement a '__init__' method.
		'''
		pass

	@classmethod
	def delInstance(cls):
		cls.lock.acquire()
		try:
			if cls.__dict__.get('__it__') is not None:
				del cls.__it__
		finally:
			cls.lock.release()
