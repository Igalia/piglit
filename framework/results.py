# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# This permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR(S) BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

""" Module for results generation """

from __future__ import print_function, absolute_import
import os
import sys

try:
    import simplejson as json
except ImportError:
    import json

import framework.status as status
from framework.backends import (CURRENT_JSON_VERSION, piglit_encoder,
                                JSONBackend)

__all__ = [
    'TestrunResult',
    'TestResult',
    'load_results',
]


class TestResult(dict):
    def recursive_update(self, dictionary):
        """ Recursively update the TestResult

        The problem with using self.update() is this:
        >>> t = TestResult()
        >>> t.update({'subtest': {'test1': 'pass'}})
        >>> t.update({'subtest': {'test2': 'pass'}})
        >>> t['subtest']
        {'test2': 'pass'}

        This function is different, because it recursively updates self, it
        doesn't clobber existing entires in the same way
        >>> t = TestResult()
        >>> t.recursive_update({'subtest': {'test1': 'pass'}})
        >>> t.recursive_update({'subtest': {'test2': 'pass'}})
        >>> t['subtest']
        {'test1': 'pass', 'test2': 'pass'}

        Arguments:
        dictionary -- a dictionary instance to update the TestResult with

        """
        def update(d, u, check):
            for k, v in u.iteritems():
                if isinstance(v, dict):
                    d[k] = update(d.get(k, {}), v, True)
                else:
                    if check and k in d:
                        print("Warning: duplicate subtest: {} value: {} old value: {}".format(k, v, d[k]))
                    d[k] = v
            return d

        update(self, dictionary, False)

    @classmethod
    def load(cls, res):
        """Load an already generated result.

        This is used as an alternate constructor which converts an existing
        dictionary into a TestResult object. It converts a key 'result' into a
        status.Status object

        """
        result = cls(res)

        # Replace the result with a status object. 'result' is a required key
        # for results, so don't do any checking. This should fail if it doesn't
        # exist.
        result['result'] = status.status_lookup(result['result'])

        return result


class TestrunResult(object):
    def __init__(self):
        self.serialized_keys = ['options',
                                'name',
                                'tests',
                                'uname',
                                'wglinfo',
                                'glxinfo',
                                'lspci',
                                'results_version',
                                'time_elapsed']
        self.name = None
        self.uname = None
        self.options = None
        self.glxinfo = None
        self.lspci = None
        self.time_elapsed = None
        self.results_version = CURRENT_JSON_VERSION
        self.tests = {}

    def write(self, file_):
        """ Write only values of the serialized_keys out to file """
        with open(file_, 'w') as f:
            json.dump(dict((k, v) for k, v in self.__dict__.iteritems()
                           if k in self.serialized_keys),
                      f, default=piglit_encoder, indent=JSONBackend.INDENT)

    @classmethod
    def load(cls, results_file):
        """Create a TestrunResult from a completed file."""
        result = cls()
        result.results_version = 0
        result.__dict__.update(json.load(results_file))

        for key, value in result.tests.iteritems():
            result.tests[key] = TestResult.load(value)

        return result

    @classmethod
    def resume(cls, results_dir):
        """ Create a TestrunResult from an interupted run

        This class method creates and returns a TestrunResult from a partial
        result file. This does not load old streaminmg results by design, one
        should not resume a run with a different piglit, it leads to all kinds
        of problems.

        """
        # Pylint can't infer that the json being loaded is a dict
        # pylint: disable=maybe-no-member
        assert os.path.isdir(results_dir), \
            "TestrunResult.resume() requires a directory"

        # Load the metadata
        with open(os.path.join(results_dir, 'metadata.json'), 'r') as f:
            meta = json.load(f)
        assert meta['results_version'] == CURRENT_JSON_VERSION, \
            "Old results version, resume impossible"

        testrun = cls()
        testrun.name = meta['name']
        testrun.options = meta['options']
        testrun.uname = meta.get('uname')
        testrun.glxinfo = meta.get('glxinfo')
        testrun.lspci = meta.get('lspci')

        # Load all of the test names and added them to the test list
        for file_ in os.listdir(os.path.join(results_dir, 'tests')):
            with open(os.path.join(results_dir, 'tests', file_), 'r') as f:
                try:
                    test = json.load(f)
                except ValueError:
                    continue
            # XXX: There has to be a better way to get a single key: value out
            # of a dict even when the key name isn't known
            for key, value in test.iteritems():
                testrun.tests[key] = TestResult.load(value)

        return testrun


