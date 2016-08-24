# Copyright (c) 2014,2016 Intel Corporation

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

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import collections
import functools
import os
import posixpath
import shutil
import sys

try:
    import simplejson as json
except ImportError:
    import json

import six
try:
    import jsonstreams
    _STREAMS = True
except ImportError:
    _STREAMS = False

from framework import status, results, exceptions, compat
from .abstract import FileBackend, write_compressed
from .register import Registry
from . import compression

__all__ = [
    'REGISTRY',
    'JSONBackend',
]

# The current version of the JSON results
CURRENT_JSON_VERSION = 9

# The level to indent a final file
INDENT = 4

_DECODER_TABLE = {
    'Subtests': results.Subtests,
    'TestResult': results.TestResult,
    'TestrunResult': results.TestrunResult,
    'TimeAttribute': results.TimeAttribute,
    'Totals': results.Totals,
}


def piglit_encoder(obj):
    """ Encoder for piglit that can transform additional classes into json

    Adds support for status.Status objects and for set() instances

    """
    if isinstance(obj, status.Status):
        return six.text_type(obj)
    elif isinstance(obj, set):
        return list(obj)
    elif hasattr(obj, 'to_json'):
        return obj.to_json()
    return obj


def piglit_decoder(obj):
    """Json decoder for piglit that can load TestResult objects."""
    if isinstance(obj, dict):
        try:
            return _DECODER_TABLE[obj['__type__']].from_dict(obj)
        except KeyError:
            pass
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
        # If jsonstreams is not present then build a complete tree of all of
        # the data and write it with json.dump
        if not _STREAMS:
            # Create a dictionary that is full of data to be written to a
            # single file
            data = collections.OrderedDict()

            # Load the metadata and put it into a dictionary
            with open(os.path.join(self._dest, 'metadata.json'), 'r') as f:
                data.update(json.load(f))

            # If there is more metadata add it the dictionary
            if metadata:
                data.update(metadata)

            # Add the tests to the dictionary
            data['tests'] = collections.OrderedDict()

            test_dir = os.path.join(self._dest, 'tests')
            for test in os.listdir(test_dir):
                test = os.path.join(test_dir, test)
                if os.path.isfile(test):
                    # Try to open the json snippets. If we fail to open a test
                    # then throw the whole thing out. This gives us atomic
                    # writes, the writing worked and is valid or it didn't
                    # work.
                    try:
                        with open(test, 'r') as f:
                            data['tests'].update(
                                json.load(f, object_hook=piglit_decoder))
                    except ValueError:
                        pass
            assert data['tests']

            data = results.TestrunResult.from_dict(data)

            # write out the combined file. Use the compression writer from the
            # FileBackend
            with self._write_final(os.path.join(self._dest, 'results.json')) as f:
                json.dump(data, f, default=piglit_encoder, indent=INDENT)

        # Otherwise use jsonstreams to write the final dictionary. This uses an
        # external library, but is slightly faster and uses considerably less
        # memory that building a complete tree.
        else:
            encoder = functools.partial(json.JSONEncoder, default=piglit_encoder)

            with self._write_final(os.path.join(self._dest, 'results.json')) as f:
                with jsonstreams.Stream(jsonstreams.Type.object, fd=f, indent=4,
                                        encoder=encoder, pretty=True) as s:
                    s.write('__type__', 'TestrunResult')
                    with open(os.path.join(self._dest, 'metadata.json'),
                              'r') as n:
                        s.iterwrite(six.iteritems(json.load(n)))

                    if metadata:
                        s.iterwrite(six.iteritems(metadata))

                    test_dir = os.path.join(self._dest, 'tests')
                    with s.subobject('tests') as t:
                        for test in os.listdir(test_dir):
                            test = os.path.join(test_dir, test)
                            if os.path.isfile(test):
                                try:
                                    with open(test, 'r') as f:
                                        a = json.load(
                                            f, object_hook=piglit_decoder)
                                except ValueError:
                                    continue

                                t.iterwrite(six.iteritems(a))


        # Delete the temporary files
        os.unlink(os.path.join(self._dest, 'metadata.json'))
        shutil.rmtree(os.path.join(self._dest, 'tests'))

    @staticmethod
    def _write(f, name, data):
        json.dump({name: data}, f, default=piglit_encoder)


def load_results(filename, compression_):
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
    elif (os.path.exists(os.path.join(filename, 'metadata.json')) and
          not os.path.exists(os.path.join(
              filename, 'results.json.' + compression_))):
        # We want to hit this path only if there isn't a
        # results.json.<compressions>, since otherwise we'll continually
        # regenerate values that we don't need to.
        return _resume(filename)
    else:
        # Look for a compressed result first, then a bare result, finally for
        # an old main file
        for name in ['results.json.{}'.format(compression_),
                     'results.json',
                     'main']:
            if os.path.exists(os.path.join(filename, name)):
                filepath = os.path.join(filename, name)
                break
        else:
            raise exceptions.PiglitFatalError(
                'No results found in "{}" (compression: {})'.format(
                    filename, compression_))

    assert compression_ in compression.COMPRESSORS, \
        'unsupported compression type'

    with compression.DECOMPRESSORS[compression_](filepath) as f:
        testrun = _load(f)

    return _update_results(testrun, filepath)


