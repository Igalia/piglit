# coding=utf-8
# Copyright 2014-2016,2018 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import abc
import os
import subprocess

import six
from six.moves import range

from framework import core, grouptools, exceptions
from framework import options
from framework.profile import TestProfile
from framework.test.base import Test, is_crash_returncode, TestRunError

__all__ = [
    'DEQPBaseTest',
    'gen_caselist_txt',
    'get_option',
    'iter_deqp_test_cases',
    'make_profile',
]


def get_option(env_varname, config_option, default=None, required=False):
    """Query the given environment variable and then piglit.conf for the option.

    Return the value of the default argument if opt is None.

    """
    opt = os.environ.get(env_varname, None)
    if opt is not None:
        return opt

    opt = core.PIGLIT_CONFIG.safe_get(config_option[0], config_option[1])

    if required and not opt:
        raise exceptions.PiglitFatalError(
            'Cannot get env "{}" or conf value "{}:{}"'.format(
                env_varname, config_option[0], config_option[1]))
    return opt or default


_EXTRA_ARGS = get_option('PIGLIT_DEQP_EXTRA_ARGS',
                         ('deqp', 'extra_args'),
                         default='').split()


def select_source(bin_, filename, mustpass, extra_args):
    """Return either the mustpass list or the generated list."""
    if options.OPTIONS.deqp_mustpass:
        return gen_mustpass_tests(mustpass)
    else:
        return iter_deqp_test_cases(
            gen_caselist_txt(bin_, filename, extra_args))


def make_profile(test_list, test_class):
    """Create a TestProfile instance."""
    profile = TestProfile()
    for testname in test_list:
        # deqp uses '.' as the testgroup separator.
        piglit_name = testname.replace('.', grouptools.SEPARATOR)
        profile.test_list[piglit_name] = test_class(testname)

    return profile


def gen_mustpass_tests(mustpass):
    """Return a testlist from the mustpass list."""
    with open(mustpass, 'r') as f:
        for l in f:
            l = l.strip()
            if l:
                yield l


def gen_caselist_txt(bin_, caselist, extra_args):
    """Generate a caselist.txt and return its path.

    Extra args should be a list of extra arguments to pass to deqp.

    """
    # dEQP is stupid (2014-12-07):
    #   1. To generate the caselist file, dEQP requires that the process's
    #      current directory must be that same as that of the executable.
    #      Otherwise, it fails to find its data files.
    #   2. dEQP creates the caselist file in the process's current directory
    #      and provides no option to change its location.
    #   3. dEQP creates a GL context when generating the caselist. Therefore,
    #      the caselist must be generated on the test target rather than the
    #      build host. In other words, when the build host and test target
    #      differ then we cannot pre-generate the caselist on the build host:
    #      we must *dynamically* generate it during the testrun.
    basedir = os.path.dirname(bin_)
    caselist_path = os.path.join(basedir, caselist)

    # TODO: need to catch some exceptions here...
    with open(os.devnull, 'w') as d:
        env = os.environ.copy()
        env['MESA_GL_VERSION_OVERRIDE'] = '4.6'
        env['MESA_GLES_VERSION_OVERRIDE'] = '3.2'

        subprocess.check_call(
            [bin_, '--deqp-runmode=txt-caselist'] + extra_args, cwd=basedir,
            stdout=d, stderr=d, env=env)
    assert os.path.exists(caselist_path)
    return caselist_path


def iter_deqp_test_cases(case_file):
    """Iterate over original dEQP testcase names."""
    with open(case_file, 'r') as caselist_file:
        for i, line in enumerate(caselist_file):
            if line.startswith('GROUP:'):
                continue
            elif line.startswith('TEST:'):
                yield line[len('TEST:'):].strip()
            else:
                raise exceptions.PiglitFatalError(
                    'deqp: {}:{}: ill-formed line'.format(case_file, i))


@six.add_metaclass(abc.ABCMeta)
class DEQPBaseTest(Test):
    __RESULT_MAP = {
        "Pass": "pass",
        "Fail": "fail",
        "QualityWarning": "warn",
        "InternalError": "fail",
        "Crash": "crash",
        "NotSupported": "skip",
        "ResourceError": "crash",
    }

    @abc.abstractproperty
    def deqp_bin(self):
        """The path to the exectuable."""

    @abc.abstractproperty
    def extra_args(self):
        """Extra arguments to be passed to the each test instance.

        Needs to return a list, since self.command uses the '+' operator, which
        only works to join two lists together.

        """
        return _EXTRA_ARGS

    def __init__(self, case_name):
        command = [self.deqp_bin, '--deqp-case=' + case_name]

        super(DEQPBaseTest, self).__init__(command)

        # dEQP's working directory must be the same as that of the executable,
        # otherwise it cannot find its data files (2014-12-07).
        # This must be called after super or super will overwrite it
        self.cwd = os.path.dirname(self.deqp_bin)

    @Test.command.getter
    def command(self):
        """Return the command plus any extra arguments."""
        command = super(DEQPBaseTest, self).command
        return command + self.extra_args

    def __find_map(self):
        """Run over the lines and set the result."""
        # splitting this into a separate function allows us to return cleanly,
        # otherwise this requires some break/else/continue madness
        for line in self.result.out.split('\n'):
            line = line.lstrip()
            for k, v in six.iteritems(self.__RESULT_MAP):
                if line.startswith(k):
                    self.result.result = v
                    return

    def interpret_result(self):
        if is_crash_returncode(self.result.returncode):
            self.result.result = 'crash'
        elif self.result.returncode != 0:
            self.result.result = 'fail'
        else:
            self.__find_map()

        # We failed to parse the test output. Fallback to 'fail'.
        if self.result.result == 'notrun':
            self.result.result = 'fail'

    def _run_command(self, *args, **kwargs):
        """Rerun the command if X11 connection failure happens."""
        for _ in range(5):
            super(DEQPBaseTest, self)._run_command(*args, **kwargs)
            x_err_msg = "FATAL ERROR: Failed to open display"
            if x_err_msg in self.result.err or x_err_msg in self.result.out:
                continue
            return

        raise TestRunError('Failed to connect to X server 5 times', 'fail')
