###########################################################################
#
# Copyright 2010-2011 VMware, Inc.
# All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sub license, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice (including the
# next paragraph) shall be included in all copies or substantial portions
# of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
###########################################################################

"""Testing framework that assists invoking external programs and outputing
results in ANT's junit XML format, used by Jenkins-CI."""


import locale
import optparse
import os.path
import shutil
import string
import sys
import time


__all__ = [
    'Error',
    'Failure',
    'Main',
    'Report',
    'Test',
    'TestSuite',
]


class Failure(Exception):
    pass


class Error(Exception):
    pass


def escape(s):
    '''Escape and encode a XML string.'''
    if not isinstance(s, unicode):
        #s = s.decode(locale.getpreferredencoding(), 'replace')
        s = s.decode('ascii', 'ignore')
    s = s.replace('&', '&amp;')
    s = s.replace('<', '&lt;')
    s = s.replace('>', '&gt;')
    s = s.replace('"', '&quot;')
    s = s.replace("'", '&apos;')
    s = s.encode('UTF-8')
    return s


# same as string.printable, but without '\v\f'
_printable = string.digits + string.letters + string.punctuation + ' \t\n\r'
_printable = ''.join([chr(_c) in _printable and chr(_c) or '?' for _c in range(256)])
del _c


class Report:
    """Write test results in ANT's junit XML format.

    See also:
    - https://github.com/jenkinsci/jenkins/tree/master/test/src/test/resources/hudson/tasks/junit
    - http://www.junit.org/node/399
    - http://wiki.apache.org/ant/Proposals/EnhancedTestReports
    """

    def __init__(self, filename, time = True):
        self.path = os.path.dirname(os.path.abspath(filename))
        if not os.path.exists(self.path):
            os.makedirs(self.path)

        self.stream = open(filename, 'wt')
        self.testsuites = []
        self.inside_testsuite = False
        self.inside_testcase = False
        self.time = time

    def start(self):
        self.stream.write('<?xml version="1.0" encoding="UTF-8" ?>\n')
        self.stream.write('<testsuites>\n')

    def stop(self):
        if self.inside_testcase:
            self.stream.write('</testcase>\n')
            self.inside_testcase = False
        if self.inside_testsuite:
            self.stream.write('</testsuite>\n')
            self.inside_testsuite = False
        self.stream.write('</testsuites>\n')
        self.stream.flush()
        self.stream.close()

    def escapeName(self, name):
        '''Dots are special for junit, so escape them with underscores.'''
        name = name.replace('.', '_')
        return name

    def startSuite(self, name):
        self.testsuites.append(self.escapeName(name))

    def stopSuite(self):
        if self.inside_testsuite:
            self.stream.write('</testsuite>\n')
            self.inside_testsuite = False
        self.testsuites.pop(-1)

    def startCase(self, name):
        assert not self.inside_testcase
        self.inside_testcase = True

        if not self.inside_testsuite:
            self.stream.write('<testsuite name="%s">\n' % escape('.'.join(self.testsuites[:1])))
            self.inside_testsuite = True

        self.case_name = name
        self.buffer = []
        self.stdout = []
        self.stderr = []
        self.start_time = time.time()

    def stopCase(self, duration = None):
        assert self.inside_testcase
        self.inside_testcase = False

        if len(self.testsuites) == 1:
            classname = self.testsuites[0] + '.' + self.testsuites[0]
        else:
            classname = '.'.join(self.testsuites)
        name = self.case_name

        self.stream.write('<testcase classname="%s" name="%s"' % (escape(classname), escape(name)))
        if duration is None:
            if self.time:
                stop_time = time.time()
                duration = stop_time - self.start_time
        if duration is not None:
            self.stream.write(' time="%f"' % duration)

        if not self.buffer and not self.stdout and not self.stderr:
            self.stream.write('/>\n')
        else:
            self.stream.write('>')

            for entry in self.buffer:
                self.stream.write(entry)
            if self.stdout:
                self.stream.write('<system-out>')
                for text in self.stdout:
                    self.stream.write(escape(text))
                self.stream.write('</system-out>')
            if self.stderr:
                self.stream.write('<system-err>')
                for text in self.stderr:
                    self.stream.write(escape(text))
                self.stream.write('</system-err>')

            self.stream.write('</testcase>\n')

        self.stream.flush()

    def addStdout(self, text):
        if isinstance(text, str):
            text = text.translate(_printable)
        self.stdout.append(text)

    def addStderr(self, text):
        if isinstance(text, str):
            text = text.translate(_printable)
        self.stderr.append(text)

    def addSkipped(self):
        self.buffer.append('<skipped/>\n')

    def addError(self, message, stacktrace=""):
        self.buffer.append('<error message="%s"' % escape(message))
        if not stacktrace:
            self.buffer.append('/>')
        else:
            self.buffer.append('>')
            self.buffer.append(escape(stacktrace))
            self.buffer.append('</error>')

    def addFailure(self, message, stacktrace=""):
        self.buffer.append('<failure message="%s"' % escape(message))
        if not stacktrace:
            self.buffer.append('/>')
        else:
            self.buffer.append('>')
            self.buffer.append(escape(stacktrace))
            self.buffer.append('</failure>')

    def addMeasurement(self, name, value):
        '''Embedded a measurement in the standard output.

        https://wiki.jenkins-ci.org/display/JENKINS/Measurement+Plots+Plugin
        '''

        if value is not None:
            message = '<measurement><name>%s</name><value>%f</value></measurement>\n' % (name, value)
            self.addStdout(message)

    def addAttachment(self, path):
        '''Attach a file.

        https://wiki.jenkins-ci.org/display/JENKINS/JUnit+Attachments+Plugin
        '''

        attach_dir = os.path.join(self.path, '.'.join(self.testsuites + [self.case_name]))
        if not os.path.exists(attach_dir):
            os.makedirs(attach_dir)
        shutil.copy2(path, attach_dir)

    def addWorkspaceURL(self, path):
        import urlparse
        try:
            workspace_path = os.environ['WORKSPACE']
            job_url = os.environ['JOB_URL']
        except KeyError:
            self.addStdout(path + '\n')
        else:
            rel_path = os.path.relpath(path, workspace_path)
            workspace_url = urlparse.urljoin(job_url, 'ws/')
            url = urlparse.urljoin(workspace_url, rel_path)
            if os.path.isdir(path):
                url += '/'
            self.addStdout(url + '\n')


