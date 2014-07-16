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


""" Tests for the programs package

Currently there aren't very many tests for the modules in this package, so just
having a single test module seems appropriate

"""

import os
import shutil
import ConfigParser
import framework.core as core
import framework.programs.run as run
import framework.tests.utils as utils
import nose.tools as nt

CONF_FILE = """
[nose-test]
; a section for testing behavior
dir = foo
"""


# Helpers
class _TestWithEnvClean(object):
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


# Tests
class TestGetConfigEnv(_TestWithEnvClean):
    def test(self):
        """ get_config() finds $XDG_CONFIG_HOME/piglit.conf """
        self.defer(lambda: core.PIGLIT_CONFIG == ConfigParser.SafeConfigParser)
        self.add_teardown('XDG_CONFIG_HOME')
        if os.path.exists('piglit.conf'):
            shutil.move('piglit.conf', 'piglit.conf.restore')
            self.defer(shutil.move, 'piglit.conf.restore', 'piglit.conf')

        with utils.tempdir() as tdir:
            os.environ['XDG_CONFIG_HOME'] = tdir
            with open(os.path.join(tdir, 'piglit.conf'), 'w') as f:
                f.write(CONF_FILE)
            core.get_config()

        nt.ok_(core.PIGLIT_CONFIG.has_section('nose-test'),
               msg='$XDG_CONFIG_HOME not found')


class TestGetConfigHomeFallback(_TestWithEnvClean):
    def test(self):
        """ get_config() finds $HOME/.config/piglit.conf """
        self.defer(lambda: core.PIGLIT_CONFIG == ConfigParser.SafeConfigParser)
        self.add_teardown('HOME')
        self.add_teardown('XDG_CONFIG_HOME')
        if os.path.exists('piglit.conf'):
            shutil.move('piglit.conf', 'piglit.conf.restore')
            self.defer(shutil.move, 'piglit.conf.restore', 'piglit.conf')

        with utils.tempdir() as tdir:
            os.environ['HOME'] = tdir
            os.mkdir(os.path.join(tdir, '.config'))
            with open(os.path.join(tdir, '.config/piglit.conf'), 'w') as f:
                f.write(CONF_FILE)

        nt.ok_(core.PIGLIT_CONFIG.has_section('nose-test'),
               msg='$HOME/.config not found')


class TestGetConfigLocal(_TestWithEnvClean):
    # These need to be empty to force '.' to be used
    def test(self):
        """ get_config() finds ./piglit.conf """
        self.defer(lambda: core.PIGLIT_CONFIG == ConfigParser.SafeConfigParser)
        self.add_teardown('HOME')
        self.add_teardown('XDG_CONFIG_HOME')
        if os.path.exists('piglit.conf'):
            shutil.move('piglit.conf', 'piglit.conf.restore')
            self.defer(shutil.move, 'piglit.conf.restore', 'piglit.conf')

        with utils.tempdir() as tdir:
            self.defer(os.chdir, os.getcwd())
            os.chdir(tdir)

            with open(os.path.join(tdir, 'piglit.conf'), 'w') as f:
                f.write(CONF_FILE)

            core.get_config()

        nt.ok_(core.PIGLIT_CONFIG.has_section('nose-test'),
               msg='./piglit.conf not found')


class TestGetConfigRoot(_TestWithEnvClean):
    def test(self):
        """ get_config() finds "piglit root"/piglit.conf """
        self.defer(lambda: core.PIGLIT_CONFIG == ConfigParser.SafeConfigParser)
        self.add_teardown('HOME')
        self.add_teardown('XDG_CONFIG_HOME')

        if os.path.exists('piglit.conf'):
            shutil.move('piglit.conf', 'piglit.conf.restore')
            self.defer(shutil.move, 'piglit.conf.restore', 'piglit.conf')

        with open('piglit.conf', 'w') as f:
            f.write(CONF_FILE)
        self.defer(os.unlink, 'piglit.conf')
        self.defer(os.chdir, os.getcwd())
        os.chdir('..')

        core.get_config()

        nt.ok_(core.PIGLIT_CONFIG.has_section('nose-test'),
               msg='$PIGLIT_ROOT not found')
