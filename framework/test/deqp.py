# Copyright 2014, 2015 Intel Corporation
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

import abc
import os
import subprocess

# Piglit modules
from framework import core, grouptools, exceptions
from framework.profile import Test, TestProfile

__all__ = [
    'DEQPBaseTest',
    'gen_caselist_txt',
    'get_option',
    'iter_deqp_test_cases',
    'make_profile',
]


def make_profile(test_list, test_class):
    """Create a TestProfile instance."""
    profile = TestProfile()
    for testname in test_list:
        # deqp uses '.' as the testgroup separator.
        piglit_name = testname.replace('.', grouptools.SEPARATOR)
        profile.test_list[piglit_name] = test_class(testname)

    return profile


def get_option(env_varname, config_option):
    """Query the given environment variable and then piglit.conf for the option.

    Return None if the option is unset.

    """
    opt = os.environ.get(env_varname, None)
    if opt is not None:
        return opt

    opt = core.PIGLIT_CONFIG.safe_get(config_option[0], config_option[1])

    return opt


def gen_caselist_txt(bin_, caselist):
    """Generate a caselist.txt and return its path."""
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
    subprocess.check_call(
        [bin_, '--deqp-runmode=txt-caselist'], cwd=basedir)
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


class DEQPBaseTest(Test):
    __metaclass__ = abc.ABCMeta
    __RESULT_MAP = {"Pass": "pass",
                    "Fail": "fail",
                    "QualityWarning": "warn",
                    "InternalError": "fail",
                    "Crash": "crash",
                    "NotSupported": "skip"}

    @abc.abstractproperty
    def deqp_bin(self):
        """The path to the exectuable."""

    @abc.abstractproperty
    def extra_args(self):
        """Extra arguments to be passed to the each test instance.

        Needs to return a list, since self.command uses the '+' operator, which
        only works to join two lists together.

        """

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

    def interpret_result(self):
        if self.result['returncode'] != 0:
            self.result['result'] = 'fail'
            return

        for line in self.result['out'].split('\n'):
            line = line.lstrip()
            for k, v in self.__RESULT_MAP.iteritems():
                if line.startswith(k):
                    self.result['result'] = v
                    return

        # We failed to parse the test output. Fallback to 'fail'.
        self.result['result'] = 'fail'
