# -*- coding: utf-8 -*-

# quick.tests minus compiler tests.

from tests.quick import profile
from framework.test import GLSLParserTest

__all__ = ['profile']

# Remove all glsl_parser_tests, as they are compiler test
profile.filter_tests(lambda p, t: not isinstance(t, GLSLParserTest))

# Remove asmparasertests as well, since they're parser tests.
del profile.tests['asmparsertest']
