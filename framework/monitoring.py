# coding=utf-8
# Copyright (c) 2016 Intel Corporation
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
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

"""Module implementing classes for monitoring

This module provides a set of classes for monitoring the system on multiple
sources. The sources can be standard files, locked files or the dmesg.
The patterns of the errors are defined in piglit.conf, at the section
monitored-errors.

When one of the regex is found in the corresponding source Piglit will abort
with code 3.

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import abc
import errno
import os
import re

import six

from framework.core import PIGLIT_CONFIG
from framework.dmesg import LinuxDmesg
from framework import exceptions

__all__ = [
    'BaseMonitoring',
    'Monitoring',
    'MonitoringFile',
    'MonitoringLinuxDmesg',
]



class Monitoring(object):
    """Class that initialize and update monitoring classes

    This reads the rules from the section monitored-errors in piglit.conf
    and initializes the monitoring objects. Their type must be derived from
    BaseMonitoring and specialized according the field 'type'.

    """

    # starting time: user must know current machine/GPU state before
    # starting piglit: we can resume test execution based on last stopping
    # if we consider that our system is still in good shape (ie not blocking
    # run forever)
    _abort_error = None
    _monitoring_rules = None

    def __init__(self, monitoring_enabled):
        """Create a LinuxMonitored instance"""
        # Get the monitoring rules from piglit.conf and store them into a dict.
        self._monitoring_rules = {}

        if monitoring_enabled and PIGLIT_CONFIG.has_section('monitored-errors'):
            for key, _ in PIGLIT_CONFIG.items('monitored-errors'):
                if PIGLIT_CONFIG.has_section(key):
                    type = PIGLIT_CONFIG.required_get(key, 'type')
                    regex = PIGLIT_CONFIG.required_get(key, 'regex')
                    parameters = PIGLIT_CONFIG.required_get(key, 'parameters')

                    self.add_rule(key, type, parameters, regex)

    @property
    def abort_needed(self):
        """Simply return if _abort_error variable is not empty"""
        return self._abort_error is not None

    @property
    def error_message(self):
        """Simply return _abort_error message"""
        return self._abort_error

    def add_rule(self, key, type, parameters, regex):
        """Add a new monitoring rule

        This method adds a new monitoring rule. The type must be file,
        locked_file or dmesg.

        Arguments:
        key -- The key for storing the rule in a dict
        type -- The type of monitoring
        parameters -- Depending on the type, can be a file path or a command
                      options
        regex -- The rule regex

        """
        rule = None

        if type == 'file':
            rule = MonitoringFile(parameters,
                                  regex)
        elif type == 'locked_file':
            rule = MonitoringFile(parameters,
                                  regex, True)
        elif type == 'dmesg':
            rule = MonitoringLinuxDmesg(parameters,
                                        regex)
        else:
            raise exceptions.PiglitFatalError(
                "No available monitoring class for the type {}.".format(type))

        self._monitoring_rules[key] = rule

    def delete_rule(self, key):
        """Remove a monitoring rule

        Arguments:
        key -- The rule key

        """
        self._monitoring_rules.pop(key, None)

    def update_monitoring(self):
        """Update the new messages for each monitoring object"""
        if self._monitoring_rules:
            for monitoring_rule in six.itervalues(self._monitoring_rules):
                monitoring_rule.update_monitoring()

    def check_monitoring(self):
        """Check monitoring objects statue

        This method checks the state for each monitoring object.
        If one of them found the pattern in the new messages,
        set itself on abort_needed state.

        """
        # Get a new snapshot of the source
        self.update_monitoring()

        if self._monitoring_rules:
            for rule_key, monitoring_rule in six.iteritems(
                    self._monitoring_rules):
                # Get error message
                self._abort_error = monitoring_rule.check_monitoring()
                # if error message is not empty, abort is requested
                if self.abort_needed:
                    self._abort_error = "From the rule {}:\n{}".format(
                        rule_key,
                        self._abort_error)
                    break


@six.add_metaclass(abc.ABCMeta)
class BaseMonitoring(object):
    """Abstract base class for Monitoring derived objects

    This provides the bases of the constructor and most subclasses should call
    super() in __init__(). It provides a concrete implementation of the
    check_monitoring() method, which should be suitible for all subclasses.

    The update_monitoring() method need to be override for all subclasses.

    """
    @abc.abstractmethod
    def __init__(self, monitoring_source, regex):
        """Abstract constructor for BaseMonitoring subclasses

        Arguments;
        monitoring_source -- The source to monitor
        regex -- The errors pattern

        """
        self._monitoring_source = monitoring_source
        self._monitoring_regex = re.compile(regex)
        self._new_messages = ''

    @property
    def new_messages(self):
        """Simply return if _abort_error variable is not empty"""
        return self._new_messages

    @abc.abstractmethod
    def update_monitoring(self):
        """Update _new_messages list

        This method should read a monitoring source like a file or a program
        output, determine new entries since the last read of the source,
        and update self._new_messages with those entries

        """
        pass

    def check_monitoring(self):
        """Check _new_messages

        This method checks if the regex is found in the new messages,
        then return a string that contain the monitored location and
        the matched line

        """
        if self._new_messages and self._monitoring_regex:
            for line in self._new_messages:
                if self._monitoring_regex.search(line):
                    return line


if os.name == 'posix':
    import fcntl


    class MonitoringFile(BaseMonitoring):
        """Monitoring from a file

        This class is for monitoring the system from a file that
        can be a standard file or a locked file. The locked file
        algorithm uses fnctl, so it doesn't work on non Unix systems

        Arguments:
        is_locked -- True if the target is a locked file

        """
        _is_locked = False

        def __init__(self, monitoring_source, regex, is_locked=False):
            """Create a MonitoringFile instance"""
            self._last_message = None
            self._is_locked = is_locked
            super(MonitoringFile, self).__init__(monitoring_source, regex)

        def update_monitoring(self):
            """Open the file and get the differences

            Get the contents of the file, then calculate new messages.
            This implements also a specific method for reading locked files.

            """

            try:
                with open(self._monitoring_source, 'r') as f:
                    lines = []
                    if self._is_locked:
                        # Create a duplicated file descriptor, this avoid lock
                        fd = os.dup(f.fileno())
                        # use I/O control for reading the lines
                        fcntl.fcntl(fd, fcntl.F_SETFL, os.O_NONBLOCK)

                        while True:
                            try:
                                line = os.read(fd, 1024)
                                if not line:
                                    break;
                            except OSError as e:
                                if e.errno == errno.EAGAIN:
                                    break
                                else:
                                    raise e
                            lines.append(line.decode("utf-8", "strict"))
                        os.close(fd)

                    else:
                        lines = f.read().splitlines()

                    f.close()

                    # Find all new entries, do this by slicing the list of the
                    # lines to only returns elements after the last element
                    # stored. If there are not matches a value error is raised,
                    # that means all of the lines are new
                    l = 0
                    for index, item in enumerate(reversed(lines)):
                        if item == self._last_message:
                            l = len(lines) - index  # don't include the matched element
                            break
                    self._new_messages = lines[l:]
                    # Attempt to store the last element of lines,
                    # unless there was no line
                    self._last_message = lines[-1] if lines else None
            except Exception:
                # if an error occured, we consider there are no new messages
                self._new_messages = []
                pass


    class MonitoringLinuxDmesg(BaseMonitoring, LinuxDmesg):
        """Monitoring on dmesg

        This class is for monitoring on the system dmesg. It's inherited
        from LinuxDmesg for the dmesg processing methods.

        Work only on Linux operating system.

        """
        def __init__(self, monitoring_source, regex):
            """Create a MonitoringLinuxDmesg instance"""
            self.DMESG_COMMAND = ['dmesg']+monitoring_source.split()
            BaseMonitoring.__init__(self, monitoring_source, regex)
            LinuxDmesg.__init__(self)

        def update_monitoring(self):
            """Call update_dmesg"""
            self.update_dmesg()
