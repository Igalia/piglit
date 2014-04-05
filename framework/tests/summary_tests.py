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


""" Module providing tests for the summary module """

import json
import copy
import nose.tools as nt
import framework.summary as summary
import framework.tests.utils as utils


def test_initialize_summary():
    """ Test that Summary initializes """
    with utils.resultfile() as tfile:
        test = summary.Summary([tfile.name])
        assert test


def test_summary_add_to_set():
    """ Generate tests for testing Summary.test sets """
    old = copy.deepcopy(utils.JSON_DATA)
    new = copy.deepcopy(utils.JSON_DATA)

    for ostat, nstat, set_ in [('fail', 'pass', 'fixes'),
                               ('pass', 'warn', 'regressions'),
                               ('dmesg-fail', 'warn', 'changes'),
                               ('dmesg-warn', 'crash', 'problems'),
                               ('notrun', 'pass', 'enabled'),
                               ('skip', 'dmesg-warn', 'enabled'),
                               ('fail', 'notrun', 'disabled'),
                               ('crash', 'skip', 'disabled'),
                               ('skip', 'skip', 'skipped'),
                               ('notrun', 'fail', 'problems'),
                               ('fail', 'notrun', 'problems'),
                               ('pass', 'fail', 'problems')]:
        check_sets.description = "{0} -> {1} should be added to {2}".format(
                ostat, nstat, set_)

        yield check_sets, old, ostat, new, nstat, set_


def check_sets(old, ostat, new, nstat, set_):
    """ Check that the statuses are added to the correct set """
    old['tests']['sometest']['result'] = ostat
    new['tests']['sometest']['result'] = nstat

    with utils.with_tempfile(json.dumps(old)) as ofile:
        with utils.with_tempfile(json.dumps(new)) as nfile:
            summ = summary.Summary([ofile, nfile])

            print summ.tests
            nt.assert_equal(1, len(summ.tests[set_]),
                            msg="{0} was not appended".format(set_))
