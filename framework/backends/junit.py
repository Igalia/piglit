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

""" Module implementing a JUnitBackend for piglit """

import os
import re
import posixpath
import itertools
import shutil
try:
    from lxml import etree
except ImportError:
    import xml.etree.cElementTree as etree
from .abstract import Backend, FSyncMixin
from framework.core import PIGLIT_CONFIG

__all__ = [
    'JUnitBackend',
]


class JUnitBackend(FSyncMixin, Backend):
    """ Backend that produces ANT JUnit XML

    Based on the following schema:
    https://svn.jenkins-ci.org/trunk/hudson/dtkit/dtkit-format/dtkit-junit-model/src/main/resources/com/thalesgroup/dtkit/junit/model/xsd/junit-7.xsd

    """
    _REPLACE = re.compile(r'[/\\]')

    def __init__(self, dest, junit_test_suffix='', start_count=0, **options):
        FSyncMixin.__init__(self, **options)
        self._dest = dest
        self._test_suffix = junit_test_suffix
        self._counter = itertools.count(start_count)

        # make dictionaries of all test names expected to crash/fail
        # for quick lookup when writing results.  Use lower-case to
        # provide case insensitive matches.
        self._expected_failures = {}
        if PIGLIT_CONFIG.has_section("expected-failures"):
            for (fail, _) in PIGLIT_CONFIG.items("expected-failures"):
                self._expected_failures[fail.lower()] = True
        self._expected_crashes = {}
        if PIGLIT_CONFIG.has_section("expected-crashes"):
            for (fail, _) in PIGLIT_CONFIG.items("expected-crashes"):
                self._expected_crashes[fail.lower()] = True

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
                except etree.XMLSyntaxError:
                    continue

        # set the test count by counting the number of tests.
        # This must be bytes or unicode
        piglit.attrib['tests'] = str(len(piglit))

        with open(os.path.join(self._dest, 'results.xml'), 'w') as f:
            f.write("<?xml version='1.0' encoding='utf-8'?>\n")
            # lxml has a pretty print we want to use
            if etree.__name__ == 'lxml.etree':
                f.write(etree.tostring(root, pretty_print=True))
            else:
                f.write(etree.tostring(root))

        shutil.rmtree(os.path.join(self._dest, 'tests'))

    def write_test(self, name, data):
        # Split the name of the test and the group (what junit refers to as
        # classname), and replace piglits '/' separated groups with '.', after
        # replacing any '.' with '_' (so we don't get false groups). Also
        # remove any '\\' that has been inserted on windows accidentally
        classname, testname = posixpath.split(name)
        classname = classname.replace('.', '_')
        classname = JUnitBackend._REPLACE.sub('.', classname)

        # Add the test to the piglit group rather than directly to the root
        # group, this allows piglit junit to be used in conjunction with other
        # piglit
        # TODO: It would be nice if other suites integrating with piglit could
        # set different root names.
        classname = 'piglit.' + classname

        expected_result = "pass"

        # replace special characters and make case insensitive
        lname = (classname + "." + testname).lower()
        lname = lname.replace("=", ".")
        lname = lname.replace(":", ".")

        if lname in self._expected_failures:
            expected_result = "failure"
            # a test can either fail or crash, but not both
            assert( lname not in self._expected_crashes )

        if lname in self._expected_crashes:
            expected_result = "error"

        # Create the root element
        element = etree.Element('testcase', name=testname + self._test_suffix,
                                classname=classname,
                                time=str(data['time']),
                                status=str(data['result']))

        # Add stdout
        out = etree.SubElement(element, 'system-out')
        out.text = data['out']

        # Add stderr
        err = etree.SubElement(element, 'system-err')
        err.text = data['err']

        # Add relevant result value, if the result is pass then it doesn't need
        # one of these statuses
        if data['result'] == 'skip':
            etree.SubElement(element, 'skipped')

        elif data['result'] in ['warn', 'fail', 'dmesg-warn', 'dmesg-fail']:
            if expected_result == "failure":
                err.text += "\n\nWARN: passing test as an expected failure"
            else:
                etree.SubElement(element, 'failure')

        elif data['result'] == 'crash':
            if expected_result == "error":
                err.text += "\n\nWARN: passing test as an expected crash"
            else:
                etree.SubElement(element, 'error')

        elif expected_result != "pass":
            err.text += "\n\nERROR: This test passed when it "\
                        "expected {0}".format(expected_result)
            etree.SubElement(element, 'failure')

        t = os.path.join(self._dest, 'tests',
                         '{}.xml'.format(self._counter.next()))
        with open(t, 'w') as f:
            f.write(etree.tostring(element))
            self._fsync(f)
