# encoding=utf-8
# Copyright © 2018 Intel Coproration

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import random

from framework import grouptools
from .shader import profile as _profile

__all__ = ['profile']

profile = _profile.copy()


# There are too many of theset, it's simply exhaustive to run them all.
class FilterVsIn(object):
    """Filter out 80% of the Vertex Attrib 64 vs_in tests."""

    def __init__(self):
        self.random = random.Random()
        self.random.seed(42)

    def __call__(self, name, _):
        if 'vs_in' in grouptools.split(name):
            # 20%
            return self.random.random() <= .2
        return True


profile.filters.append(FilterVsIn())

# These take too long
profile.filters.append(lambda n, _: '-explosion' not in n)
