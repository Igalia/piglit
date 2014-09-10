# -*- coding: utf-8 -*-

# quick.tests minus compiler tests.

from tests.quick import profile
from framework.test import GLSLParserTest

__all__ = ['profile']

# Remove all glsl_parser_tests, as they are compiler test
profile.filter_tests(lambda p, t: not isinstance(t, GLSLParserTest))

# Drop ARB_vertex_program/ARB_fragment_program compiler tests.
del profile.tests['spec']['ARB_vertex_program']
del profile.tests['spec']['ARB_fragment_program']
del profile.tests['asmparsertest']
