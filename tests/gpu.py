# -*- coding: utf-8 -*-

# quick.tests minus compiler tests.

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

from tests.quick import profile as _profile
from framework.test import GLSLParserTest

__all__ = ['profile']

profile = _profile.copy()  # pylint: disable=invalid-name

# Remove all parser tests, as they are compiler test
profile.filter_tests(lambda p, t: not isinstance(t, GLSLParserTest))
profile.filter_tests(lambda n, _: not n.startswith('asmparsertest'))
