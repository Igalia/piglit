# Copyright (c) 2013-2014 Intel Corporation
#
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

""" Test integreation for the X Test Suite """

import os
import re
import sys
import subprocess
import itertools
import framework.core
from framework.profile import TestProfile, Test

__all__ = ['profile']

X_TEST_SUITE = framework.core.PIGLIT_CONFIG.get('xts', 'path')


class XTSProfile(TestProfile):
    """ A subclass of TestProfile that provides a setup hook for XTS """
    def _pre_run_hook(self):
        """ This hook sets the XTSTest.results_path variable

        Setting this variable allows images created by XTS to moved into the
        results directory

        """
        XTSTest.RESULTS_PATH = self.results_dir

        try:
            os.mkdir(os.path.join(self.results_dir, 'images'))
        except OSError as e:
            # If the exception is not 'directory already exists', raise the
            # exception
            if e.errno != 17:
                raise


class XTSTest(Test):
    """ X Test Suite class

    Runs a single test or subtest from XTS.

    Arguments:
    name -- the name of the test
    testname -- the name of the test file
    testnum -- the number of the test file

    """
    RESULTS_PATH = None

    def __init__(self, name, testname, testnum):
        super(XTSTest, self).__init__(
            ['./' + os.path.basename(name), '-i', str(testnum)])
        self.testname = '{0}-{1}'.format(testname, testnum)
        self.cwd = os.path.dirname(os.path.realpath(name))
        self.test_results_file = os.path.join(self.cwd, self.testname)
        self.env.update(
            {"TET_RESFILE": self.test_results_file,
             "XT_RESET_DELAY": '0',
             "XT_FONTPATH_GOOD": '/usr/share/fonts/X11/misc',
             "XT_FONTPATH": os.path.join(X_TEST_SUITE, 'xts5', 'fonts'),
             #XXX: Are the next 3 necissary
             "XT_LOCAL": 'Yes',
             "XT_TCP": 'No',
             "XT_DISPLAYHOST": ''})

    def _process_log_for_images(self, log):
        """ Parse the image logfile """
        images = []
        search = re.compile('See file (Err[0-9]+.err)')

        for line in log.splitlines():
            match = search.search(line)
            if match is not None:
                # Can we parse any other useful information out to give a
                # better description of each image?
                desc = match.group(1)

                # The error logs are text, with a header with width, height,
                # and depth, then run-length-encoded pixel values (in
                # hexadecimal).  Use xtsttopng to convert the error log to a
                # pair of PNGs so we can put them in the summary.
                command = ['xtsttopng', os.path.join(self.cwd, match.group(1))]
                try:
                    out = subprocess.check_output(command, cwd=self.cwd)
                except OSError:
                    images.append({'image_desc': 'image processing failed'})
                    continue

                # Each Err*.err log contains a rendered image, and a reference
                # image that it was compared to.  We relocate the to our tree
                # with more useful names.  (Otherwise, since tests generate
                # error logs with numbers sequentially starting from 0, each
                # subtest with an error would overwrite the previous test's
                # images).
                ref_path = '{0}/images/{1}-{2}-ref.png'.format(
                    XTSTest.RESULTS_PATH, self.testname, match.group(1))
                render_path = '{0}/images/{1}-{2}-render.png'.format(
                    XTSTest.RESULTS_PATH, self.testname, match.group(1))

                split = out.splitlines()
                os.rename(os.path.join(self.cwd, split[0]), render_path)
                os.rename(os.path.join(self.cwd, split[1]), ref_path)

                images.append({'image_desc': desc,
                               'image_ref': ref_path,
                               'image_render': render_path})

        return images

    def interpret_result(self):
        try:
            with open(self.test_results_file, 'r') as rfile:
                log = rfile.read()
                self.result['out'] = log
                os.remove(self.test_results_file)
        except IOError:
            self.result['err'] = "No results file found"

        if self.result['returncode'] == 0:
            if re.search('FAIL', self.result['out']) is not None:
                self.result['result'] = 'fail'
            elif re.search('PASS', self.result['out']) is not None:
                self.result['result'] = 'pass'
            else:
                self.result['result'] = 'fail'
        elif self.result['returncode'] == 77:
            self.result['result'] = 'skip'
        elif self.result['returncode'] == 1:
            if re.search('Could not open all VSW5 fonts', log):
                self.result['result'] = 'warn'
            else:
                self.result['result'] = 'fail'
        else:
            self.result['result'] = 'fail'

        self.result['images'] = self._process_log_for_images(log)


def populate_profile():
    """ Populate the profile attribute """
    # Add all tests to the profile
    profile = XTSProfile()
    fpath = os.path.join(X_TEST_SUITE, 'xts5')
    for dirpath, _, filenames in os.walk(fpath):
        for fname in filenames:
            # only look at the .m test files
            testname, ext = os.path.splitext(fname)
            if ext != '.m':
                continue

            # incrementing number generator
            counts = (x for x in itertools.count(1, 1))

            # Walk the file looking for >>ASSERTION, each of these corresponds
            # to a generated subtest, there can be multiple subtests per .m
            # file
            with open(os.path.join(dirpath, fname), 'r') as rfile:
                for line in rfile:
                    if line.startswith('>>ASSERTION'):
                        num = next(counts)
                        group = '{0}/{1}/{2}'.format(
                            os.path.relpath(dirpath, X_TEST_SUITE),
                            testname, num)

                        profile.tests[group] = XTSTest(
                            os.path.join(dirpath, testname),
                            testname,
                            num)
    return profile


# If the symlink for the XTS has not been created exit
if not os.path.exists(X_TEST_SUITE):
    print "xtest symlink not found!"
    sys.exit(0)

profile = populate_profile()
