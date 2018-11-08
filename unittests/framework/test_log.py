# coding=utf-8
# Copyright (c) 2014, 2016 Intel Corporation

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

"""Tests for log.py module."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import collections
import sys
import threading

import pytest
import six

import framework.log as log

# pylint: disable=no-self-use,protected-access


@pytest.fixture
def log_state():
    """Create a unique state instance per test."""
    return {'total': 1, 'complete': 0, 'lastlength': 0, 'running': [],
            'summary': collections.defaultdict(lambda: 0)}


class TestLogFactory(object):
    """Tests for the LogFactory class."""

    class TestGet(object):
        """Tests for the LogFactory.get method."""

        def test_returns_log(self):
            """Returns a BaseLog derived instance."""
            logger = log.LogManager('quiet', 100)
            log_inst = logger.get()
            assert isinstance(log_inst, log.BaseLog)

    def test_log_state_update(self):
        """log.BaseLog.log updates shared state managed by LogManager"""
        logger = log.LogManager('quiet', 100)
        log_inst = logger.get()
        log_inst.start(None)
        log_inst.log('pass')

        assert logger._state['total'] == 100
        assert logger._state['summary'] == {'pass': 1}
        assert logger._state['complete'] == 1


class TestQuietLog(object):
    """Test QuietLog class."""

    class TestOutput(object):
        """Test the output of the various methods."""

        @pytest.fixture(autouse=True, scope='function')
        def mock_stdout(self, mocker):
            mocker.patch.object(sys, 'stdout', six.StringIO())

        def test_log(self, log_state):  # pylint: disable=redefined-outer-name
            """Test the output of the log method."""
            quiet = log.QuietLog(log_state, threading.Lock())
            quiet.start(None)
            quiet.log('pass')
            sys.stdout.seek(0)

            actual = sys.stdout.read()
            assert actual == b'[1/1] pass: 1 -\n'

        def test_summary(self, log_state):  # pylint: disable=redefined-outer-name
            """Test the output of the summary method."""
            quiet = log.QuietLog(log_state, threading.Lock())
            # Call log to set the total correctly, then truncate and remove the
            # values, so the we can test
            quiet.start(None)
            quiet.log('pass')
            sys.stdout.seek(0)
            sys.stdout.truncate()

            quiet.summary()
            sys.stdout.seek(0)

            # Because of the 'lastlength' mechanims there will likely be
            # trainling whitespace after the the output, it's not useful to
            # test that here, so just strip it.
            assert sys.stdout.read().rstrip() == b'[1/1] pass: 1'

        def test_start(self, log_state):  # pylint: disable=redefined-outer-name
            """Test that the start method doesn't have output."""
            quiet = log.QuietLog(log_state, threading.Lock())
            quiet.start(None)
            quiet.start('foo')
            sys.stdout.seek(0)

            actual = sys.stdout.read()
            assert actual == b''


class TestVerboseLog(object):
    """Tests for the VerboseLog class."""

    class TestOutput(object):
        """Test the output of the various methods."""

        @pytest.fixture(autouse=True, scope='function')
        def mock_stdout(self, mocker):
            mocker.patch.object(sys, 'stdout', six.StringIO())

        def test_log(self, log_state):  # pylint: disable=redefined-outer-name
            """Test the output of the log method."""
            l = log.VerboseLog(log_state, threading.Lock())
            l.start('foo')
            sys.stdout.seek(0)
            sys.stdout.truncate()

            l.log('pass')
            sys.stdout.seek(0)

            actual = sys.stdout.read()
            assert actual == b'pass: foo\n\n[1/1] pass: 1 /\n'

        def test_summary(self, log_state):  # pylint: disable=redefined-outer-name
            """Test the output of the summary method."""
            l = log.VerboseLog(log_state, threading.Lock())
            l.start('foo')
            l.log('pass')
            sys.stdout.seek(0)
            sys.stdout.truncate()

            l.summary()
            sys.stdout.seek(0)

            assert sys.stdout.read().rstrip() == b'[1/1] pass: 1'

        def test_start(self, log_state):  # pylint: disable=redefined-outer-name
            """Test that the start method doesn't have output."""
            l = log.VerboseLog(log_state, threading.Lock())
            l.start('foo')
            sys.stdout.seek(0)

            assert sys.stdout.read().rstrip() == b'running: foo\n\n[0/1]  \\'


class TestDummyLog(object):
    """Tests for the DummyLog class."""

    class TestOutput(object):
        """Test the output of the various methods."""

        @pytest.fixture(autouse=True, scope='function')
        def mock_stdout(self, mocker):
            mocker.patch.object(sys, 'stdout', six.StringIO())

        def test_log(self, log_state):  # pylint: disable=redefined-outer-name
            """Test the output of the log method."""
            quiet = log.DummyLog(log_state, threading.Lock())
            quiet.start(None)
            quiet.log('pass')
            sys.stdout.seek(0)

            actual = sys.stdout.read()
            assert actual == b''

        def test_summary(self, log_state):  # pylint: disable=redefined-outer-name
            """Test the output of the summary method."""
            quiet = log.DummyLog(log_state, threading.Lock())
            quiet.summary()
            sys.stdout.seek(0)

            actual = sys.stdout.read()
            assert actual == b''

        def test_start(self, log_state):  # pylint: disable=redefined-outer-name
            """Test that the start method doesn't have output."""
            quiet = log.DummyLog(log_state, threading.Lock())
            quiet.start('foo')
            sys.stdout.seek(0)

            actual = sys.stdout.read()
            assert actual == b''
