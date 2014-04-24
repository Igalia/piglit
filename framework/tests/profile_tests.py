# Copyright (c) 2014 Intel Corporation

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

""" Provides test for the framework.profile modules """

import nose.tools as nt
import framework.profile as profile


def test_initialize_testprofile():
    """ TestProfile initializes """
    profile.TestProfile()


@nt.raises(SystemExit)
def test_load_test_profile_no_profile():
    """ Loading a module with no profile name exits

    Beacuse loadTestProfile uses test.{} to load a module we need a module in
    tests that doesn't have a profile attribute. The only module that currently
    meets that requirement is __init__.py

    """
    profile.loadTestProfile('__init__')


def test_load_test_profile_returns():
    """ loadTestProfile returns a TestProfile instance """
    profile_ = profile.loadTestProfile('sanity')
    assert isinstance(profile_, profile.TestProfile)
