# coding=utf-8
# Copyright (c) 2014-2016 Intel Corporation

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

""" Module implementing a JUnitBackend for piglit """

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os.path
import shutil
try:
    from lxml import etree
except ImportError:
    import xml.etree.cElementTree as etree
try:
    import simplejson as json
except ImportError:
    import json

import six

from framework import grouptools, results, exceptions
from framework.core import PIGLIT_CONFIG
from .abstract import FileBackend
from .register import Registry

__all__ = [
    'REGISTRY',
    'JUnitBackend',
]


_JUNIT_SPECIAL_NAMES = ('api', 'search')


def junit_escape(name):
    name = name.replace('.', '_')
    if name in _JUNIT_SPECIAL_NAMES:
        name += '_'
    return name


class JUnitWriter(object):
    """A class that provides a write mechanism for junit tests."""

    def __init__(self, test_suffix, efail, ecrash):
        self._test_suffix = test_suffix
        self._expected_crashes = ecrash
        self._expected_failures = efail

    @staticmethod
    def _make_names(name):
        """Takes a name from piglit (using grouptools.SEPARATOR and returns a
        split classnam, testname pair in junit format.
        """
        classname, testname = grouptools.splitname(name)
        classname = classname.split(grouptools.SEPARATOR)
        classname = [junit_escape(e) for e in classname]
        classname = '.'.join(classname)

        # Add the test to the piglit group rather than directly to the root
        # group, this allows piglit junit to be used in conjunction with other
        # piglit
        # TODO: It would be nice if other suites integrating with piglit could
        # set different root names.
        classname = 'piglit.' + classname

        return (classname, junit_escape(testname))

    @staticmethod
    def _set_xml_err(element, data, expected_result):
        """Adds the 'system-err' element."""
        err = etree.SubElement(element, 'system-err')
        err.text = data.err
        err.text += '\n\npid: {}\nstart time: {}\nend time: {}\n'.format(
            data.pid, data.time.start, data.time.end)

        if data.result in ['fail', 'dmesg-warn', 'dmesg-fail']:
            if expected_result == "failure":
                err.text += "\n\nWARN: passing test as an expected failure"
            elif expected_result == 'error':
                err.text += \
                    "\n\nERROR: Test should have been crash but was failure"
        elif data.result in ['crash', 'timeout']:
            if expected_result == "error":
                err.text += "\n\nWARN: passing test as an expected crash"
            elif expected_result == 'failure':
                err.text += \
                    "\n\nERROR: Test should have been failure but was crash"
        elif expected_result != "pass":
            err.text += "\n\nERROR: This test passed when it "\
                        "expected {0}".format(expected_result)

    @staticmethod
    def _make_result(element, result, expected_result):
        """Adds the skipped, failure, or error element."""
        res = None
        # If the result is skip, then just add the skipped message and go on
        if result == 'incomplete':
            res = etree.SubElement(element, 'failure',
                                   message='Incomplete run.')
        elif result in ['fail', 'dmesg-warn', 'dmesg-fail']:
            if expected_result == "failure":
                res = etree.SubElement(element, 'skipped',
                                       message='expected failure')
            elif expected_result == 'error':
                res = etree.SubElement(element, 'failure',
                                       message='expected crash, but got '
                                               'failure')
            else:
                res = etree.SubElement(element, 'failure')
        elif result in ['crash', 'timeout']:
            if expected_result == "error":
                res = etree.SubElement(element, 'skipped',
                                       message='expected crash')
            elif expected_result == 'failure':
                res = etree.SubElement(element, 'error',
                                       message='expected failure, but got '
                                               'error')
            else:
                res = etree.SubElement(element, 'error')
        elif expected_result != "pass":
            res = etree.SubElement(element, 'failure',
                                   message="expected {}, but got {}".format(
                                       expected_result, result))
        elif result == 'skip':
            res = etree.SubElement(element, 'skipped')

        # Add the piglit type to the failure result
        if res is not None:
            res.attrib['type'] = six.text_type(result)

    def _make_root(self, testname, classname, data):
        """Creates and returns the root element."""
        element = etree.Element('testcase',
                                name=self._make_full_test_name(testname),
                                classname=classname,
                                # Incomplete will not have a time.
                                time=str(data.time.total),
                                status=str(data.result))

        return element

    def _make_full_test_name(self, testname):
        # Jenkins will display special pages when the test has certain names.
        # https://jenkins-ci.org/issue/18062
        # https://jenkins-ci.org/issue/19810
        # The testname variable is used in the calculate_result closure, and
        # must not have the suffix appended.
        return testname + self._test_suffix

    def _expected_result(self, name):
        """Get the expected result of the test."""
        name = name.replace("=", ".").replace(":", ".")
        expected_result = "pass"

        if name in self._expected_failures:
            expected_result = "failure"
            # a test can either fail or crash, but not both
            assert name not in self._expected_crashes

        if name in self._expected_crashes:
            expected_result = "error"

        return expected_result

    def __call__(self, f, name, data):
        classname, testname = self._make_names(name)
        element = self._make_root(testname, classname, data)
        expected_result = self._expected_result(
            '{}.{}'.format(classname, testname).lower())

        # If this is an incomplete status then none of these values will be
        # available, nor
        if data.result != 'incomplete':
            self._set_xml_err(element, data, expected_result)

            # Add stdout
            out = etree.SubElement(element, 'system-out')
            out.text = data.out

            # Prepend command line to stdout
            out.text = data.command + '\n' + out.text

        self._make_result(element, data.result, expected_result)

        f.write(six.text_type(etree.tostring(element).decode('utf-8')))


