# Copyright (c) 2013-2014 Intel Corporation
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

""" Module implementing classes for reading posix dmesg

Currently this module only has the default DummyDmesg, and a LinuxDmesg.
The method used on Linux requires that timetamps are enabled, and no other
posix system has timestamps.

On OSX and *BSD one would likely want to implement a system that reads the
sysloger, since timestamps can be added by the sysloger, and are not inserted
by dmesg.

"""

import re
import sys
import subprocess
import warnings

__all__ = ['LinuxDmesg', 'DummyDmesg', 'get_dmesg']


class LinuxDmesg(object):
    """ Read dmesg on posix systems

    This reads the dmesg ring buffer, stores the contents of the buffer, and
    then compares it whenever called, returning the difference between the
    calls. It requires timestamps to be enabled.

    """
    DMESG_COMMAND = ['dmesg', '--level', 'emerg,alert,crit,err,warn,notice']

    def __init__(self):
        """ Create a dmesg instance """
        self._last_message = None
        self._new_messages = []
        self.regex = None

        # Populate self.dmesg initially, otherwise the first test will always
        # be full of dmesg crud.
        self.update_dmesg()

        if not self._last_message:
            # We need to ensure that there is something in dmesg to check
            # timestamps.
            warnings.warn("Cannot check dmesg for timestamp support. If you "
                          "do not have timestamps enabled in your kernel you "
                          "get incomplete dmesg captures", RuntimeWarning)
        elif not re.match(r'\[\s*\d+\.\d+\]', self._last_message):
            # Do an initial check to ensure that dmesg has timestamps, we need
            # timestamps to work correctly. A proper linux dmesg timestamp
            # looks like this: [    0.00000]
            raise DmesgError(
                "Your kernel does not seem to support timestamps, which "
                "are required by the --dmesg option")

    def update_result(self, result):
        """ Takes a TestResult object and updates it with dmesg statuses

        If dmesg is enabled, and if dmesg has been updated, then replace pass
        with dmesg-warn and warn and fail with dmesg-fail. If dmesg has not
        been updated, it will return the original result passed in.

        Arguments:
        result -- A TestResult instance

        """

        def replace(res):
            """ helper to replace statuses with the new dmesg status

            Replaces pass with dmesg-warn, and warn and fail with dmesg-fail,
            otherwise returns the input

            """
            return {"pass": "dmesg-warn",
                    "warn": "dmesg-fail",
                    "fail": "dmesg-fail"}.get(res, res)

        # Get a new snapshot of dmesg
        self.update_dmesg()

        # if update_dmesg() found new entries replace the results of the test
        # and subtests
        if self._new_messages:

            if self.regex:
                for line in self._new_messages:
                    if self.regex.search(line):
                        break
                else:
                    return result

            result['result'] = replace(result['result'])

            # Replace any subtests
            if 'subtest' in result:
                for key, value in result['subtest'].iteritems():
                    result['subtest'][key] = replace(value)

            # Add the dmesg values to the result
            result['dmesg'] = "\n".join(self._new_messages)

        return result

    def update_dmesg(self):
        """ Call dmesg using subprocess.check_output

        Get the contents of dmesg, then calculate new messages, finally set
        self.dmesg equal to the just read contents of dmesg.

        """
        dmesg = subprocess.check_output(self.DMESG_COMMAND).strip().splitlines()

        # Find all new entries, do this by slicing the list of dmesg to only
        # returns elements after the last element stored. If there are not
        # matches a value error is raised, that means all of dmesg is new
        l = 0
        for index, item in enumerate(reversed(dmesg)):
            if item == self._last_message:
                l = len(dmesg) - index  # don't include the matched element
                break
        self._new_messages = dmesg[l:]

        # Attempt to store the last element of dmesg, unless there was no dmesg
        self._last_message = dmesg[-1] if dmesg else None


class DummyDmesg(object):
    """ An dummy class for dmesg on non unix-like systems

    This implements a dummy version of the LinuxDmesg, and can be used anytime
    it is not desirable to actually read dmesg, such as non-posix systems, or
    when the contents of dmesg don't matter.

    """
    DMESG_COMMAND = []

    def __init__(self):
        pass

    def update_dmesg(self):
        """ Dummy version of update_dmesg """
        pass

    def update_result(self, result):
        """ Dummy version of update_result """
        return result


class DmesgError(EnvironmentError):
    pass


def get_dmesg(not_dummy=True):
    """ Return a Dmesg type instance

    Normally this does a system check, and returns the type that proper for
    your system. However, if Dummy is True then it will always return a
    DummyDmesg instance.

    """
    if sys.platform.startswith('linux') and not_dummy:
        return LinuxDmesg()
    return DummyDmesg()