def load_results(filename):
    """ Loader function for TestrunResult class

    This function takes a single argument of a results file.

    It makes quite a few assumptions, first it assumes that it has been passed
    a folder, if that fails then it looks for a plain text json file called
    "main"

    """
    # This will load any file or file-like thing. That would include pipes and
    # file descriptors
    if not os.path.isdir(filename):
        filepath = filename
    elif os.path.exists(os.path.join(filename, 'metadata.json')):
        # If the test is still running we need to use the resume code, since
        # there will not be a results.json file.
        # We want to return here since the results are known current (there's
        # an assert in TestrunResult.load), and there is no filepath
        # to pass to update_results
        # XXX: This needs to be run before searching for a results.json file so
        #      that if the new run is overwriting an old one we load the
        #      partial and not the original. It might be better to just delete
        #      the contents of the folder if there is anything in it.
        # XXX: What happens if the tests folder gets deleted in the middle of
        #      this?
        return TestrunResult.resume(filename)
    else:
        # If there are both old and new results in a directory pick the new
        # ones first
        if os.path.exists(os.path.join(filename, 'results.json')):
            filepath = os.path.join(filename, 'results.json')
        # Version 0 results are called 'main'
        elif os.path.exists(os.path.join(filename, 'main')):
            filepath = os.path.join(filename, 'main')
        else:
            raise Exception("No results found")

    with open(filepath, 'r') as f:
        testrun = TestrunResult.load(f)

    return update_results(testrun, filepath)


def update_results(results, filepath):
    """ Update results to the lastest version

    This function is a wraper for other update_* functions, providing
    incremental updates from one version to another.

    Arguments:
    results -- a TestrunResults instance
    filepath -- the name of the file that the Testrunresults instance was
                created from

    """

    def loop_updates(results):
        """ Helper to select the proper update sequence """
        # Python lacks a switch statement, the workaround is to use a
        # dictionary
        updates = {
            0: _update_zero_to_one,
            1: _update_one_to_two,
        }

        while results.results_version < CURRENT_JSON_VERSION:
            results = updates[results.results_version](results)

        return results

    # If the results version is the current version there is no need to
    # update, just return the results
    if results.results_version == CURRENT_JSON_VERSION:
        return results

    results = loop_updates(results)

    # Move the old results, and write the current results
    filedir = os.path.dirname(filepath)
    try:
        os.rename(filepath, os.path.join(filedir, 'results.json.old'))
        results.write(os.path.join(filedir, 'results.json'))
    except OSError:
        print("WARNING: Could not write updated results {}".format(filepath),
              file=sys.stderr)

    return results