class JUnitSubtestWriter(JUnitWriter):
    """A JUnitWriter derived class that treats subtest at testsuites.

    This class will turn a piglit test with subtests into a testsuite element
    with each subtest as a testcase element. This subclass is needed because
    not all JUnit readers (like the JUnit plugin for Jenkins) handle nested
    testsuites correctly.
    """

    def _make_root(self, testname, classname, data):
        if data.subtests:
            testname = '{}.{}'.format(classname, testname)
            element = etree.Element('testsuite',
                                    name=testname,
                                    time=str(data.time.total),
                                    tests=six.text_type(len(data.subtests)))
            for test, result in six.iteritems(data.subtests):
                etree.SubElement(element,
                                 'testcase',
                                 name=self._make_full_test_name(test),
                                 classname=testname,
                                 status=six.text_type(result))

        else:
            element = super(JUnitSubtestWriter, self)._make_root(
                testname, classname, data)
        return element

    def __call__(self, f, name, data):
        classname, testname = self._make_names(name)
        element = self._make_root(testname, classname, data)

        # If this is an incomplete status then none of these values will be
        # available, nor
        if data.result != 'incomplete':
            self._set_xml_err(element, data, 'pass')

            # Add stdout
            out = etree.SubElement(element, 'system-out')
            out.text = data.out
            # Prepend command line to stdout
            out.text = data.command + '\n' + out.text

            if data.subtests:
                for subname, result in six.iteritems(data.subtests):
                    # replace special characters and make case insensitive
                    elem = element.find('.//testcase[@name="{}"]'.format(
                        self._make_full_test_name(subname)))
                    assert elem is not None
                    self._make_result(
                        elem, result,
                        self._expected_result('{}.{}.{}'.format(
                            classname, testname, subname).lower()))
            else:
                self._make_result(element, data.result,
                                  self._expected_result('{}.{}'.format(
                                      classname, testname).lower()))
        else:
            self._make_result(element, data.result,
                              self._expected_result('{}.{}'.format(
                                  classname, testname).lower()))

        f.write(six.text_type(etree.tostring(element).decode('utf-8')))


class JUnitBackend(FileBackend):
    """ Backend that produces ANT JUnit XML

    Based on the following schema:
    https://svn.jenkins-ci.org/trunk/hudson/dtkit/dtkit-format/dtkit-junit-model/src/main/resources/com/thalesgroup/dtkit/junit/model/xsd/junit-7.xsd

    """
    _file_extension = 'xml'
    _write = None  # this silences the abstract-not-subclassed warning

    def __init__(self, dest, junit_suffix='', junit_subtests=False, **options):
        super(JUnitBackend, self).__init__(dest, **options)

        # make dictionaries of all test names expected to crash/fail
        # for quick lookup when writing results.  Use lower-case to
        # provide case insensitive matches.
        expected_failures = {}
        if PIGLIT_CONFIG.has_section("expected-failures"):
            for fail, _ in PIGLIT_CONFIG.items("expected-failures"):
                expected_failures[fail.lower()] = True
        expected_crashes = {}
        if PIGLIT_CONFIG.has_section("expected-crashes"):
            for fail, _ in PIGLIT_CONFIG.items("expected-crashes"):
                expected_crashes[fail.lower()] = True

        if not junit_subtests:
            self._write = JUnitWriter(
                junit_suffix, expected_failures, expected_crashes)
        else:
            self._write = JUnitSubtestWriter(  # pylint: disable=redefined-variable-type
                junit_suffix, expected_failures, expected_crashes)

    def initialize(self, metadata):
        """ Do nothing

        Junit doesn't support restore, and doesn't have an initial metadata
        block to write, so all this method does is create the tests directory

        """
        tests = os.path.join(self._dest, 'tests')
        if os.path.exists(tests):
            shutil.rmtree(tests)
        os.mkdir(tests)

    def finalize(self, metadata=None):
        """ Scoop up all of the individual peices and put them together """
        root = etree.Element('testsuites')
        piglit = etree.Element('testsuite', name='piglit')
        root.append(piglit)
        for each in os.listdir(os.path.join(self._dest, 'tests')):
            with open(os.path.join(self._dest, 'tests', each), 'r') as f:
                # parse returns an element tree, and that's not what we want,
                # we want the first (and only) Element node
                # If the element cannot be properly parsed then consider it a
                # failed transaction and ignore it.
                try:
                    piglit.append(etree.parse(f).getroot())
                except etree.ParseError:
                    continue

        # set the test count by counting the number of tests.
        # This must be unicode (py3 str)
        piglit.attrib['tests'] = str(len(piglit))

        with open(os.path.join(self._dest, 'results.xml'), 'w') as f:
            f.write("<?xml version='1.0' encoding='utf-8'?>\n")
            # lxml has a pretty print we want to use
            if etree.__name__ == 'lxml.etree':
                out = etree.tostring(root, pretty_print=True)
            else:
                out = etree.tostring(root)
            f.write(out.decode('utf-8'))

        shutil.rmtree(os.path.join(self._dest, 'tests'))


