# coding=utf-8
#
# Copyright (c) 2015-2016, 2019 Intel Corporation
# Copyright © 2020 Valve Corporation.
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


"""Tests for replayer's options module."""

import pytest

from urllib.parse import urlparse

from framework.replay import options

# pylint: disable=protected-access
# pylint: disable=invalid-name
# pylint: disable=no-self-use


def test_options_clear():
    """options.Options.clear(): resests options values to init state."""
    baseline = options._Options()

    test = options._Options()
    test.device_name = "dummy-device"
    test.keep_image = True
    test.clear()

    assert list(iter(baseline)) == list(iter(test))


@pytest.mark.parametrize("url, expected", [
    ("this://is.a.valid/url", urlparse("this://is.a.valid/url")),
    ("this://is.not.a[/valid/url", None),
])
def test_options_set_download_url(url, expected):
    """options.Options.set_download_url(): safely sets the parsed download url."""
    o = options._Options()

    o.set_download_url(url)

    assert o.download["url"] == expected
