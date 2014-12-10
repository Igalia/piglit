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

""" Provides tests for the dmesg class

Tests that require sudo have sudo in their name, if you don't have sudo or
don't want to run them use '-e sudo' with nosetests

"""

from __future__ import print_function, absolute_import
import os
import sys
import subprocess
import re
import json

import nose.tools as nt
from nose.plugins.skip import SkipTest

import framework.dmesg as dmesg
import framework.core
import framework.test
import framework.backends
import framework.tests.utils as utils


# Helpers
def _get_dmesg():
    """ checks to ensure dmesg is not DummyDmesg, raises skip if it is

    If we are on a non-posix system we will get a dummy dmesg, go ahead and
    skip in that case
    """
    test = dmesg.get_dmesg()
    if isinstance(test, dmesg.DummyDmesg):
        raise SkipTest("A DummyDmesg was returned, testing dmesg impossible.")
    return test


def _write_dev_kmesg():
    """ Try to write to /dev/kmesg, skip if not possible

    Writing to the dmesg ringbuffer at /dev/kmesg ensures that we varies
    functionality in the LinuxDmesg class will go down the
    dmesg-has-new-entries path.

    If we anything goes wrong here just skip.
    """
    err = subprocess.call(['sudo', 'sh', '-c',
                           'echo "piglit dmesg test" > /dev/kmsg'])
    if err != 0:
        raise SkipTest("Writing to the ringbuffer failed")


class DummyJsonWriter(object):
    """ A very simple dummy for json writer """
    def __init__(self):
        self.result = None

    def write_dict_item(self, _, result):
        self.result = result


class DummyLog(object):
    """ A very smiple dummy for the Logger """
    def __init__(self):
        pass

    def pre_log(self, *args):
        return None

    def log(self, *args):
        pass

    def post_log(self, *args):
        pass


class TestDmesg(dmesg.BaseDmesg):
    """ A special implementation of Dmesg that is easy to test with """
    def update_dmesg(self):
        pass


# Tests
def test_linux_initialization():
    """ Test that LinuxDmesg initializes """
    dmesg.LinuxDmesg()


def test_dummy_initialization():
    """ Test that DummyDmesg initializes """
    dmesg.DummyDmesg()


def test_get_dmesg_dummy():
    """ Test that get_dmesg function returns a Dummy when asked """
    dummy = dmesg.get_dmesg(not_dummy=False)
    nt.assert_is(type(dummy), dmesg.DummyDmesg,
                 msg=("Error: get_dmesg should have returned DummyDmesg, "
                      "but it actually returned {}".format(type(dummy))))


def test_get_dmesg_linux():
    """ Test that get_dmesg() returns a LinuxDmesg instance when asked """
    if not sys.platform.startswith('linux'):
        raise SkipTest("Cannot test a LinuxDmesg on a non linux system")
    posix = _get_dmesg()
    nt.assert_is(type(posix), dmesg.LinuxDmesg,
                 msg=("Error: get_dmesg should have returned LinuxDmesg, "
                      "but it actually returned {}".format(type(posix))))


def sudo_test_update_dmesg_with_updates():
    """ update_dmesg() updates results when there is a new entry in dmesg

    This will skip on non-Posix systems, since there is no way to actually test
    it.

    Because this test needs to write into the dmesg ringbuffer to assure that
    the ringbuffer has changed and that our class successfully catches that
    change it requires root access, gained by sudo. In the event that it cannot
    get sudo it will skip.

    """
    test = _get_dmesg()
    _write_dev_kmesg()

    test.update_dmesg()
    nt.assert_not_equal(test._new_messages, [],
                        msg=("{0} does not return updates, even when dmesg "
                             "has been updated.".format(test.__class__)))


def sudo_test_update_dmesg_without_updates():
    """ update_dmesg() does not update results when there is no change in dmesg

    This will skip on non-Posix systems, since there is no way to actually test
    it.

    Because this test needs to write into the dmesg ringbuffer to assure that
    the ringbuffer has changed and that our class successfully catches that
    change it requires root access, gained by sudo. In the event that it cannot
    get sudo it will skip.

    """
    test = _get_dmesg()
    test.update_dmesg()
    nt.assert_equal(test._new_messages, [],
                    msg=("{0} returned updates, even when dmesg has not been "
                         "updated.".format(test.__class__)))


def test_dmesg_wrap_partial():
    """ Test that dmesg still works after dmesg wraps partially

    We can overwrite the DMESG_COMMAND class variable to emluate dmesg being
    filled up and overflowing. What this test does is starts with a string that
    looks like this: "a\nb\nc\n" (this is used to emluate the contents of
    dmesg), we then replace that with "b\nc\nd\n", and ensure that the update
    of dmesg contains only 'd', becasue 'd' is the only new value in the
    updated dmesg.

    """
    # We don't want weird side effects of changing DMESG_COMMAND globally, so
    # instead we set it as a class instance and manually clear the
    # _last_messages attribute
    test = dmesg.LinuxDmesg()
    test.DMESG_COMMAND = ['echo', 'a\nb\nc\n']
    test.update_dmesg()

    # Update the DMESG_COMMAND to add d\n and remove a\n, this simluates the
    # wrap
    test.DMESG_COMMAND = ['echo', 'b\nc\nd\n']
    test.update_dmesg()

    nt.assert_items_equal(test._new_messages, ['d'],
                          msg=("_new_messages should be equal to ['d'], but is"
                               " {} instead.".format(test._new_messages)))


