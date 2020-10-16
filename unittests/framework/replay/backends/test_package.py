# coding=utf-8
#
# Copyright (c) 2014, 2016 Intel Corporation
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

""" Tests for replayer's backend package """

import pytest

from framework.replay import backends

# pylint: disable=no-self-use



# Helpers


class TestBackend():
    """A dummy test backend"""

    def __init__(self, trace_path, output_dir=None, calls=[]):
        self._trace_path = trace_path

    def dump(self):
        return [self._trace_path]

# Prevent pytest from trying to collect TestBackend as tests:
TestBackend.__test__ = False

@pytest.yield_fixture
def mock_backend(mocker, backend):
    """Add an extra backend for testing."""
    mocker.patch.dict(
        backends.DUMPBACKENDS,
        {'test_backend': backends.register.Registry(
            extensions=['.test_backend'],
            backend=backend,
        )})
    yield


# Tests


class TestDump(object):
    """Tests for the dump function."""

    @pytest.mark.parametrize("backend", [
        (TestBackend),
    ])
    def test_basic(self, mock_backend):  # pylint: disable=unused-argument
        """backends.dump: works as expected."""
        p = 'foo.test_backend'
        test = backends.dump(p)
        assert [p] == test

    @pytest.mark.raises(exception=backends.DumpBackendError)
    def test_unknown(self):
        """backends.dump(): An error is raised if no backend is registered for an
         extension.
        """
        backends.dump('foo.test_extension')

    @pytest.mark.raises(exception=backends.DumpBackendNotImplementedError)
    @pytest.mark.parametrize("backend", [
        (None),
    ])
    def test_notimplemented(self, mock_backend):  # pylint: disable=unused-argument
        """backends.dump(): An error is raised if a dumper isn't properly
        implmented.
        """
        backends.dump('foo.test_backend')
