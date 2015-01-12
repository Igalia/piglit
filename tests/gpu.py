# -*- coding: utf-8 -*-

# quick.tests minus compiler tests.

from tests.quick import profile
from framework.test import GLSLParserTest

__all__ = ['profile']

# Remove all parser tests, as they are compiler test
profile.filter_tests(lambda p, t: not isinstance(t, GLSLParserTest))
profile.filter_tests(lambda n, _: not n.startswith('asmparsertest'))