def _load(results_file):
    """Load a junit results instance and return a TestrunResult.

    It's worth noting that junit is not as descriptive as piglit's own json
    format, so some data structures will be empty compared to json.

    This tries to not make too many assumptions about the strucuter of the
    JUnit document.

    """
    run_result = results.TestrunResult()

    splitpath = os.path.splitext(results_file)[0].split(os.path.sep)
    if splitpath[-1] != 'results':
        run_result.name = splitpath[-1]
    elif len(splitpath) > 1:
        run_result.name = splitpath[-2]
    else:
        run_result.name = 'junit result'

    tree = etree.parse(results_file).getroot().find('.//testsuite')
    for test in tree.iterfind('testcase'):
        result = results.TestResult()
        # Take the class name minus the 'piglit.' element, replace junit's '.'
        # separator with piglit's separator, and join the group and test names
        name = test.attrib['name']
        if 'classname' in test.attrib:
            name = grouptools.join(test.attrib['classname'], name)
        name = name.replace('.', grouptools.SEPARATOR)
        is_piglit = False
        if name.startswith("piglit"):
            is_piglit = True
            name = name.split(grouptools.SEPARATOR, 1)[1]

        # Remove the trailing _ if they were added (such as to api and search)
        if name.endswith('_'):
            name = name[:-1]

        result.result = test.attrib['status']

        # This is the fallback path, we'll try to overwrite this with the value
        # in stderr
        result.time = results.TimeAttribute()
        if 'time' in test.attrib:
            result.time = results.TimeAttribute(end=float(test.attrib['time']))
        syserr = test.find('system-err')
        if syserr is not None:
            result.err = syserr.text

        # The command is prepended to system-out, so we need to separate those
        # into two separate elements
        out_tag = test.find('system-out')
        if out_tag is not None:
            if is_piglit:
                out = out_tag.text.split('\n')
                result.command = out[0]
                result.out = '\n'.join(out[1:])
            else:
                result.out = out_tag.text

        # Try to get the values in stderr for time and pid
        for line in result.err.split('\n'):
            if line.startswith('time start:'):
                result.time.start = float(line[len('time start: '):])
                continue
            elif line.startswith('time end:'):
                result.time.end = float(line[len('time end: '):])
                continue
            elif line.startswith('pid:'):
                result.pid = json.loads(line[len('pid: '):])
                continue


        run_result.tests[name] = result

    run_result.calculate_group_totals()

    return run_result


def load(results_dir, compression):  # pylint: disable=unused-argument
    """Searches for a results file and returns a TestrunResult.

    wraps _load and searches for the result file.

    """
    if not os.path.isdir(results_dir):
        return _load(results_dir)
    elif os.path.exists(os.path.join(results_dir, 'tests')):
        raise NotImplementedError('resume support of junit not implemented')
    elif os.path.exists(os.path.join(results_dir, 'results.xml')):
        return _load(os.path.join(results_dir, 'results.xml'))
    else:
        raise exceptions.PiglitFatalError("No results found")


REGISTRY = Registry(
    extensions=['.xml'],
    backend=JUnitBackend,
    load=load,
    meta=lambda x: x,  # The venerable no-op function
)
