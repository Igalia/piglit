"""A profile that runs only ShaderTest instances."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import collections
import os

import six

from framework.options import OPTIONS
from framework import grouptools
from framework.profile import TestProfile
from framework.test.shader_test import ShaderTest, MultiShaderTest
from .py_modules.constants import GENERATED_TESTS_DIR, TESTS_DIR

__all__ = ['profile']

profile = TestProfile()

shader_tests = collections.defaultdict(list)

# Find and add all shader tests.
basepath = os.path.normpath(os.path.join(TESTS_DIR, '..'))
for basedir in [TESTS_DIR, GENERATED_TESTS_DIR]:
    for dirpath, _, filenames in os.walk(basedir):
        groupname = grouptools.from_path(os.path.relpath(dirpath, basedir))
        for filename in filenames:
            testname, ext = os.path.splitext(filename)
            if ext == '.shader_test':
                dirname = os.path.relpath(dirpath, basepath)
                if OPTIONS.process_isolation:
                    test = ShaderTest.new(os.path.join(dirname, filename))
                else:
                    shader_tests[groupname].append(os.path.join(dirname, filename))
                    continue
            else:
                continue

            group = grouptools.join(groupname, testname)
            assert group not in profile.test_list, group

            profile.test_list[group] = test

# Because we need to handle duplicate group names in TESTS and GENERATED_TESTS
# this dictionary is constructed, then added to the actual test dictionary.
for group, files in six.iteritems(shader_tests):
    assert group not in profile.test_list, 'duplicate group: {}'.format(group)
    # If there is only one file in the directory use a normal shader_test.
    # Otherwise use a MultiShaderTest
    if len(files) == 1:
        group = grouptools.join(
            group, os.path.basename(os.path.splitext(files[0])[0]))
        profile.test_list[group] = ShaderTest.new(files[0])
    else:
        profile.test_list[group] = MultiShaderTest.new(files)
