"""A profile that runs only ShaderTest instances."""

from framework.test import ShaderTest
from tests.all import profile

__all__ = ['profile']

profile.filter_tests(lambda _, t: isinstance(t, ShaderTest))
