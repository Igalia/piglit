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

""" Module of tests for the run commandline parser """

import os
import nose.tools as nt
import framework.tests.utils as utils
import framework.programs.run as run


class TestPlatform(utils.TestWithEnvClean):
    """ Test piglitrun -p/--platform options """
    def test_default(self):
        """ Run parser platform: When no option is passed the default option is
        used """
        args = run._run_parser(['quick.py', 'foo'])
        nt.assert_equal(args.platform, 'mixed_glx_egl')

    def test_options(self):
        """ Run parser platform: when an option is present it replaces default
        """
        args = run._run_parser(['-p', 'x11_egl', 'quick.py', 'foo'])
        nt.assert_equal(args.platform, 'x11_egl')

    def test_env_no_options(self):
        """ Run parser platform: When no option is passed env overrides default
        """
        self.add_teardown('PIGLIT_PLATFORM')
        os.environ['PIGLIT_PLATFORM'] = 'glx'

        args = run._run_parser(['quick.py', 'foo'])
        nt.assert_equal(args.platform, 'glx')

    def test_env_options(self):
        """ Run parser platform: when an option is passed it overwrites env """
        self.add_teardown('PIGLIT_PLATFORM')
        os.environ['PIGLIT_PLATFORM'] = 'glx'

        args = run._run_parser(['-p', 'x11_egl', 'quick.py', 'foo'])
        nt.assert_equal(args.platform, 'x11_egl')
