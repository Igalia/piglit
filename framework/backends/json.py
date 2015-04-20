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

""" Module providing json backend for piglit """

from __future__ import print_function, absolute_import
import os
import sys
import shutil
import posixpath

try:
    import simplejson as json
except ImportError:
    import json

from framework import status, results, exceptions
from .abstract import FileBackend
from .register import Registry

__all__ = [
    'REGISTRY',
    'JSONBackend',
]

# The current version of the JSON results
CURRENT_JSON_VERSION = 5

# The level to indent a final file
INDENT = 4


def piglit_encoder(obj):
    """ Encoder for piglit that can transform additional classes into json

    Adds support for status.Status objects and for set() instances

    """
    if isinstance(obj, status.Status):
        return str(obj)
    elif isinstance(obj, set):
        return list(obj)
    return obj


def piglit_decoder(obj):
    """Json decoder for piglit that can load TestResult objects."""
    if isinstance(obj, dict) and 'result' in obj:
        return results.TestResult.load(obj)
    return obj


class JSONBackend(FileBackend):
    """ Piglit's native JSON backend

    This writes out to piglit's native json backend. This class uses the python
    json module or the simplejson.

    This class is atomic, writes either completely fail or completley succeed.
    To achieve this it writes individual files for each test and for the
    metadata, and composes them at the end into a single file and removes the
    intermediate files. When it tries to compose these files if it cannot read
    a file it just ignores it, making the result atomic.

    """
    _file_extension = 'json'

    def initialize(self, metadata):
        """ Write boilerplate json code

        This writes all of the json except the actual tests.

        Arguments:
        metadata -- a dictionary of values to be written

        """
        # If metadata is None then this is a loaded result and there is no need
        # to initialize
        metadata['results_version'] = CURRENT_JSON_VERSION

        with open(os.path.join(self._dest, 'metadata.json'), 'w') as f:
            json.dump(metadata, f, default=piglit_encoder)

        # make the directory for the tests
        try:
            os.mkdir(os.path.join(self._dest, 'tests'))
        except OSError:
            pass

    def finalize(self, metadata=None):
        """ End json serialization and cleanup

        This method is called after all of tests are written, it closes any
        containers that are still open and closes the file

        """
        # Create a dictionary that is full of data to be written to a single
        # file
        data = {}

        # Load the metadata and put it into a dictionary
        with open(os.path.join(self._dest, 'metadata.json'), 'r') as f:
            data.update(json.load(f))

        # If there is more metadata add it the dictionary
        if metadata:
            data.update(metadata)

        # Add the tests to the dictionary
        data['tests'] = {}

        test_dir = os.path.join(self._dest, 'tests')
        for test in os.listdir(test_dir):
            test = os.path.join(test_dir, test)
            if os.path.isfile(test):
                # Try to open the json snippets. If we fail to open a test then
                # throw the whole thing out. This gives us atomic writes, the
                # writing worked and is valid or it didn't work.
                try:
                    with open(test, 'r') as f:
                        data['tests'].update(json.load(f))
                except ValueError:
                    pass
        assert data['tests']

        # write out the combined file.
        with open(os.path.join(self._dest, 'results.json'), 'w') as f:
            json.dump(data, f, default=piglit_encoder,
                      indent=INDENT)

        # Delete the temporary files
        os.unlink(os.path.join(self._dest, 'metadata.json'))
        shutil.rmtree(os.path.join(self._dest, 'tests'))

    @staticmethod
    def _write(f, name, data):
        json.dump({name: data}, f, default=piglit_encoder)


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
        return _resume(filename)
    else:
        # If there are both old and new results in a directory pick the new
        # ones first
        if os.path.exists(os.path.join(filename, 'results.json')):
            filepath = os.path.join(filename, 'results.json')
        # Version 0 results are called 'main'
        elif os.path.exists(os.path.join(filename, 'main')):
            filepath = os.path.join(filename, 'main')
        else:
            raise exceptions.PiglitFatalError(
                'No results found in "{}"'.format(filename))

    with open(filepath, 'r') as f:
        testrun = _load(f)

    return _update_results(testrun, filepath)


def set_meta(results):
    """Set json specific metadata on a TestrunResult."""
    results.results_version = CURRENT_JSON_VERSION


def _load(results_file):
    """Load a json results instance and return a TestrunResult.

    This function converts an existing, fully completed json run.
    
    """
    result = results.TestrunResult()
    result.results_version = 0  # This should get overwritten
    result.__dict__.update(json.load(results_file, object_hook=piglit_decoder))

    return result


