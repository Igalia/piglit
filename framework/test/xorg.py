# encoding=utf-8
# Copyright (c) 2013-2014 Intel Corporation
# Copyright © 2013-2014, 2019 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# This permission notice shall be included in all copies or substantial
# portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

"""Test integration with Xorg suites.

This includes XTS and the rendercheck suite.
"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os
import re
import shutil
import subprocess
import tempfile

from .base import Test


class XTSTest(Test):  # pylint: disable=too-few-public-methods
    """ X Test Suite class

    Runs a single test or subtest from XTS.

    Arguments:
    name -- the name of the test
    testname -- the name of the test file
    testnum -- the number of the test file

    """
    RESULTS_PATH = None

    def __init__(self, command, testname, testdir, **kwargs):
        super(XTSTest, self).__init__(command, **kwargs)
        self.testname = testname
        self.testdir = testdir
        self.outdir = tempfile.mkdtemp()
        self.result_file = os.path.join(self.outdir, testdir, testname)
        self.env.update({"TET_RESFILE": self.result_file})

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
                command = ['xtsttopng', os.path.join(self.result_file,
                                                     self.testdir,
                                                     match.group(1))]
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
                ref_path = os.path.join(
                    self.RESULTS_PATH, 'images', '{1}-{2}-ref.png'.format(
                        self.testname, match.group(1)))
                render_path = os.path.join(
                    self.RESULTS_PATH, 'images', '{1}-{2}-render.png'.format(
                        self.testname, match.group(1)))

                split = out.splitlines()
                os.rename(os.path.join(self.cwd, split[0]), render_path)
                os.rename(os.path.join(self.cwd, split[1]), ref_path)

                images.append({'image_desc': desc,
                               'image_ref': ref_path,
                               'image_render': render_path})

        return images

    def interpret_result(self):
        super(XTSTest, self).interpret_result()

        try:
            with open(self.result_file, 'r') as rfile:
                log = rfile.read()
                self.result.out = log
                os.remove(self.result_file)
        except IOError:
            self.result.err = "No results file found"
            log = ""
        finally:
            shutil.rmtree(self.outdir)

        if self.result.returncode == 0:
            if re.search('FAIL', self.result.out) is not None:
                self.result.result = 'fail'
            elif re.search('PASS', self.result.out) is not None:
                self.result.result = 'pass'
            else:
                self.result.result = 'fail'
        elif self.result.returncode == 77:
            self.result.result = 'skip'
        elif self.result.returncode == 1:
            if re.search('Could not open all VSW5 fonts', log):
                self.result.result = 'warn'
            else:
                self.result.result = 'fail'

        self.result.images = self._process_log_for_images(log)


class RendercheckTest(Test):

    def __init__(self, command, testname):
        super(RendercheckTest, self).__init__(command)
        self.testname = testname

    def interpret_result(self):
        super(RendercheckTest, self).interpret_result()

        if self.result.returncode == 0:
            self.result.result = 'pass'
        elif self.result.returncode == 77:
            self.result.result = 'skip'
