#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.


import sys

import framework.core as core


def test_iterations(*parameters):
    """ Magic that allows a single method to create a whole bunch of functions.

    This is desirable because of the way unittest works: Each method gets a
    name in the output, and each method stops on the first error. This makes
    using a loop useless, if 10/20 iterations should fail, the first failure
    stops the loop.  The solution other than using a decorator is to create a
    test for each iteration, which could result in hundreds of tests.

    """

    def decorator(method, parameters=parameters):
        def tuplify(x):
            if not isinstance(x, tuple):
                return (x, )
            return x

        for parameter in map(tuplify, parameters):
            name_for_parameter = "{0}({1})".format(method.__name__,
                                                   ", ".join(map(repr, parameter)))
            frame = sys._getframe(1)  # pylint: disable-msg=W0212
            frame.f_locals[name_for_parameter] = \
                lambda self, m=method, p=parameter: m(self, *p)
        return None
    return decorator


def create_testresult(name, lspci="fake lspci", glxinfo="fake glxinfo",
                      tests={}):
    """ Helper function that returns a complete TestResults file

    This takes one required argument
    :name: This must be set, it names the run. If two runs have the same name
           there can be problems in the summary code.

    This function also takes three optional arguments
    :lspci:   This is the lspci information in the file. Default="fake lspci"
    :glxinfo: glxinfo in the file. Default="fake glxinfo"
    :tests:   A dictionary of tests

    """
    assert(isinstance(tests, dict))

    return core.TestResult({"options": {"profile": "fake",
                                        "filter": [],
                                        "exclude_filter": []},
                            "name": "{0}".format(name),
                            "lspci": "{0}".format(lspci),
                            "glxinfo": "{0}".format(glxinfo),
                            "time_elapsed": 10.23456789,
                            "tests": tests})


def create_test(name, result, info="fake info", returncode=0, time=0.123456789,
                command="fake command"):
    """ Helper function that takes input and returns a dictionary of results
    for a single tests

    Takes two required arguments:
    :name: The name of the tests
    :result: The result the test returned

    Additionally takes four optional arguments:
    :info:       The info entry. Default="fake info"
    :returncode: The returncode of the test. Default=0
    :time:       The amount of time the test ran. Default=0.123456789
    :command:    The command that was executed. Default="fake command"

    """
    #FIXME: This should be able to create other entries for failed tests

    return {name: {"info": info,
                   "returncode": returncode,
                   "time": time,
                   "command": command,
                   "result": result}}
