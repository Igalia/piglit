# coding=utf-8
# Copyright (c) 2016 Intel Corporation

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

"""A small library that contains libraries equivalent to six

This function is pending upstreaming in six.

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

import six

def python_2_bool_compatible(class_):
    """A decorator to fix __bool__/__nonzero__ name changes."""
    if six.PY2:
        try:
            class_.__nonzero__ = class_.__bool__
        except AttributeError:
            raise ValueError(
                "@python_2_bool_compatible cannot be applied to {} because "
                "it doesn't define __bool__().".format(class_.__name__))
    return class_


# Some version of six don't have this function
try:
    from six import python_2_unicode_compatible
except ImportError:
    def python_2_unicode_compatible(class_):
        """A decorator to fix __str/__bytes__/__unicode__ name changes."""
        if six.PY2:
            failed = False
            try:
                class_.__unicode__ = class_.__str__
            except AttributeError:
                failed = True

            try:
                class_.__str__ = class_.__bytes__
            except AttributeError:
                if failed:
                    raise ValueError(
                        "@python_2_unicode_compatible cannot be applied to {} "
                        "because it doesn't define __str__() "
                        "or __bytes__().".format(class_.__name__))
        return class_


try:
    from six import viewvalues
except ImportError:
    if six.PY2:
        viewvalues = lambda d: d.viewvalues()
    elif six.PY3:
        viewvalues = lambda d: d.values()
