# Copyright 2014 Intel Corporation
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

import ConfigParser
import os
import subprocess

# Piglit modules
import framework
from framework.profile import Test, TestProfile

__all__ = ['profile']

def get_option(env_varname, config_option):
    """Query the given environment variable and then piglit.conf for the
    option. Return None if the option is unset.
    """
    opt = os.environ.get(env_varname, None)
    if opt is not None:
        return opt

    try:
        opt = framework.core.PIGLIT_CONFIG.get(config_option[0],
                                               config_option[1])
    except ConfigParser.NoSectionError:
        pass
    except ConfigParser.NoOptionError:
        pass

    return opt


# Path to the deqp-gles3 executable.
DEQP_GLES3_EXE = get_option('PIGLIT_DEQP_GLES3_EXE', ('deqp-gles3', 'exe'))

DEQP_GLES3_EXTRA_ARGS = get_option('PIGLIT_DEQP_GLES3_EXTRA_ARGS',
                                   ('deqp-gles3', 'extra_args'))


def gen_caselist_txt():
    """Generate dEQP-GLES3-cases.txt and return its path."""
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
    basedir = os.path.dirname(DEQP_GLES3_EXE)
    caselist_path = os.path.join(basedir, 'dEQP-GLES3-cases.txt')
    subprocess.check_call(
        [DEQP_GLES3_EXE, '--deqp-runmode=txt-caselist'],
        cwd=basedir)
    assert os.path.exists(caselist_path)
    return caselist_path


def iter_deqp_test_cases():
    """Iterate over original dEQP GLES3 testcase names."""
    caselist_path = gen_caselist_txt()
    with open(caselist_path) as caselist_file:
        for i, line in enumerate(caselist_file):
            if line.startswith('GROUP:'):
                continue
            elif line.startswith('TEST:'):
                yield line[len('TEST:'):].strip()
            else:
                raise Exception('{0}:{1}: ill-formed line'.format(caselist_path, i))


class DEQPTest(Test):

    __RESULT_MAP = {"Pass"           : "pass",
                    "Fail"           : "fail",
                    "QualityWarning" : "warn",
                    "InternalError"  : "fail",
                    "Crash"          : "crash",
                    "NotSupported"   : "skip"}

    def __init__(self, case_name):
        command = [DEQP_GLES3_EXE, '--deqp-case=' + case_name]
        if DEQP_GLES3_EXTRA_ARGS is not None:
            command.extend(DEQP_GLES3_EXTRA_ARGS.split())
        super(DEQPTest, self).__init__(command)

        # dEQP's working directory must be the same as that of the executable,
        # otherwise it cannot find its data files (2014-12-07).
        self.cwd = os.path.dirname(DEQP_GLES3_EXE)

    def interpret_result(self):
        if self.result['returncode'] != 0:
            self.result['result'] = 'fail'
            return

        for line in self.result['out'].split('\n'):
            line = line.lstrip()
            for k, v in DEQPTest.__RESULT_MAP.iteritems():
                if line.startswith(k):
                    self.result['result'] = v
                    return

        # We failed to parse the test output. Fallback to 'fail'.
        self.result['result'] = 'fail'

def add_tests():
    if DEQP_GLES3_EXE is None:
        return

    for deqp_testname in iter_deqp_test_cases():
        # dEQP uses '.' as the testgroup separator.
        piglit_testname = deqp_testname.replace('.', '/')

        test = DEQPTest(deqp_testname)
        profile.test_list[piglit_testname] = test


profile = TestProfile()
add_tests()
