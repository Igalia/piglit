# coding=utf-8
# Copyright 2014-2016, 2019 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

"""Piglit integrations for Crucible Test Suite.

upstream: https://gitlab.freedesktop.org/mesa/crucible/

"""

import os
import subprocess
import tempfile

try:
    from lxml import etree
except ImportError:
    import xml.etree.ElementTree as etree

from framework import grouptools, backends, core, exceptions
from framework import status
from framework.profile import TestProfile, Test

__all__ = ['profile']

crucible_bin = core.get_option('PIGLIT_CRUCIBLE_BIN',
                               ('crucible', 'bin'),
                               required=True)


class CrucibleTest(Test):
    """Test representation for Crucible"""
    def __init__(self, case_name):
        self.__out_xml = tempfile.NamedTemporaryFile(delete=True).name
        command = [crucible_bin, 'run',
                   '--junit-xml={}'.format(self.__out_xml), case_name]
        self._case = case_name
        super(CrucibleTest, self).__init__(command)

    def interpret_result(self):
        try:
            test = backends.junit.REGISTRY.load(self.__out_xml, 'none')
            result = test.get_result(next(test.tests.keys()))
            self.result.result = result.name
            super(CrucibleTest, self).interpret_result()
        except etree.ParseError:
            # This error is what ElementTree will generate, and is the parent
            # of what lxml will generate.
            self.result.result = status.CRASH
        finally:
            os.remove(self.__out_xml)

def gen_caselist_txt(bin_):
    with open('crucible.txt', 'w') as d:
        subprocess.check_call(
            [bin_, 'ls-tests'],
            stdout=d, stderr=d)
    assert os.path.exists('crucible.txt')
    return 'crucible.txt'

def _populate_profile():
    profile = TestProfile()
    case_file = gen_caselist_txt(crucible_bin)
    with open(case_file, 'r') as caselist_file:
        for i, line in enumerate(caselist_file):
            case = line.rstrip()
            piglit_name = case.replace('.', grouptools.SEPARATOR)
            profile.test_list[piglit_name] = CrucibleTest(case)
    os.remove('crucible.txt')
    return profile

profile = _populate_profile()
