# Copyright (c) 2014 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


""" Base classes for backends

This module provides mixins and base classes for backend modules.

"""

import os
import abc


class FSyncMixin(object):
    """ Mixin class that adds fsync support

    This class provides an init method that sets self._file_sync from a keyword
    arugment file_fsync, and then provides an _fsync() method that does an
    fsync if self._file_sync is truthy

    """
    def __init__(self, file_fsync=False, **options):
        self._file_sync = file_fsync

    def _fsync(self, file_):
        """ Sync the file to disk

        If self._fsync is truthy this will sync self._file to disk

        """
        file_.flush()

        if self._file_sync:
            os.fsync(file_.fileno())


class Backend(object):
    """ Abstract base class for summary backends

    This class provides an abstract ancestor for classes implementing backends,
    providing a light public API. The goal of this API is to be "just enough",
    not a generic writing solution. To that end it provides two public methods,
    'finalize', and 'write_test'. These two methods are designed to be just
    enough to write a backend without needing format specific options.

    Any locking that is necessary should be done in the child classes, as not
    all potential backends need locking (for example, a SQL based backend might
    be thread safe and not need to be locked during write)

    """
    __metaclass__ = abc.ABCMeta

    @abc.abstractmethod
    def __init__(self, dest, metadata, **options):
        """ Generic constructor

        This method should setup the container and open any files or conections
        as necissary. It should not however, write anything into the backend
        store, that job is for the iniitalize method.

        In addition it takes keyword arguments that define options for the
        backends. options should be prefixed to identify which backends they
        apply to. For example, a json specific value should be passed as
        json_*, while a file specific value should be passed as file_*)

        Arguments:
        dest -- the place to write the results to. This should be correctly
                handled based on the backend, the example is calls open() on a
                file, but other backends might want different options

        """

    @abc.abstractmethod
    def initialize(self, metadata):
        """ Write initial metadata and setup

        This method is used to write metadata into the backend store and do any
        other initial backend writing that is required. This method and the
        finalize() method are bookends, one starts, the other finishes.

        Arguments:
        metadata -- a dict or dict-like object that contains metadata to be
                    written into the backend

        """

    @abc.abstractmethod
    def finalize(self, metadata=None):
        """ Write final metadata into to the store and close it

        This method writes any final metadata into the store, what can be
        written is implementation specific, backends are free to ignore any
        data that is not applicable.

        metadata is not required, and Backend derived classes need to handle
        being passed None correctly.

        Keyword Arguments:
        metadata -- Any metadata to be written in after the tests, should be a
                    dict or dict-like object


        """

    @abc.abstractmethod
    def write_test(self, name, data):
        """ Write a test into the backend store

        This method writes an actual test into the backend store.

        Arguments:
        name -- the name of the test to be written
        data -- a TestResult object representing the test data

        """
