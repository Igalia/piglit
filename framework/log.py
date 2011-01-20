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

from patterns import Singleton
import logging

class Logger(Singleton):
	def __logMessage(self, logfunc, message, **kwargs):
		[logfunc(line, **kwargs) for line in message.split('\n')]

	def getLogger(self, channel = None):
		if 0 == len(logging.root.handlers):
			logging.basicConfig(
				format = "[%(asctime)s] :: %(message)+8s :: %(name)s",
				datefmt = "%c",
				level = logging.INFO,
			)
		if channel is None:
			channel = "base"
		logger = logging.getLogger(channel)
		return logger

	def log(self, type = logging.INFO, msg = "", channel = None):
		self.__logMessage(lambda m, **kwargs: self.getLogger(channel).log(type, m, **kwargs), msg)

log = Logger().log