def test_dmesg_wrap_complete():
    """ Test that dmesg still works after dmesg wraps completely

    just like the partial version, but with nothingin common.

    """
    # We don't want weird side effects of changing DMESG_COMMAND globally, so
    # instead we set it as a class instance and manually clear the
    # _last_messages attribute
    test = dmesg.LinuxDmesg()
    test.DMESG_COMMAND = ['echo', 'a\nb\nc\n']
    test.update_dmesg()

    # Udamte the DMESG_COMMAND to add d\n and remove a\n, this simluates the
    # wrap
    test.DMESG_COMMAND = ['echo', '1\n2\n3\n']
    test.update_dmesg()

    nt.assert_items_equal(test._new_messages, ['1', '2', '3'],
                          msg=("_new_messages should be equal to "
                               "['1', '2', '3'], but is {} instead".format(
                                   test._new_messages)))


@utils.nose_generator
def test_update_result_replace():
    """ Generates tests for update_result """

    def create_test_result(res):
        result = framework.results.TestResult()
        result['result'] = res
        result['subtest'] = {}
        result['subtest']['test'] = res
        return result

    dmesg = TestDmesg()

    for res in ['pass', 'fail', 'crash', 'warn', 'skip', 'notrun']:
        dmesg.regex = None
        dmesg._new_messages = ['add', 'some', 'stuff']
        new_result = dmesg.update_result(create_test_result(res))

        check_update_result.description = "Test update_result: {0}".format(res)
        yield check_update_result, new_result['result'], res

        check_update_result.description = \
            "Test update_result subtest: {0}".format(res)
        yield check_update_result, new_result['subtest']['test'], res

        # check that the status is not updated when Dmesg.regex is set and does
        # not match the dmesg output
        dmesg.regex = re.compile("(?!)")
        dmesg._new_messages = ['more', 'new', 'stuff']
        new_result = dmesg.update_result(create_test_result(res))

        check_equal_result.description = \
            "Test update_result with non-matching regex: {0}".format(res)
        yield check_equal_result, new_result['result'], res

        # check that the status is updated when Dmesg.regex is set and matches
        # the dmesg output
        dmesg.regex = re.compile("piglit.*test")
        dmesg._new_messages = ['piglit.awesome.test', 'and', 'stuff']
        new_result = dmesg.update_result(create_test_result(res))

        check_update_result.description = \
            "Test update_result with matching regex: {0} ".format(res)
        yield check_update_result, new_result['result'], res


def check_equal_result(result, status):
    """ Tests that the result and status are equal

    Dmesg.update_results() should not change the status if Dmesg.regex is set
    and the dmesg output did not match it.

    """

    nt.assert_equal(result, status,
                    msg="status should not have changed from {} to {}".format(
                        status, result))


def check_update_result(result, status):
    """ Tests that update result replaces results correctly

    Dmesg.update_results() should take a TestResult instance and replace the
    result instance with a dmesg-statuses when appropriate. Appropriate
    instances to be replaced are: pass, warn, fail.

    """
    if status == "pass":
        nt.assert_equal(result, 'dmesg-warn',
                        msg="pass should be replaced with dmesg-warn")
    elif status in ['warn', 'fail']:
        nt.assert_equal(result, 'dmesg-fail',
                        msg="{} should be replaced with dmesg-fail".format(
                            status))
    else:
        nt.assert_equal(result, status,
                        msg="{} should not have changed, but it did.".format(
                            result))


def test_update_result_add_dmesg():
    """ Tests update_result's addition of dmesg attribute """
    test = TestDmesg()

    result = framework.results.TestResult()
    result['result'] = 'pass'

    test._new_messages = ['some', 'new', 'messages']
    result = test.update_result(result)

    nt.assert_in('dmesg', result,
                 msg="result does not have dmesg member but should")


def test_json_serialize_updated_result():
    """ Test that a TestResult that has been updated is json serializable """
    test = TestDmesg()

    result = framework.results.TestResult()
    result['result'] = 'pass'

    test._new_messages = ['some', 'new', 'messages']
    result = test.update_result(result)

    encoder = json.JSONEncoder(default=framework.backends.piglit_encoder)
    encoder.encode(result)


@utils.privileged_test
@utils.nose_generator
def test_testclasses_dmesg():
    """ Generator that creates tests for """
    lists = [(framework.test.PiglitTest,
              ['attribs', '-auto', '-fbo'], 'PiglitTest'),
             (framework.test.GleanTest, 'basic', "GleanTest"),
             (framework.test.ShaderTest,
              'tests/shaders/loopfunc.shader_test', 'ShaderTest'),
             (framework.test.GLSLParserTest,
              'tests/glslparsertest/shaders/main1.vert', 'GLSLParserTest')]

    for tclass, tfile, desc in lists:
        check_classes_dmesg.description = "Test dmesg in {}".format(desc)
        yield check_classes_dmesg, tclass, tfile


def check_classes_dmesg(test_class, test_args):
    """ Do the actual check on the provided test class for dmesg """
    if not os.path.exists('bin'):
        raise SkipTest("This tests requires a working, built version of "
                       "piglit")

    test = _get_dmesg()

    # Create the test and then write to dmesg to ensure that it actually works
    test = test_class(test_args)
    test._test_hook_execute_run = _write_dev_kmesg

    json = DummyJsonWriter()

    test.execute(None, DummyLog(), json, test)

    nt.assert_in(json.result['result'], ['dmesg-warn', 'dmesg-fail'],
                 msg="{0} did not update status with dmesg".format(type(test)))
