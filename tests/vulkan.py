#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""All Vulkan tests that come with piglit, using default settings."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

import os

from framework.profile import TestProfile
from framework import grouptools
from framework.test.piglit_test import VkRunnerTest
from .py_modules.constants import TESTS_DIR, GENERATED_TESTS_DIR

__all__ = ['profile']

profile = TestProfile()

# Find and add all shader tests.
for basedir in [TESTS_DIR, GENERATED_TESTS_DIR]:
    _basedir = os.path.join(basedir, 'vulkan')
    for dirpath, _, filenames in os.walk(_basedir):
        groupname = grouptools.from_path(os.path.relpath(dirpath, _basedir))
        groupname = grouptools.join('vulkan', groupname)
        dirname = os.path.relpath(dirpath, os.path.join(basedir, '..'))
        for filename in filenames:
            testname, ext = os.path.splitext(filename)
            if ext != '.vk_shader_test':
                continue
            test = VkRunnerTest(os.path.join(dirname, filename))
            group = grouptools.join(groupname, testname)
            assert group not in profile.test_list, group

            profile.test_list[group] = test
