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

from __future__ import print_function, absolute_import
import sys
import os
import shutil
import ConfigParser

import nose.tools as nt

import framework.tests.utils as utils
import framework.programs.run as run
import framework.core as core


class TestWithEnvClean(object):
    """ Class that does cleanup with saved state

    This could be done with test fixtures, but this should be cleaner in the
    specific case of cleaning up environment variables

    Nose will run a method (bound or unbound) at the start of the test called
    setup() and one at the end called teardown(), we have added a teardown
    method.

    Using this gives us the assurance that we're not relying on settings from
    other tests, making ours pass or fail, and that os.enviorn is the same
    going in as it is going out.

    This is modeled after Go's defer keyword.

    """
    def __init__(self):
        self._saved = set()
        self._teardown_calls = []

    def add_teardown(self, var, restore=True):
        """ Add os.environ values to remove in teardown """
        if var in os.environ:
            self._saved.add((var, os.environ.get(var), restore))
            del os.environ[var]

    def defer(self, func, *args):
        """ Add a function (with arguments) to be run durring cleanup """
        self._teardown_calls.append((func, args))

    def teardown(self):
        """ Teardown the test

        Restore any variables that were unset at the begining of the test, and
        run any differed methods.

        """
        for key, value, restore in self._saved:
            # If value is None the value was unset previously, put it back
            if value is None:
                del os.environ[key]
            elif restore:
                os.environ[key] = value

        # Teardown calls is a FIFO stack, the defered calls must be run in
        # reversed order to make any sense
        for call, args in reversed(self._teardown_calls):
            call(*args)


class _Helpers(TestWithEnvClean):
    """ Some helpers to be shared between tests """
    def _unset_config(self):
        """ Ensure that no config files are being accidently loaded """
        self.add_teardown('HOME')
        self.add_teardown('XDG_CONFIG_HOME')

    def _move_piglit_conf(self):
        """ Move piglit.conf from local and from piglit root

        They are moved and put back so they aren't accidentally loaded

        """
        if os.path.exists('piglit.conf'):
            shutil.move('piglit.conf', 'piglit.conf.restore')
            self.defer(shutil.move, 'piglit.conf.restore', 'piglit.conf')

        root = os.path.join(run.__file__, '..', '..', 'piglit.conf')
        if os.path.exists(root):
            shutil.move(root, root + '.restore')
            self.defer(shutil.move, root + '.restore', root)

    def setup(self):
        # Set core.PIGLIT_CONFIG back to pristine between tests
        core.PIGLIT_CONFIG = ConfigParser.SafeConfigParser()


class TestBackend(_Helpers):
    """ Test piglit run -b/--backend option """
    _CONF = '[core]\nbackend=junit'

    def test_default(self):
        """ run parser: backend final fallback path """
        self._unset_config()
        self._move_piglit_conf()

        args = run._run_parser(['quick.py', 'foo'])
        nt.assert_equal(args.backend, 'json')

    def test_option_default(self):
        """ Run parser: backend replaces default """
        args = run._run_parser(['-b', 'json', 'quick.py', 'foo'])
        nt.assert_equal(args.backend, 'json')

    def test_option_conf(self):
        """ Run parser: backend option replaces conf """
        with utils.tempdir() as tdir:
            os.environ['XDG_CONFIG_HOME'] = tdir
            with open(os.path.join(tdir, 'piglit.conf'), 'w') as f:
                f.write(self._CONF)

            args = run._run_parser(['-b', 'json', 'quick.py', 'foo'])
            nt.assert_equal(args.backend, 'json')

    def test_conf_default(self):
        """ Run parser platform: conf is used as a default when applicable """
        self._unset_config()
        self._move_piglit_conf()

        with utils.tempdir() as tdir:
            os.environ['XDG_CONFIG_HOME'] = tdir
            with open(os.path.join(tdir, 'piglit.conf'), 'w') as f:
                f.write(self._CONF)

            args = run._run_parser(['quick.py', 'foo'])
            nt.assert_equal(args.backend, 'junit')

    def test_bad_value_in_conf(self):
        """ run parser: an error is raised when the platform in conf is bad """
        self._unset_config()
        self._move_piglit_conf()

        # This has sideffects, it shouldn't effect anything in this module, but
        # it may cause later problems. But without this we get ugly error spew
        # from this test.
        sys.stderr = open(os.devnull, 'w')

        with utils.tempdir() as tdir:
            with open(os.path.join(tdir, 'piglit.conf'), 'w') as f:
                f.write('[core]\nbackend=foobar')

            with nt.assert_raises(SystemExit):
                run._run_parser(['-f', os.path.join(tdir, 'piglit.conf'),
                                 'quick.py', 'foo'])