def _update_zero_to_one(results):
    """ Update version zero results to version 1 results

    Changes from version 0 to version 1

    - dmesg is sometimes stored as a list, sometimes stored as a string. In
      version 1 it is always stored as a string
    - in version 0 subtests are somtimes stored as duplicates, sometimes stored
      only with a single entry, in version 1 tests with subtests are only
      recorded once, always.
    - Version 0 can have an info entry, or returncode, out, and err entries,
      Version 1 will only have the latter
    - version 0 results are called 'main', while version 1 results are called
      'results.json' (This is not handled internally, it's either handled by
      update_results() which will write the file back to disk, or needs to be
      handled manually by the user)

    """
    updated_results = {}
    remove = set()

    for name, test in results.tests.iteritems():
        # fix dmesg errors if any
        if isinstance(test.get('dmesg'), list):
            test['dmesg'] = '\n'.join(test['dmesg'])

        # If a test as an info attribute, we want to remove it, if it doesn't
        # have a returncode, out, or attribute we'll want to get those out of
        # info first
        #
        # This expects that the order of info is rougly returncode, errors,
        # output, *extra it can handle having extra information in the middle,
        if (None in [test.get('out'), test.get('err'), test.get('returncode')]
                and test.get('info')):

            # This attempts to split everything before Errors: as a returncode,
            # and everything before Output: as Errors, and everything else as
            # output. This may result in extra info being put in out, this is
            # actually fine since out is only parsed by humans.
            returncode, split = test['info'].split('\n\nErrors:')
            err, out = split.split('\n\nOutput:')

            # returncode can be 0, and 0 is falsy, so ensure it is actually
            # None
            if test.get('returncode') is None:
                # In some cases the returncode might not be set (like the test
                # skipped), in that case it will be None, so set it
                # apropriately
                try:
                    test['returncode'] = int(
                        returncode[len('returncode: '):].strip())
                except ValueError:
                    test['returncode'] = None
            if not test.get('err'):
                test['err'] = err.strip()
            if not test.get('out'):
                test['out'] = out.strip()

        # Remove the unused info key
        if test.get('info'):
            del test['info']

        # If there is more than one subtest written in version 0 results that
        # entry will be a complete copy of the original entry with '/{name}'
        # appended. This loop looks for tests with subtests, removes the
        # duplicate entries, and creates a new entry in update_results for the
        # single full tests.
        #
        # this must be the last thing done in this loop, or there will be pain
        if test.get('subtest'):
            for sub in test['subtest'].iterkeys():
                # adding the leading / ensures that we get exactly what we
                # expect, since endswith does a character by chacter match, if
                # the subtest name is duplicated it wont match, and if there
                # are more trailing characters it will not match
                #
                # We expect duplicate names like this:
                #  "group1/groupA/test1/subtest 1": <thing>,
                #  "group1/groupA/test1/subtest 2": <thing>,
                #  "group1/groupA/test1/subtest 3": <thing>,
                #  "group1/groupA/test1/subtest 4": <thing>,
                #  "group1/groupA/test1/subtest 5": <thing>,
                #  "group1/groupA/test1/subtest 6": <thing>,
                #  "group1/groupA/test1/subtest 7": <thing>,
                # but what we want is groupg1/groupA/test1 and none of the
                # subtest as keys in the dictionary at all
                if name.endswith('/{0}'.format(sub)):
                    testname = name[:-(len(sub) + 1)]  # remove leading /
                    assert testname[-1] != '/'

                    remove.add(name)
                    break
            else:
                # This handles two cases, first that the results have only
                # single entries for each test, regardless of subtests (new
                # style), or that the test onhly as a single subtest and thus
                # was recorded correctly
                testname = name

            if testname not in updated_results:
                updated_results[testname] = test

    for name in remove:
        del results.tests[name]
    results.tests.update(updated_results)

    # set the results version
    results.results_version = 1

    return results


def _update_one_to_two(results):
    """Update version 1 results to version 2.

    Version two results are actually identical to version one results, however,
    there was an error in version 1 at the end causing metadata in the options
    dictionary to be incorrect. Version 2 corrects that.

    Namely uname, glxinfo, wglinfo, and lspci were put in the options['env']
    instead of in the root.

    """
    if 'env' in results.options:
        env = results.options['env']
        if env.get('glxinfo'):
            results.glxinfo = env['glxinfo']
        if env.get('lspci'):
            results.lspci = env['lspci']
        if env.get('uname'):
            results.uname = env['uname']
        if env.get('wglinfo'):
            results.wglinfo = env['wglinfo']
        del results.options['env']

    results.results_version = 2

    return results
