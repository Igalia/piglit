"""Profile that removes all GPU based tests.

This profile is the inverse of gpu.py.

It runs GLSLParserTests, asmparsertests, and ARB_vertex_program and
ARB_fragment_program tests only.

Using driver specific overrides these can be forced to run on arbitrary
hardware.

"""

from tests.quick import profile
from framework.test import GLSLParserTest

__all__ = ['profile']


def filter_gpu(name, test):
    """Remove all tests that are run on the GPU."""
    if isinstance(test, GLSLParserTest):
        return True

    if name.startswith('spec/ARB_vertex_program'):
        return True
    if name.startswith('spec/ARB_fragment_program'):
        return True
    if name.startswith('asmparsertest'):
        return True

    return False


profile.filter_tests(filter_gpu)
