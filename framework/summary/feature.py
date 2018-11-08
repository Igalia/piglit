# coding=utf-8
# Copyright (c) 2016-2016 Intel Corporation

# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# This permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR(S) BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

try:
    import simplejson as json
except ImportError:
    import json

from framework import profile, status


class FeatResults(object):  # pylint: disable=too-few-public-methods
    """Container object for results.

    Has the results, feature profiles and feature computed results.

    """
    def __init__(self, results, json_file):

        with open(json_file) as data:
            feature_data = json.load(data)

        self.feat_fractions = {}
        self.feat_status = {}
        self.features = set()
        self.results = results

        profiles = {}

        # we expect all the result sets to be for the same profile
        profile_orig = profile.load_test_profile(results[0].options['profile'][0])

        for feature in feature_data:
            self.features.add(feature)

            profiles[feature] = profile_orig.copy()

            incl_str = feature_data[feature]["include_tests"]
            excl_str = feature_data[feature]["exclude_tests"]

            profiles[feature].filters.append(
                profile.RegexFilter(
                    [incl_str] if incl_str and not incl_str.isspace() else []))
            profiles[feature].filters.append(
                profile.RegexFilter(
                    [excl_str] if excl_str and not excl_str.isspace() else [],
                    inverse=True))

        for results in self.results:
            self.feat_fractions[results.name] = {}
            self.feat_status[results.name] = {}

            for feature in feature_data:
                result_set = set(results.tests)
                profile_set = set(a for a, _ in profiles[feature].itertests())

                common_set = profile_set & result_set
                passed_list = [x for x in common_set if results.tests[x].result == status.PASS]

                total = len(common_set)
                passed = len(passed_list)

                self.feat_fractions[results.name][feature] = (passed, total)
                if total == 0:
                    self.feat_status[results.name][feature] = status.NOTRUN
                else:
                    if 100 * passed // total >= feature_data[feature]["target_rate"]:
                        self.feat_status[results.name][feature] = status.PASS
                    else:
                        self.feat_status[results.name][feature] = status.FAIL
