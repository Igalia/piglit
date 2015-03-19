# Copyright (c) 2014, 2015 Intel Corporation

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

from __future__ import print_function, absolute_import
import os.path
import shutil

try:
    from lxml import etree
except ImportError:
    import xml.etree.cElementTree as etree

from framework import grouptools, results, status
from framework.core import PIGLIT_CONFIG
from .abstract import FileBackend
from .register import Registry

__all__ = [
    'REGISTRY',
    'JUnitBackend',
]


class JUnitBackend(FileBackend):
    """ Backend that produces ANT JUnit XML

    Based on the following schema:
    https://svn.jenkins-ci.org/trunk/hudson/dtkit/dtkit-format/dtkit-junit-model/src/main/resources/com/thalesgroup/dtkit/junit/model/xsd/junit-7.xsd

    """

    def __init__(self, dest, junit_suffix='', **options):
        super(JUnitBackend, self).__init__(dest, **options)
        self._test_suffix = junit_suffix

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
                except etree.ParseError:
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

        def calculate_result():
            """Set the result."""
            expected_result = "pass"

            # replace special characters and make case insensitive
            lname = (classname + "." + testname).lower()
            lname = lname.replace("=", ".")
            lname = lname.replace(":", ".")

            if lname in self._expected_failures:
                expected_result = "failure"
                # a test can either fail or crash, but not both
                assert lname not in self._expected_crashes

            if lname in self._expected_crashes:
                expected_result = "error"

            res = None
            # Add relevant result value, if the result is pass then it doesn't
            # need one of these statuses
            if data['result'] == 'skip':
                res = etree.SubElement(element, 'skipped')

            elif data['result'] in ['warn', 'fail', 'dmesg-warn',
                                    'dmesg-fail']:
                if expected_result == "failure":
                    err.text += "\n\nWARN: passing test as an expected failure"
                    res = etree.SubElement(element, 'skipped',
                                           message='expected failure')
                else:
                    res = etree.SubElement(element, 'failure')

            elif data['result'] == 'crash':
                if expected_result == "error":
                    err.text += "\n\nWARN: passing test as an expected crash"
                    res = etree.SubElement(element, 'skipped',
                                           message='expected crash')
                else:
                    res = etree.SubElement(element, 'error')

            elif expected_result != "pass":
                err.text += "\n\nERROR: This test passed when it "\
                            "expected {0}".format(expected_result)
                res = etree.SubElement(element, 'failure')

            # Add the piglit type to the failure result
            if res is not None:
                res.attrib['type'] = str(data['result'])

        # Split the name of the test and the group (what junit refers to as
        # classname), and replace piglits '/' separated groups with '.', after
        # replacing any '.' with '_' (so we don't get false groups).
        classname, testname = grouptools.splitname(name)
        classname = classname.replace('.', '_')
        classname = classname.replace(grouptools.SEPARATOR, '.')

        # Add the test to the piglit group rather than directly to the root
        # group, this allows piglit junit to be used in conjunction with other
        # piglit
        # TODO: It would be nice if other suites integrating with piglit could
        # set different root names.
        classname = 'piglit.' + classname

        # Jenkins will display special pages when the test has certain names.
        # https://jenkins-ci.org/issue/18062
        # https://jenkins-ci.org/issue/19810
        # The testname variable is used in the calculate_result
        # closure, and must not have the suffix appended.
        full_test_name = testname + self._test_suffix
        if full_test_name in ('api', 'search'):
            testname += '_'
            full_test_name = testname + self._test_suffix

        # Create the root element
        element = etree.Element('testcase', name=full_test_name,
                                classname=classname,
                                time=str(data['time']),
                                status=str(data['result']))

        # Add stdout
        out = etree.SubElement(element, 'system-out')
        out.text = data['out']

        # Prepend command line to stdout
        out.text = data['command'] + '\n' + out.text

        # Add stderr
        err = etree.SubElement(element, 'system-err')
        err.text = data['err']

        calculate_result()

        t = os.path.join(self._dest, 'tests',
                         '{}.xml'.format(self._counter.next()))
        with open(t, 'w') as f:
            f.write(etree.tostring(element))
            self._fsync(f)


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

    tree = etree.parse(results_file).getroot().find('.//testsuite[@name="piglit"]')
    for test in tree.iterfind('testcase'):
        result = results.TestResult()
        # Take the class name minus the 'piglit.' element, replace junit's '.'
        # separator with piglit's separator, and join the group and test names
        name = test.attrib['classname'].split('.', 1)[1]
        name = name.replace('.', grouptools.SEPARATOR)
        name = grouptools.join(name, test.attrib['name'])

        # Remove the trailing _ if they were added (such as to api and search)
        if name.endswith('_'):
            name = name[:-1]

        result['result'] = status.status_lookup(test.attrib['status'])
        result['time'] = float(test.attrib['time'])
        result['err'] = test.find('system-err').text

        # The command is prepended to system-out, so we need to separate those
        # into two separate elements
        out = test.find('system-out').text.split('\n')
        result['command'] = out[0]
        result['out'] = '\n'.join(out[1:])

        run_result.tests[name] = result
    
    return run_result


def load(results_dir):
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
        raise Exception("No results found")


REGISTRY = Registry(
    extensions=['.xml'],
    backend=JUnitBackend,
    load=load,
    meta=lambda x: x,  # The venerable no-op function
)
