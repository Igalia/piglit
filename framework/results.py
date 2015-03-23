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

""" Module for results generation """

from __future__ import print_function, absolute_import
import framework.status as status

__all__ = [
    'TestrunResult',
    'TestResult',
]


class TestResult(dict):
    def recursive_update(self, dictionary):
        """ Recursively update the TestResult

        The problem with using self.update() is this:
        >>> t = TestResult()
        >>> t.update({'subtest': {'test1': 'pass'}})
        >>> t.update({'subtest': {'test2': 'pass'}})
        >>> t['subtest']
        {'test2': 'pass'}

        This function is different, because it recursively updates self, it
        doesn't clobber existing entires in the same way
        >>> t = TestResult()
        >>> t.recursive_update({'subtest': {'test1': 'pass'}})
        >>> t.recursive_update({'subtest': {'test2': 'pass'}})
        >>> t['subtest']
        {'test1': 'pass', 'test2': 'pass'}

        Arguments:
        dictionary -- a dictionary instance to update the TestResult with

        """
        def update(d, u, check):
            for k, v in u.iteritems():
                if isinstance(v, dict):
                    d[k] = update(d.get(k, {}), v, True)
                else:
                    if check and k in d:
                        print("Warning: duplicate subtest: {} value: {} old value: {}".format(k, v, d[k]))
                    d[k] = v
            return d

        update(self, dictionary, False)

    @classmethod
    def load(cls, res):
        """Load an already generated result.

        This is used as an alternate constructor which converts an existing
        dictionary into a TestResult object. It converts a key 'result' into a
        status.Status object

        """
        result = cls(res)

        # Replace the result with a status object. 'result' is a required key
        # for results, so don't do any checking. This should fail if it doesn't
        # exist.
        result['result'] = status.status_lookup(result['result'])

        return result


class TestrunResult(object):
    def __init__(self):
        self.name = None
        self.uname = None
        self.options = None
        self.glxinfo = None
        self.wglinfo = None
        self.lspci = None
        self.time_elapsed = None
        self.tests = {}
