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
import textwrap
import framework.core as core
import framework.tests.utils as utils
import nose.tools as nt


def _reset_piglit_config():
    """ Set core.PIGLIT_CONFIG back to pristine """
    core.PIGLIT_CONFIG = ConfigParser.SafeConfigParser()


class TestGetConfig(utils.TestWithEnvClean):
    CONF_FILE = textwrap.dedent("""
    [nose-test]
    ; a section for testing behavior
    dir = foo
    """)

    def __unset_config(self):
        self.defer(_reset_piglit_config)
        self.add_teardown('XDG_CONFIG_HOME')
        self.add_teardown('HOME')

    def __move_local(self):
        """ Move a local piglit.conf so it isn't overwritten """
        if os.path.exists('piglit.conf'):
            shutil.move('piglit.conf', 'piglit.conf.restore')
            self.defer(shutil.move, 'piglit.conf.restore', 'piglit.conf')

    def setup(self):
        self.__unset_config()
        self.__move_local()

    def test_xdg_config_home(self):
        """ get_config() finds $XDG_CONFIG_HOME/piglit.conf """
        with utils.tempdir() as tdir:
            os.environ['XDG_CONFIG_HOME'] = tdir
            with open(os.path.join(tdir, 'piglit.conf'), 'w') as f:
                f.write(TestGetConfig.CONF_FILE)
            core.get_config()

        nt.ok_(core.PIGLIT_CONFIG.has_section('nose-test'),
               msg='$XDG_CONFIG_HOME not found')

    def test_config_home_fallback(self):
        """ get_config() finds $HOME/.config/piglit.conf """
        with utils.tempdir() as tdir:
            os.environ['HOME'] = tdir
            os.mkdir(os.path.join(tdir, '.config'))
            with open(os.path.join(tdir, '.config/piglit.conf'), 'w') as f:
                f.write(TestGetConfig.CONF_FILE)
            core.get_config()

        nt.ok_(core.PIGLIT_CONFIG.has_section('nose-test'),
               msg='$HOME/.config/piglit.conf not found')

    def test_local(self):
        """ get_config() finds ./piglit.conf """
        with utils.tempdir() as tdir:
            self.defer(os.chdir, os.getcwd())
            os.chdir(tdir)

            with open(os.path.join(tdir, 'piglit.conf'), 'w') as f:
                f.write(TestGetConfig.CONF_FILE)

            core.get_config()

        nt.ok_(core.PIGLIT_CONFIG.has_section('nose-test'),
               msg='./piglit.conf not found')

    def test_piglit_root(self):
        """ get_config() finds "piglit root"/piglit.conf """
        with open('piglit.conf', 'w') as f:
            f.write(TestGetConfig.CONF_FILE)
        self.defer(os.unlink, 'piglit.conf')
        self.defer(os.chdir, os.getcwd())
        os.chdir('..')

        core.get_config()

        nt.ok_(core.PIGLIT_CONFIG.has_section('nose-test'),
               msg='$PIGLIT_ROOT not found')