class TestPlatform(_Helpers):
    """ Test piglitrun -p/--platform options """
    _CONF = '[core]\nplatform=gbm'

    def __set_env(self):
        """ Set PIGLIT_PLATFORM """
        self.add_teardown('PIGLIT_PLATFORM')
        os.environ['PIGLIT_PLATFORM'] = 'glx'

    def test_default(self):
        """ run parser: platform final fallback path """
        self._unset_config()
        self._move_piglit_conf()

        args = run._run_parser(['quick.py', 'foo'])
        nt.assert_equal(args.platform, 'mixed_glx_egl')

    def test_option_default(self):
        """ Run parser: platform replaces default """
        args = run._run_parser(['-p', 'x11_egl', 'quick.py', 'foo'])
        nt.assert_equal(args.platform, 'x11_egl')

    def test_option_env(self):
        """ Run parser: platform option replaces env """
        self.__set_env()

        args = run._run_parser(['-p', 'x11_egl', 'quick.py', 'foo'])
        nt.assert_equal(args.platform, 'x11_egl')

    def test_option_conf(self):
        """ Run parser: platform option replaces conf """
        with utils.tempdir() as tdir:
            os.environ['XDG_CONFIG_HOME'] = tdir
            with open(os.path.join(tdir, 'piglit.conf'), 'w') as f:
                f.write(self._CONF)

            args = run._run_parser(['-p', 'x11_egl', 'quick.py', 'foo'])
            nt.assert_equal(args.platform, 'x11_egl')

    def test_env_no_options(self):
        """ Run parser: When no platform is passed env overrides default
        """
        self.__set_env()

        args = run._run_parser(['quick.py', 'foo'])
        nt.assert_equal(args.platform, 'glx')

    def test_conf_default(self):
        """ Run parser platform: conf is used as a default when applicable """
        self._unset_config()
        self._move_piglit_conf()

        with utils.tempdir() as tdir:
            os.environ['XDG_CONFIG_HOME'] = tdir
            with open(os.path.join(tdir, 'piglit.conf'), 'w') as f:
                f.write(self._CONF)

            args = run._run_parser(['quick.py', 'foo'])
            nt.assert_equal(args.platform, 'gbm')

    def test_env_conf(self):
        """ Run parser: env overwrides a conf value """
        self._unset_config()
        self._move_piglit_conf()
        self.__set_env()

        with utils.tempdir() as tdir:
            os.environ['XDG_CONFIG_HOME'] = tdir
            with open(os.path.join(tdir, 'piglit.conf'), 'w') as f:
                f.write(self._CONF)

            args = run._run_parser(['quick.py', 'foo'])
            nt.assert_equal(args.platform, 'glx')

    def test_bad_value_in_conf(self):
        """ run parser: an error is raised when the platform in conf is bad """
        self._unset_config()
        self._move_piglit_conf()

        # This has sideffects, it shouldn't effect anything in this module, but
        # it may cause later problems. But without this we get ugly error spew
        # from this test.
        sys.stderr = open(os.devnull, 'w')

        with utils.tempdir() as tdir:
            with open(os.path.join(tdir, 'piglit.conf'), 'w') as f:
                f.write('[core]\nplatform=foobar')

            with nt.assert_raises(SystemExit):
                run._run_parser(['-f', os.path.join(tdir, 'piglit.conf'),
                                 'quick.py', 'foo'])
