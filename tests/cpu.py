"""Profile that removes all GPU based tests.

This profile is the inverse of gpu.py.

It runs GLSLParserTests, asmparsertests, and ARB_vertex_program and
ARB_fragment_program tests only.

Using driver specific overrides these can be forced to run on arbitrary
hardware.

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
from tests.quick import profile as _profile
from framework.test import GLSLParserTest

__all__ = ['profile']

profile = _profile.copy()  # pylint: disable=invalid-name


def filter_gpu(name, test):
    """Remove all tests that are run on the GPU."""
    if isinstance(test, GLSLParserTest) or name.startswith('asmparsertest'):
        return True
    return False


profile.filter_tests(filter_gpu)