def set_meta(results):
    """Set json specific metadata on a TestrunResult."""
    results.results_version = CURRENT_JSON_VERSION


def _load(results_file):
    """Load a json results instance and return a TestrunResult.

    This function converts an existing, fully completed json run.

    """
    try:
        result = json.load(results_file, object_hook=piglit_decoder)
    except ValueError as e:
        raise exceptions.PiglitFatalError(
            'While loading json results file: "{}",\n'
            'the following error occurred:\n{}'.format(results_file.name,
                                                      str(e)))

    if isinstance(result, results.TestrunResult):
        return result
    return results.TestrunResult.from_dict(result, _no_totals=True)


def _resume(results_dir):
    """Loads a partially completed json results directory."""
    # TODO: could probably use TestrunResult.from_dict here
    # Pylint can't infer that the json being loaded is a dict
    # pylint: disable=maybe-no-member
    assert os.path.isdir(results_dir), \
        "TestrunResult.resume() requires a directory"

    # Load the metadata
    with open(os.path.join(results_dir, 'metadata.json'), 'r') as f:
        meta = json.load(f)
    assert meta['results_version'] == CURRENT_JSON_VERSION, \
        "Old results version, resume impossible"

    meta['tests'] = {}

    # Load all of the test names and added them to the test list
    for file_ in os.listdir(os.path.join(results_dir, 'tests')):
        with open(os.path.join(results_dir, 'tests', file_), 'r') as f:
            try:
                meta['tests'].update(json.load(f, object_hook=piglit_decoder))
            except ValueError:
                continue

    return results.TestrunResult.from_dict(meta)


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
            5: _update_five_to_six,
            6: _update_six_to_seven,
            7: _update_seven_to_eight,
            8: _update_eight_to_nine,
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
    with write_compressed(file_) as f:
        json.dump(results, f, default=piglit_encoder, indent=INDENT)


def _update_zero_to_one(result):
    """ Update version zero results to version 1 results

    Changes from version 0 to version 1

    - dmesg is sometimes stored as a list, sometimes stored as a string. In
      version 1 it is always stored as a string
    - in version 0 subtests are sometimes stored as duplicates, sometimes stored
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

    for name, test in six.iteritems(result.tests):
        assert not isinstance(test, results.TestResult), \
            'Test was erroniaously turned into a TestResult'

        # fix dmesg errors if any
        if isinstance(test.get('dmesg'), list):
            test['dmesg'] = '\n'.join(test['dmesg'])

        # If a test as an info attribute, we want to remove it, if it doesn't
        # have a returncode, out, or attribute we'll want to get those out of
        # info first
        #
        # This expects that the order of info is roughly returncode, errors,
        # output, *extra it can handle having extra information in the middle,
        if (None in [test.get('out'), test.get('err'),
                     test.get('returncode')] and test.get('info')):

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
                # appropriately
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
            for sub in six.iterkeys(test['subtest']):
                # adding the leading / ensures that we get exactly what we
                # expect, since endswith does a character by character match, if
                # the subtest name is duplicated it won't match, and if there
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
        del result.tests[name]
    result.tests.update(updated_results)

    # set the results version
    result.results_version = 1

    return result


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

    for name, test in six.iteritems(results.tests):
        new_tests[name.replace('/', '@').replace('\\', '@')] = test

    results.tests = new_tests
    results.results_version = 5

    return results


def _update_five_to_six(result):
    """Updates json results from version 5 to 6.

    This uses a special field to for marking TestResult instances, rather than
    just checking for fields we expect.

    """
    new_tests = {}

    for name, test in six.iteritems(result.tests):
        new_tests[name] = results.TestResult.from_dict(test)

    result.tests = new_tests
    result.results_version = 6

    return result


def _update_six_to_seven(result):
    """Update json results from version 6 to 7.

    Version 7 results always contain the totals member.

    """
    if not result.totals:
        result.calculate_group_totals()
    result.results_version = 7

    return result


def _update_seven_to_eight(result):
    """Update json results from version 7 to 8.

    This update replaces the time attribute float with a TimeAttribute object,
    which stores a start time and an end time, and provides methods for getting
    total and delta.

    This value is used for both TestResult.time and TestrunResult.time_elapsed.

    """
    for test in compat.viewvalues(result.tests):
        test.time = results.TimeAttribute(end=test.time)

    result.time_elapsed = results.TimeAttribute(end=result.time_elapsed)

    result.results_version = 8

    return result


def _update_eight_to_nine(result):
    """Update json results from version 8 to 9.

    This changes the PID feild of the TestResult object to alist of Integers or
    null rather than a single integer or null.

    """
    for test in compat.viewvalues(result.tests):
        test.pid = [test.pid]

    result.results_version = 9

    return result


REGISTRY = Registry(
    extensions=['', '.json'],
    backend=JSONBackend,
    load=load_results,
    meta=set_meta,
)
