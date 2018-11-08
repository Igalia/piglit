# coding=utf-8
# Copyright (c) 2014-2016 Intel Corporation

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

"""Tests for JSON backend version updates."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os
try:
    import simplejson as json
except ImportError:
    import json
try:
    import mock
except ImportError:
    from unittest import mock

import jsonschema
import pytest

from framework import backends

# pylint: disable=protected-access,no-self-use


@pytest.yield_fixture(autouse=True, scope='module')
def setup_module():
    with mock.patch.dict(backends.json.compression.os.environ,
                         {'PIGLIT_COMPRESSION': 'none'}):
        yield


class TestV7toV8(object):
    """Tests for Version 7 to version 8."""

    data = {
        "results_version": 7,
        "name": "test",
        "options": {
            "profile": ['quick'],
            "dmesg": False,
            "verbose": False,
            "platform": "gbm",
            "sync": False,
            "valgrind": False,
            "filter": [],
            "concurrent": "all",
            "test_count": 0,
            "exclude_tests": [],
            "exclude_filter": [],
            "env": {},
        },
        "lspci": "stuff",
        "uname": "more stuff",
        "glxinfo": "and stuff",
        "wglinfo": "stuff",
        "clinfo": "stuff",
        "tests": {
            'a@test': {
                'time': 1.2,
                'dmesg': '',
                'result': 'fail',
                '__type__': 'TestResult',
                'command': '/a/command',
                'traceback': None,
                'out': '',
                'environment': 'A=variable',
                'returncode': 0,
                'err': '',
                'pid': 5,
                'subtests': {
                    '__type__': 'Subtests',
                },
                'exception': None,
            }
        },
        "time_elapsed": 1.2,
        '__type__': 'TestrunResult',
    }

    @pytest.fixture
    def result(self, tmpdir):
        p = tmpdir.join('result.json')
        p.write(json.dumps(self.data, default=backends.json.piglit_encoder))
        with p.open('r') as f:
            return backends.json._update_seven_to_eight(backends.json._load(f))

    def test_time(self, result):
        """backends.json.update_results (7 -> 8): test time is stored as start
        and end.
        """
        assert result['tests']['a@test']['time']['start'] == 0.0
        assert result['tests']['a@test']['time']['end'] == 1.2

    def test_time_elapsed(self, result):
        """backends.json.update_results (7 -> 8): total time is stored as start
        and end.
        """
        assert result['time_elapsed']['start'] == 0.0
        assert result['time_elapsed']['end'] == 1.2

    def test_valid(self, result):
        with open(os.path.join(os.path.dirname(__file__), 'schema',
                               'piglit-8.json'),
                  'r') as f:
            schema = json.load(f)
        jsonschema.validate(
            json.loads(json.dumps(result, default=backends.json.piglit_encoder)),
            schema)


class TestV8toV9(object):
    """Tests for Version 8 to version 9."""

    data = {
        "results_version": 8,
        "name": "test",
        "options": {
            "profile": ['quick'],
            "dmesg": False,
            "verbose": False,
            "platform": "gbm",
            "sync": False,
            "valgrind": False,
            "filter": [],
            "concurrent": "all",
            "test_count": 0,
            "exclude_tests": [],
            "exclude_filter": [],
            "env": {},
        },
        "lspci": "stuff",
        "uname": "more stuff",
        "glxinfo": "and stuff",
        "wglinfo": "stuff",
        "clinfo": "stuff",
        "tests": {
            'a@test': {
                "time": {
                    'start': 1.2,
                    'end': 1.8,
                    '__type__': 'TimeAttribute'
                },
                'dmesg': '',
                'result': 'fail',
                '__type__': 'TestResult',
                'command': '/a/command',
                'traceback': None,
                'out': '',
                'environment': 'A=variable',
                'returncode': 0,
                'err': '',
                'pid': 5,
                'subtests': {
                    '__type__': 'Subtests',
                },
                'exception': None,
            },
            'b@test': {
                "time": {
                    'start': 1.2,
                    'end': 1.8,
                    '__type__': 'TimeAttribute'
                },
                'dmesg': '',
                'result': 'fail',
                '__type__': 'TestResult',
                'command': '/a/command',
                'traceback': None,
                'out': '',
                'environment': 'A=variable',
                'returncode': 0,
                'err': '',
                'subtests': {
                    '__type__': 'Subtests',
                },
                'exception': None,
            }
        },
        "time_elapsed": {
            'start': 1.2,
            'end': 1.8,
            '__type__': 'TimeAttribute'
        },
        '__type__': 'TestrunResult',
    }

    @pytest.fixture
    def result(self, tmpdir):
        p = tmpdir.join('result.json')
        p.write(json.dumps(self.data, default=backends.json.piglit_encoder))
        with p.open('r') as f:
            return backends.json._update_eight_to_nine(backends.json._load(f))

    def test_pid(self, result):
        assert result['tests']['a@test']['pid'] == [5]

    def test_no_pid(self, result):
        assert result['tests']['b@test']['pid'] == []

    def test_valid(self, result):
        with open(os.path.join(os.path.dirname(__file__), 'schema',
                               'piglit-9.json'),
                  'r') as f:
            schema = json.load(f)
        jsonschema.validate(
            json.loads(json.dumps(result, default=backends.json.piglit_encoder)),
            schema)


class TestV9toV10(object):
    """Tests for Version 8 to version 9."""

    data = {
        "results_version": 9,
        "name": "test",
        "options": {
            "profile": ['quick'],
            "dmesg": False,
            "verbose": False,
            "platform": "gbm",
            "sync": False,
            "valgrind": False,
            "filter": [],
            "concurrent": "all",
            "test_count": 0,
            "exclude_tests": [],
            "exclude_filter": [],
            "env": {},
        },
        "lspci": "stuff",
        "uname": "more stuff",
        "glxinfo": "and stuff",
        "wglinfo": "stuff",
        "clinfo": "stuff",
        "tests": {
            'a@test': {
                "time": {
                    'start': 1.2,
                    'end': 1.8,
                    '__type__': 'TimeAttribute'
                },
                'dmesg': '',
                'result': 'fail',
                '__type__': 'TestResult',
                'command': '/a/command',
                'traceback': None,
                'out': '',
                'environment': 'A=variable',
                'returncode': 0,
                'err': '',
                'pid': [5],
                'subtests': {
                    '__type__': 'Subtests',
                },
                'exception': None,
            },
        },
        "time_elapsed": {
            'start': 1.2,
            'end': 1.8,
            '__type__': 'TimeAttribute'
        },
        '__type__': 'TestrunResult',
    }

    @pytest.fixture
    def result(self, tmpdir):
        p = tmpdir.join('result.json')
        p.write(json.dumps(self.data, default=backends.json.piglit_encoder))
        with p.open('r') as f:
            return backends.json._update_nine_to_ten(backends.json._load(f))

    @pytest.mark.parametrize("key", ['glxinfo', 'wglinfo', 'clinfo', 'uname', 'lspci'])
    def test(self, key, result):
        assert key not in result, 'Root key/value not removed'
        assert key in result['info']['system'], 'Key not added to info/system'
        assert result['info']['system'][key] == self.data[key], \
            'Value not set properly.'

    def test_valid(self, result):
        with open(os.path.join(os.path.dirname(__file__), 'schema',
                               'piglit-10.json'),
                  'r') as f:
            schema = json.load(f)
        jsonschema.validate(
            json.loads(json.dumps(result, default=backends.json.piglit_encoder)),
            schema)
