# coding=utf-8
"""A profile that runs only GLSLParserTest instances."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os

from framework import grouptools
from framework.profile import TestProfile
from framework.test.glsl_parser_test import GLSLParserTest, GLSLParserNoConfigError
from framework.test.piglit_test import ASMParserTest, ROOT_DIR
from .py_modules.constants import GENERATED_TESTS_DIR, TESTS_DIR

__all__ = ['profile']

profile = TestProfile()

# Find and add all shader tests.
basepath = os.path.normpath(os.path.join(TESTS_DIR, '..'))
gen_basepath = os.path.relpath(os.path.join(GENERATED_TESTS_DIR, '..'), basepath)

for basedir in [TESTS_DIR, GENERATED_TESTS_DIR]:
    isgenerated = basedir == GENERATED_TESTS_DIR
    for dirpath, _, filenames in os.walk(basedir):
        groupname = grouptools.from_path(os.path.relpath(dirpath, basedir))
        for filename in filenames:
            testname, ext = os.path.splitext(filename)
            if ext in ['.vert', '.tesc', '.tese', '.geom', '.frag', '.comp']:
                dirname = os.path.relpath(dirpath, basepath)
                filepath = os.path.join(dirname, filename)
                if isgenerated:
                    installpath = os.path.relpath(filepath, gen_basepath)
                else:
                    installpath = None

                try:
                    test = GLSLParserTest.new(filepath, installpath)
                except GLSLParserNoConfigError:
                    # In the event that there is no config assume that it is a
                    # legacy test, and continue
                    continue

                # For glslparser tests you can have multiple tests with the
                # same name, but a different stage, so keep the extension.
                testname = filename
            else:
                continue

            group = grouptools.join(groupname, testname)
            assert group not in profile.test_list, group

            profile.test_list[group] = test

# Collect and add all asmparsertests
for basedir in [TESTS_DIR, GENERATED_TESTS_DIR]:
    _basedir = os.path.join(basedir, 'asmparsertest', 'shaders')
    for dirpath, _, filenames in os.walk(_basedir):
        base_group = grouptools.from_path(os.path.join(
            'asmparsertest', os.path.relpath(dirpath, _basedir)))
        type_ = os.path.basename(dirpath)

        dirname = os.path.relpath(dirpath, os.path.join(basedir, '..'))
        for filename in filenames:
            if not os.path.splitext(filename)[1] == '.txt':
                continue

            group = grouptools.join(base_group, filename)
            profile.test_list[group] = ASMParserTest(
                type_, os.path.join(dirname, filename))