class BaseTest:

    def _visit(self, report):
        raise NotImplementedError

    def fail(self, *args):
        raise Failure(*args)

    def error(self, *args):
        raise Error(*args)



class TestSuite(BaseTest):

    def __init__(self, name, tests=()):
        self.name = name
        self.tests = []
        self.addTests(tests)

    def addTest(self, test):
        self.tests.append(test)

    def addTests(self, tests):
        for test in tests:
            self.addTest(test)

    def run(self, filename = None, report = None):
        if report is None:
            if filename is None:
                filename = self.name + '.xml'
        report = Report(filename)
        report.start()
        try:
            self._visit(report)
        finally:
            report.stop()

    def _visit(self, report):
        report.startSuite(self.name)
        try:
            self.test(report)
        finally:
            report.stopSuite()

    def test(self, report):
        for test in self.tests:
            test._visit(report)


class Test(BaseTest):

    def __init__(self, name):
        self.name = name

    def _visit(self, report):
        report.startCase(self.name)
        try:
            try:
                return self.test(report)
            except Failure as ex:
                report.addFailure(*ex.args)
            except Error as ex:
                report.addError(*ex.args)
            except KeyboardInterrupt:
                raise
            except:
                report.addError(str(sys.exc_value))
        finally:
            report.stopCase()

    def test(self, report):
        raise NotImplementedError


class Main:

    default_timeout = 5*60

    def __init__(self, name):
        self.name = name

    def optparser(self):
        optparser = optparse.OptionParser(usage="\n\t%prog [options] ...")
        optparser.add_option(
            '-n', '--dry-run',
            action="store_true",
            dest="dry_run", default=False,
            help="perform a trial run without executing")
        optparser.add_option(
            '-t', '--timeout', metavar='SECONDS',
            type="float", dest="timeout", default = self.default_timeout,
            help="timeout in seconds [default: %default]")
        #optparser.add_option(
        #    '-f', '--filter',
        #    action='append',
        #    type="choice", metevar='GLOB',
        #    dest="filters", default=[],
        #    help="filter")
        return optparser

    def create_suite(self):
        raise NotImplementedError

    def run_suite(self, suite):
        filename = self.name + '.xml'
        report = Report(filename)
        suite.run()

    def main(self):
        optparser = self.optparser()
        (self.options, self.args) = optparser.parse_args(sys.argv[1:])

        suite = self.create_suite()
        self.run_suite(suite)

