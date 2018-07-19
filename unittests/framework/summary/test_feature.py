# encoding=utf-8
# Copyright © 2016 Intel Corporation

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

"""Tests for the feature summary module."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
try:
    import simplejson as json
except ImportError:
    import json
try:
    import mock
except ImportError:
    from unittest import mock

import pytest
import six

from framework import grouptools
from framework import results
from framework import profile
from framework.summary import feature

from .. import utils

DATA = {
    'spec@gl-1.0': {
        'include_tests': 'gl-1.0',
        'exclude_tests': '',
        'target_rate': 50,
    },
    'spec@gl-2.0': {
        'include_tests': 'gl-2.0',
        'exclude_tests': '',
        'target_rate': 50,
    },
}


def _maketest(res):
    """Helper utility to make a test with a result."""
    t = utils.Test(['foo'])
    t.result.result = res
    return t


PROFILE = profile.TestProfile()
PROFILE.test_list = profile.TestDict()
PROFILE.test_list['spec@gl-1.0@a'] = _maketest('pass')
PROFILE.test_list['spec@gl-1.0@b'] = _maketest('warn')
PROFILE.test_list['spec@gl-1.0@c'] = _maketest('pass')
PROFILE.test_list['spec@gl-1.0@d'] = _maketest('fail')
PROFILE.test_list['spec@gl-2.0@a'] = _maketest('fail')
PROFILE.test_list['spec@gl-2.0@b'] = _maketest('crash')
PROFILE.test_list['spec@gl-2.0@c'] = _maketest('pass')
PROFILE.test_list['spec@gl-2.0@d'] = _maketest('fail')


class TestFeatResult(object):
    """Tests for the FeatResult class."""

    @pytest.fixture(scope='session')
    def feature(self, tmpdir_factory):
        p = tmpdir_factory.mktemp('feature').join('p')
        # each write to p will replace the contents, so using json.dump which
        # makes a number of small writes will fail to produce anything useful.
        p.write(json.dumps(DATA))

        result = results.TestrunResult()
        for n, s in six.iteritems(PROFILE.test_list):
            result.tests[n] = s.result
        result.options['profile'] = [None]
        result.name = 'foo'

        with mock.patch('framework.summary.feature.profile.load_test_profile',
                        mock.Mock(return_value=PROFILE)):
            return feature.FeatResults([result], six.text_type(p))

    def test_basic(self, feature):
        """The fixture works."""
        pass

    def test_features(self, feature):
        """The features set is populated."""
        assert feature.features == {'spec@gl-1.0', 'spec@gl-2.0'}

    def test_feat_fractions(self, feature):
        """feat_fraction is populated."""
        assert feature.feat_fractions == \
            {'foo': {'spec@gl-1.0': (2, 4), 'spec@gl-2.0': (1, 4)}}

    def test_feat_status(self, feature):
        """feat_status is populated."""
        assert feature.feat_status == \
            {'foo': {'spec@gl-1.0': 'pass', 'spec@gl-2.0': 'fail'}}
