# Copyright 2014-2016 Intel Corporation
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

"""This module provides integration for dEQP into piglit."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import abc
import os
import subprocess
try:
    from lxml import etree as et
except ImportError:
    from xml.etree import cElementTree as et

import six
from six.moves import range

from framework import core, grouptools, exceptions
from framework import options
from framework import status
from framework.test import base
from framework.profile import TestProfile

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


class DEQPUnsupportedMode(exceptions.PiglitInternalError):
    """An exception raised when dEQP/piglit doesn't support the given mode.

    Primarily this is meant to be used in the deqp_gles* modules, where
    group-at-a-time can actually make them slower.
    """


def select_source(bin_, filename, mustpass, extra_args):
    """Return either the mustpass list or the generated list."""
    if options.OPTIONS.deqp_mustpass:
        return gen_mustpass_tests(mustpass)
    else:
        return iter_deqp_test_cases(
            gen_caselist_txt(bin_, filename, extra_args))


def make_profile(test_list, single=None, group=None):
    """Create a TestProfile instance."""
    if options.OPTIONS.deqp_mode == "single":
        if isinstance(single, DEQPUnsupportedMode):
            raise exceptions.PiglitFatalError(single)
        test_class = single
    else:
        if isinstance(group, DEQPUnsupportedMode):
            raise exceptions.PiglitFatalError(group)
        test_class = group

    profile = TestProfile()
    for testname in test_list:
        # deqp uses '.' as the testgroup separator.
        piglit_name = testname[0].replace('.', grouptools.SEPARATOR)
        profile.test_list[piglit_name] = test_class(*testname)

    return profile


def gen_mustpass_tests(mp_list):
    """Return a testlist from the mustpass list."""
    root = et.parse(mp_list).getroot()
    cur_group = []

    def single(root):
        for elem in root:
            if elem.tag == 'Test':
                yield ('{}.{}'.format('.'.join(cur_group), elem.get('name')), )
            else:
                cur_group.append(elem.get('name'))
                for test in single(elem):
                    yield test
                del cur_group[-1]

    def group(root):
        for elem in root:
            if elem.tag == 'TestCase':
                case = '{}.{}'.format('.'.join(cur_group), elem.get('name'))
                yield (case, ['{}.{}'.format(case, t.get('name'))
                              for t in elem.findall('.//Test')])
            elif elem.tag == 'TestSuite':
                cur_group.append(elem.get('name'))
                for test in group(elem):
                    yield test
                del cur_group[-1]

    if options.OPTIONS.deqp_mode == 'single':
        gen = single
    else:
        gen = group

    for test in gen(root):
        yield test


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

    with open(os.devnull, 'w') as d:
        subprocess.check_call(
            [bin_, '--deqp-runmode=txt-caselist'] + extra_args, cwd=basedir,
            stdout=d, stderr=d)
    assert os.path.exists(caselist_path)
    return caselist_path


def iter_deqp_test_cases(case_file):
    """Iterate over original dEQP testcase names."""
    def single(f):
        """Iterate over the txt file, and yield each test instance."""
        for i, line in enumerate(f):
            if line.startswith('GROUP:'):
                continue
            elif line.startswith('TEST:'):
                # The group mode yields a tuple, so the single mode needs to as
                # well.
                yield (line[len('TEST:'):].strip(), )
            else:
                raise exceptions.PiglitFatalError(
                    'deqp: {}:{}: ill-formed line'.format(f.name, i))

    def group(f):
        """Iterate over the txt file, and yield each group and its members.

        The group must contain actual members to be yielded.
        """
        group = ''
        tests = []

        for i, line in enumerate(f):
            if line.startswith('GROUP:'):
                new = line[len('GROUP:'):].strip()
                if group != new and tests:
                    yield (group, tests)
                    tests = []
                group = new
            elif line.startswith('TEST:'):
                tests.append(line[len('TEST:'):].strip())
            else:
                raise exceptions.PiglitFatalError(
                    'deqp: {}:{}: ill-formed line'.format(f.name, i))
        # If we get to the end of the file and we have new tests (the would
        # have been cleared if there weren't any.
        if tests:
            yield (group, tests)

    adder = None
    if options.OPTIONS.deqp_mode == 'single':
        adder = single
    elif options.OPTIONS.deqp_mode == 'group':
        adder = group
    assert adder is not None

    with open(case_file, 'r') as f:
        for x in adder(f):
            yield x


def format_trie_list(classname, testnames):
    """Create a trie list from classname and testnames.

    dEQP doesn't accept multiple --deqp-case/-n flags, so we need this.  It's
    called Trie, and it's ugly as heck. Trie is like
    {group1{group2{test1,test2}}} (as a simple example. Every other method
    involves creating files, so it's really the only viable method.

    This string looks like gobly-gook, but basicaly in python formatted
    strings a '{{' becomes a '{' (since {} has special meaning).
    """
    return '--deqp-caselist={{{0}{{{1}}}{2}'.format(
        '{'.join(classname), ','.join(testnames), '}' * len(classname))


class _DEQPCase(object):
    """Mixin to provide the --deqp-case argument."""

    def __init__(self, case_name, *args, **kwargs):
        super(_DEQPCase, self).__init__('--deqp-case=' + case_name, *args, **kwargs)


@six.add_metaclass(abc.ABCMeta)
class DEQPBaseTest(base.Test):
    """A shared base class that provides some shared methods and attributes for
    use by both the Single and Group classes.
    """

    _RESULT_MAP = {
        "Pass": status.PASS,
        "Fail": status.FAIL,
        "QualityWarning": status.WARN,
        "InternalError": status.FAIL,
        "Crash": status.CRASH,
        "NotSupported": status.SKIP,
        "ResourceError": status.CRASH,
    }

    def __init__(self, command):
        command = [self.deqp_bin, command]

        super(DEQPBaseTest, self).__init__(command)

        # dEQP's working directory must be the same as that of the executable,
        # otherwise it cannot find its data files (2014-12-07).
        # This must be called after super or super will overwrite it
        self.cwd = os.path.dirname(self.deqp_bin)

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

    # The error of the getter is a known bug in pylint
    # https://github.com/PyCQA/pylint/issues/844
    @base.Test.command.getter  # pylint: disable=no-member
    def command(self):
        """Return the command plus any extra arguments."""
        command = super(DEQPBaseTest, self).command
        return command + self.extra_args


# Pylint can't figure out the six magic.
@six.add_metaclass(abc.ABCMeta)  # pylint: disable=abstract-method
class DEQPSingleTest(_DEQPCase, DEQPBaseTest):
    """Base test class for dEQP tests to run a single test per process."""

    def __find_map(self):
        """Run over the lines and set the result."""
        # splitting this into a separate function allows us to return cleanly,
        # otherwise this requires some break/else/continue madness
        for line in self.result.out.split('\n'):
            line = line.lstrip()
            for k, v in six.iteritems(self._RESULT_MAP):
                if line.startswith(k):
                    self.result.result = v
                    return

    def interpret_result(self):
        if base.is_crash_returncode(self.result.returncode):
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
            super(DEQPSingleTest, self)._run_command(*args, **kwargs)
            x_err_msg = "FATAL ERROR: Failed to open display"
            if x_err_msg in self.result.err or x_err_msg in self.result.out:
                continue
            return

        raise base.TestRunError('Failed to connect to X server 5 times', 'fail')


# Pylint can't figure out the six magic.
@six.add_metaclass(abc.ABCMeta)  # pylint: disable=abstract-method
class DEQPGroupTest(base.ReducedProcessMixin, DEQPBaseTest):
    """A class for running DEQP in a group at a time mode.

    With this class (in contrast to DEQPBaseTest), multiple tests will be run
    in a single process ("group-at-a-time"), this reduces reliablity somewhat,
    but vastly reduces runtime, which may be an acceptable tradoff.

    Each test is stored as a subtest, and the stdout and stderr is combined.

    This class is aware of which tests should be run by the process, and if the
    process ends in a non 0 status it will attempt to resume the tests.
    """

    def __init__(self, command, case_name, subtests):
        super(DEQPGroupTest, self).__init__(command, subtests=subtests)

        self.__casename = case_name

    def _populate_subtests(self):
        # Only add the subtest name, not the group name
        self.result.subtests.update(
            {self._subtest_name(x): status.NOTRUN for x in self._expected})

    def interpret_result(self):
        # We failed to parse the test output. Fallback to 'fail'.
        if base.is_crash_returncode(self.result.returncode):
            self.result.result = status.CRASH
        elif self.result.returncode != 0:
            self.result.result = status.FAIL

        current = None

        for line in self.result.out.split('\n'):
            if self._is_subtest(line):
                # There is one case where it is valid for current to be not
                # None, and that's when a case crashed, and the resume happens
                # the case will have no status, but it should have been marked
                # crash in that case, so if it's still NOTRUN then something is
                # wrong
                assert (current is None or
                        self.result.subtests[current] is not status.NOTRUN), \
                    'Two test cases opened?'

                # Get only the test name, not the group name
                current = self._subtest_name(line[:-3])
                assert current in self.result.subtests

            elif line.startswith('  '):
                try:
                    res = self._RESULT_MAP[line.lstrip().split(' ', 1)[0]]
                except KeyError:
                    # In general there are only two cases that that will match
                    # the elif and those are during the first and last lines
                    # of the output. In some dEQP based suites that number is
                    # higher (the GLES CTS for example). However the number is
                    # small enough that simply continue-ing on the exception
                    # is more efficient to try/except than to write an if
                    # statement that covers all of the potential cases.
                    continue

                assert current is not None, 'Result without case?'

                self.result.subtests[current] = res

                # That case is closed, reset current
                current = None

    def __calculate_resume(self, current):
        self.result.out += 'Splitting command to avoid too-long argument\n\n'
        val = ((current // 1000) + 1) * 1000
        if current % 1000 == 999:
            val += 1000
        return val

    def _resume(self, current):
        # The command needs to be the deqp binary, eachsubtest, and then the
        # extra args
        command = [self.command[0]]

        command.append(format_trie_list(
            self.__casename.split('.'),
            (self._get_test(s) for s in
             self._expected[current:self.__calculate_resume(current)])))
        assert len(command[-1]) < 65000, len(command[-1])
        command.extend(self.extra_args)
        return command

    def _is_subtest(self, line):
        return line.startswith("Test case '") and line.endswith("'..")

    @staticmethod
    def _get_test(test):
        return test.rsplit('.', 1)[1]

    def _subtest_name(self, test):
        return self._get_test(test).lower()


# Pylint can't figure out the six magic.
class DEQPGroupAsteriskTest(_DEQPCase, DEQPGroupTest):  # pylint: disable=abstract-method
    """A class that uses .* to construct case names.

    This class uses an asterisk append to the end of the group name
    ("my.group.*") to run multiple cases at a time. This has the advantage of
    being very succinct, and for the Vulkan CTS avoids the problem of building
    a "trie" command so long that the binary cannot parse it. But for other
    suites like the OpenGL CTS this isn't workable because it has groups that
    contain both tests and groups other groups.
    """

    def __init__(self, case_name, subtests):
        # This is not exactly correct since if someone were to turn on the
        # group at a time mode and try to use the mustpass list it would fail,
        # but piglit doesn't support that configuration because it's slower than
        # test at a time, and removes the protections of process isolation.
        super(DEQPGroupAsteriskTest, self).__init__(
            case_name + '.*', case_name, subtests=subtests)


# Pylint can't figure out the six magic.
class DEQPGroupTrieTest(DEQPGroupTest):  # pylint: disable=abstract-method
    """A class that uses trie lists to construct the testcase arguments.

    This class uses trie lists to construct the command for the test. This has
    the advantage of working correctly for groups that contain both tests and
    groups, but has the disadvantage of being more verbose, and lists can be
    constructed that are so large the dEQP binary can't parse them.
    """

    def __init__(self, case_name, subtests):
        super(DEQPGroupTrieTest, self).__init__(
            format_trie_list(
                case_name.split('.'),
                (self._get_test(s) for s in subtests)),
            case_name,
            subtests=subtests)
