# coding=utf-8
# Copyright (c) 2015-2016 Intel Corporation

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

"""Stores global piglit options.

This is as close to a true global function as python gets. The only deal here
is that while you can mutate

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os

import six

__all__ = ['OPTIONS']

# pylint: disable=too-few-public-methods


class _Options(object):  # pylint: disable=too-many-instance-attributes
    """Contains all options for a piglit run.

    This is used as a sort of global state object, kinda like piglit.conf. This
    big difference here is that this object is largely generated internally,
    and is controlled mostly through command line options rather than through
    the configuration file.

    Options are as follows:
    execute -- False for dry run
    valgrind -- True if valgrind is to be used
    env -- environment variables set for each test before run
    deqp_mustpass -- True to enable the use of the deqp mustpass list feature.
    """

    def __init__(self):
        self.execute = True
        self.valgrind = False
        self.sync = False
        self.deqp_mustpass = False
        self.process_isolation = True
        self.jobs = None
        self.force_glsl = False
        self.spirv = False

        # env is used to set some base environment variables that are not going
        # to change across runs, without sending them to os.environ which is
        # fickle and easy to break
        self.env = {
            'PIGLIT_SOURCE_DIR':
                os.environ.get(
                    'PIGLIT_SOURCE_DIR',
                    os.path.abspath(os.path.join(os.path.dirname(__file__),
                                                 '..')))
        }

    def clear(self):
        """Reinitialize all values to defaults."""
        self.__init__()

    def __iter__(self):
        for key, values in six.iteritems(self.__dict__):
            if not key.startswith('_'):
                yield key, values


OPTIONS = _Options()
