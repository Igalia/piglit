# Copyright (c) 2013 Intel Corporation
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

# This code is based on the MIT licensed code by Emilio Monti found here:
# http://code.activestate.com/recipes/577187-python-thread-pool/

from Queue import Queue
from threading import Thread


class Worker(Thread):
    """
    Simple worker thread

    This worker simply consumes tasks off of the queue until it is empty and
    then waits for more tasks.
    """

    def __init__(self, queue):
        Thread.__init__(self)
        self.queue = queue
        self.daemon = True
        self.start()

    def run(self):
        """ This method is called in the constructor by self.start() """
        while True:
            func, args = self.queue.get()
            func(*args)  # XXX: Does this need to be try/except-ed?
            self.queue.task_done()


class ThreadPool(object):
    """
    A simple ThreadPool class that maintains a Queue object and a set of Worker
    threads.
    """

    def __init__(self, thread_count):
        self.queue = Queue(thread_count)
        self.threads = [Worker(self.queue) for _ in xrange(thread_count)]

    def add(self, func, args):
        """ Add a function and it's arguments to the queue as a tuple """
        self.queue.put((func, args))

    def join(self):
        """ Block until self.queue is empty """
        self.queue.join()
