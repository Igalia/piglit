# coding=utf-8
# Copyright (c) 2015, 2019 Intel Corporation

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

"""Exception and error classes for piglit, and exception handlers."""

import sys
import functools

__all__ = [
    'PiglitInternalError',
    'PiglitFatalError',
    'PiglitException',
    'PiglitAbort',
    'handler',
]



def handler(func):
    """Decorator function for handling errors in an entry point.

    This will handle expected errors (PiglitFatalError), and unexpected errors,
    either PiglitInternalErrors or PiglitExceptions, as well as handling
    generic Exceptions

    """

    @functools.wraps(func)
    def _inner(*args, **kwargs):
        try:
            return func(*args, **kwargs)
        except PiglitFatalError as e:
            print('Fatal Error: {}'.format(str(e)), file=sys.stderr)
            sys.exit(e.exitcode)
        except PiglitAbort as e:
            print('Aborting Piglit execution: {}'.format(str(e)),
                  file=sys.stderr)
            sys.exit(e.exitcode)
        except PiglitUserError as e:
            print('User error: {}'.format(str(e)), file=sys.stderr)
            sys.exit(e.exitcode)

    return _inner


class PiglitBaseException(Exception):

    """Base class for all piglit exceptions."""

    def __init__(self, *args, exitcode=1, **kwargs):
        super().__init__(*args, **kwargs)
        self.exitcode = exitcode


class PiglitException(PiglitBaseException):
    """Class for non-error exceptions.

    These should *always* be caught. If this class (or any subclass) is
    uncaught that is a bug in piglit.

    """

    def __str__(self):
        return ('An internal exception that should have been handled was not:'
                '\n{}'.format(super(PiglitException, self).__str__()))


class PiglitInternalError(PiglitBaseException):
    """Class for errors in piglit.

    These should always be handled.

    """

    def __str__(self):
        return 'An internal error occurred:\n{}'.format(
            super(PiglitInternalError, self).__str__())


class PiglitFatalError(PiglitBaseException):
    """Class for errors in piglit that cannot be recovered from.

    When this class (or a subclass) is raised it should be raised all the way
    to the top of the program where it exits.

    """


class PiglitUserError(PiglitBaseException):
    """Class for user configuration errors.

    When this class (or a subclass) is raised it should be raised all the way
    to the top of the program where it exits.
    """


class PiglitAbort(PiglitBaseException):
    """Class for non-errors that require piglit aborting.

    When this class (or a subclass) is raised it should be raised all the way
    to the top of the program where it exits.

    """
    def __init__(self, *args, exitcode=3, **kwargs):
        super().__init__(*args, exitcode=exitcode, **kwargs)
