# coding=utf-8
#
# Copyright (c) 2019 Collabora Ltd
# Copyright © 2019-2020 Valve Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# SPDX-License-Identifier: MIT

import logging
import contextlib
import os
import pytest
import re
import shutil
import textwrap

from os import path

from framework.replay import backends
from framework.replay.programs import compare
from framework.replay.compare_replay import Result

from .backends import testtrace


TESTS_OUTPUT = "results/output.txt"
TRACE_PNG_TEST1 = "results/trace/gl-test-device/trace1/8e0a801367e1714463475a824dab363b.png"
TRACE_PNG_TEST2 = "results/trace/vk-test-device/trace2/5efda83854befe0155ff8517a58d5b51.png"

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger()

backends.DUMPBACKENDS["testtrace"] = testtrace.REGISTRY

def write_to(content, filename):
    with open(filename, 'w') as f:
        f.write(content)


def read_from(filename):
    with open(filename) as f:
        content = f.read()
        return content


def run_compare(extra_args):
    '''
    Run tests for the .testtrace types, using the "gl-test-device" and
    "vk-test-device" device names.
    '''
    result = 0
    os.makedirs(path.dirname(TESTS_OUTPUT), exist_ok=True)
    with open(TESTS_OUTPUT, 'w') as f:
        with contextlib.redirect_stdout(f):
            result = compare.compare(["yaml",
                                      "--device-name", "gl-test-device",
                                      "--yaml-file", "./tests/traces.yml"] + extra_args)
    if result is not Result.MATCH:
        return result
    with open(TESTS_OUTPUT, 'a') as f:
        with contextlib.redirect_stdout(f):
            result = compare.compare(["yaml",
                                      "--device-name", "vk-test-device",
                                      "--yaml-file", "./tests/traces.yml"] + extra_args)
    return result


def prepare_for_run(tmp_path):
    '''
    Copy all the tests data to the test dir for the unit tests.
    This avoids polluting the normal working dir with test result artifacts.
    '''
    test_dir = path.join(str(tmp_path) , "run")
    # Copy the tests
    shutil.copytree(path.join(path.dirname(path.realpath(__file__)), "tests"),
                    path.join(test_dir, "tests"))
    # Change the working dir to the test_dir
    os.chdir(test_dir)
    # Set the replayer-db
    shutil.move("./tests/test-data", "./replayer-db/")


def cleanup(tmp_path):
    '''
    Performs the clean up of the test dir.
    '''
    if path.exists(tmp_path):
        shutil.rmtree(tmp_path)


@pytest.fixture(autouse=True)
def run_test(tmp_path):
    '''
    Wraps the execution of each test as follows:

      prepare_for_run()
      test()
      cleanup()
    '''
    logger.debug("Working dir: %s", tmp_path)
    prepare_for_run(tmp_path)
    yield
    cleanup(tmp_path)


def check_test_output(filename, expectations):
    '''
    Checks the content of the filename with the list of expectations
    passed as parameter.

    Arguments:
        filename (str): The path of the file to check
        expectations (list): A list with the content to find in the file

    Returns:
        bool: The return value. True if the content of the filename satisfies
              the expectations, False otherwise.
    '''
    content = read_from(filename)
    for e in expectations:
        ocurrencies = re.findall(e, content)
        if not len(ocurrencies):
            logger.error("Expectation not found in %s: %s", filename, e)
            return False
    return True


def test_compare_succeeds_if_all_images_match():
    assert run_compare([]) is Result.MATCH
    expectations = [
        "actual: 5efda83854befe0155ff8517a58d5b51",
        "expected: 5efda83854befe0155ff8517a58d5b51",
    ]
    assert check_test_output(TESTS_OUTPUT, expectations)


def test_compare_fails_on_image_mismatch():
    filename = "./tests/traces.yml"
    content = read_from(filename)
    content = content.replace("5efda83854befe0155ff8517a58d5b51",
                              "8e0a801367e1714463475a824dab363b")
    write_to(content, filename)
    assert run_compare([]) is Result.DIFFER
    expectations = [
        "actual: 5efda83854befe0155ff8517a58d5b51",
        "expected: 8e0a801367e1714463475a824dab363b",
        "trace/vk-test-device/trace2/olive.testtrace-0.png"
    ]
    assert check_test_output(TESTS_OUTPUT, expectations)


def test_compare_traces_with_and_without_checksum():
    filename = "./tests/traces.yml"
    content = read_from(filename)
    extra = textwrap.dedent('''\
    - path: trace1/red.testtrace
      expectations:
        - device: blah
          checksum: 000000000000000
    ''')
    content += textwrap.indent(extra, '  ')
    write_to(content, filename)

    # red.testtrace should be skipped, since it doesn't
    # have any checksums for our device
    filename = "./replayer-db/trace1/red.testtrace"
    content = "ff0000ff"
    write_to(content, filename)
    assert run_compare([]) is Result.MATCH


def test_compare_only_traces_without_checksum():
    filename = "./tests/traces.yml"
    content = textwrap.dedent('''\
    traces:
      - path: trace1/red.testtrace
        expectations:
          - device: blah
            checksum: 000000000000000
    ''')
    write_to(content, filename)

    # red.testtrace should be skipped, since it doesn't
    # have any checksums for our device
    filename = "./replayer-db/trace1/red.testtrace"
    content = "ff0000ff"
    write_to(content, filename)
    assert run_compare([]) is Result.MATCH


def test_compare_with_no_traces():
    filename = "./tests/traces.yml"
    content = 'traces:'
    write_to(content, filename)
    assert run_compare([]) is Result.MATCH
    # Check the file is empty
    assert len(read_from(TESTS_OUTPUT)) == 0


def test_compare_fails_on_dump_image_error():
    # "invalid" should fail to parse as rgba and
    # cause an error
    filename = "./replayer-db/trace1/magenta.testtrace"
    write_to("invalid\n", filename)
    run_compare([])
    expectations = [
        "actual: error",
        "expected: 8e0a801367e1714463475a824dab363b",
        "trace1/magenta.testtrace",
    ]
    assert check_test_output(TESTS_OUTPUT, expectations)


def test_compare_stores_only_logs_on_checksum_match():
    assert run_compare([]) is Result.MATCH
    assert not path.exists(TRACE_PNG_TEST1)
    assert not path.exists(TRACE_PNG_TEST2)


def test_compare_stores_images_on_checksum_mismatch():
    filename = "./tests/traces.yml"
    content = read_from(filename)
    content = content.replace("5efda83854befe0155ff8517a58d5b51",
                              "8e0a801367e1714463475a824dab363b")
    write_to(content, filename)
    assert run_compare([]) is Result.DIFFER
    assert not path.exists(TRACE_PNG_TEST1)
    assert path.exists(TRACE_PNG_TEST2)


def test_compare_stores_images_on_request():
    assert run_compare(["--keep-image"]) is Result.MATCH
    assert path.exists(TRACE_PNG_TEST1)
    assert path.exists(TRACE_PNG_TEST2)
