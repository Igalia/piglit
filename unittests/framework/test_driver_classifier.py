# coding=utf-8
# Copyright (c) 2016 Broadcom
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

"""Tests for the driver_classifier module."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

try:
    import mock
except ImportError:
    from unittest import mock

import pytest
import six

from framework import driver_classifier


class DriverClassifierTester(driver_classifier.DriverClassifier):
    """Test class for the driver classifier, taking in a fixed

    renderer string instead of calling glxinfo.
    """

    def __init__(self, renderer):
        self.override_renderer = renderer
        super(DriverClassifierTester, self).__init__()


    def collect_driver_info(self):
        self.renderer = self.override_renderer


class TestDriverClassifier(object):
    """Tests for the DriverClassifier class."""

    @pytest.mark.parametrize(
        "renderer, categories",
        [
            ('Mesa DRI Intel(R) Haswell Mobile', ['i965-hsw', 'i965', 'mesa']),
            ('Mesa DRI Intel(R) HD Graphics 530 (Skylake GT2)', ['i965-skl', 'i965', 'mesa']),
            # Old VC4 string
            ('Gallium 0.4 on VC4', ['vc4', 'mesa']),
            # Modern VC4 string
            ('Gallium 0.4 on VC4 V3D 2.1', ['vc4-2.1', 'vc4', 'mesa']),
        ],
        ids=six.text_type)
    def test_renderer_categorization(self, renderer, categories):
        """Test that when given a certain renderer string, the correct

        categories list comes back.
        """
        assert DriverClassifierTester(renderer).categories == categories

    def test_collect_glxinfo(self):
        """Should set self.renderer."""
        test = driver_classifier.DriverClassifier()
        with mock.patch('framework.driver_classifier.subprocess.check_output',
                        mock.Mock(return_value=b'some data\nand some more\n'
                                               b'OpenGL renderer string: '
                                               b'sentinal\nand some other '
                                               b'stuff')):
            test.collect_glxinfo()
        assert isinstance(test.renderer, six.text_type)
        assert test.renderer == 'sentinal'
