"""A profile that runs only GLSLParserTest instances."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

from framework.test import GLSLParserTest
from tests.all import profile as _profile

__all__ = ['profile']

profile = _profile.copy()  # pylint: disable=invalid-name

profile.filters.append(lambda _, t: isinstance(t, GLSLParserTest))