def _resume(results_dir):
    """Loads a partially completed json results directory."""
    # Pylint can't infer that the json being loaded is a dict
    # pylint: disable=maybe-no-member
    assert os.path.isdir(results_dir), \
        "TestrunResult.resume() requires a directory"

    # Load the metadata
    with open(os.path.join(results_dir, 'metadata.json'), 'r') as f:
        meta = json.load(f)
    assert meta['results_version'] == CURRENT_JSON_VERSION, \
        "Old results version, resume impossible"

    testrun = results.TestrunResult()
    testrun.name = meta['name']
    testrun.options = meta['options']
    testrun.uname = meta.get('uname')
    testrun.glxinfo = meta.get('glxinfo')
    testrun.lspci = meta.get('lspci')

    # Load all of the test names and added them to the test list
    for file_ in os.listdir(os.path.join(results_dir, 'tests')):
        with open(os.path.join(results_dir, 'tests', file_), 'r') as f:
            try:
                testrun.tests.update(json.load(f, object_hook=piglit_decoder))
            except ValueError:
                continue

    return testrun


def _update_results(results, filepath):
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
            2: _update_two_to_three,
            3: _update_three_to_four,
            4: _update_four_to_five,
        }

        while results.results_version < CURRENT_JSON_VERSION:
            results = updates[results.results_version](results)

        return results

    # If there is no version, then set it to 0, this will trigger a full
    # update.
    if not hasattr(results, 'results_version'):
        results.results_version = 0

    # If the results version is the current version there is no need to
    # update, just return the results
    if results.results_version == CURRENT_JSON_VERSION:
        return results

    results = loop_updates(results)

    # Move the old results, and write the current results
    try:
        os.rename(filepath, filepath + '.old')
        _write(results, filepath)
    except OSError:
        print("WARNING: Could not write updated results {}".format(filepath),
              file=sys.stderr)

    return results


def _write(results, file_):
    """WRite the values of the results out to a file."""
    with open(file_, 'w') as f:
        json.dump({k:v for k, v in results.__dict__.iteritems()},
                  f,
                  default=piglit_encoder,
                  indent=INDENT)


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


def _update_two_to_three(results):
    """Lower key names."""
    for key, value in results.tests.items():
        lowered = key.lower()
        if not key == lowered:
            results.tests[lowered] = value
            del results.tests[key]

    results.results_version = 3

    return results


def _update_three_to_four(results):
    """Update results v3 to v4.

    This update requires renaming a few tests. The complete lists can be found
    in framework/data/results_v3_to_v4.json, a json file containing a list of
    lists (They would be tuples if json has tuples), the first element being
    the original name, and the second being a new name to update to

    """
    mapped_updates = [
        ("spec/arb_texture_rg/fs-shadow2d-red-01",
         "spec/arb_texture_rg/execution/fs-shadow2d-red-01"),
        ("spec/arb_texture_rg/fs-shadow2d-red-02",
         "spec/arb_texture_rg/execution/fs-shadow2d-red-02"),
        ("spec/arb_texture_rg/fs-shadow2d-red-03",
         "spec/arb_texture_rg/execution/fs-shadow2d-red-03"),
        ("spec/arb_draw_instanced/draw-non-instanced",
         "spec/arb_draw_instanced/execution/draw-non-instanced"),
        ("spec/arb_draw_instanced/instance-array-dereference",
         "spec/arb_draw_instanced/execution/instance-array-dereference"),
    ]

    for original, new in mapped_updates:
        if original in results.tests:
            results.tests[new] = results.tests[original]
            del results.tests[original]

    # This needs to use posixpath rather than grouptools because version 4 uses
    # / as a separator, but grouptools isn't guaranteed to do so
    for test, result in results.tests.items():
        if posixpath.dirname(test) == 'glslparsertest':
            group = posixpath.join('glslparsertest/shaders',
                                   posixpath.basename(test))
            results.tests[group] = result
            del results.tests[test]

    results.results_version = 4

    return results


def _update_four_to_five(results):                                                                                                    
    """Updates json results from version 4 to version 5."""
    new_tests = {}

    for name, test in results.tests.iteritems():
        new_tests[name.replace('/', '@').replace('\\', '@')] = test

    results.tests = new_tests
    results.results_version = 5

    return results


REGISTRY = Registry(
    extensions=['', '.json'],
    backend=JSONBackend,
    load=load_results,
    meta=set_meta,
)
