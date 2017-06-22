# Copyright (c) 2014, 2016-2017 Intel Corporation

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

# The minimum JSON format supported
MINIMUM_SUPPORTED_VERSION = 7

# The level to indent a final file
INDENT = 4


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

            # Flush the metadata to the disk, always
            f.flush()
            os.fsync(f.fileno())

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
        tests_dir = os.path.join(self._dest, 'tests')
        file_list = sorted(
            (f for f in os.listdir(tests_dir) if f.endswith('.json')),
            key=lambda p: int(os.path.splitext(p)[0]))

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

            for test in file_list:
                test = os.path.join(tests_dir, test)
                if os.path.isfile(test):
                    # Try to open the json snippets. If we fail to open a test
                    # then throw the whole thing out. This gives us atomic
                    # writes, the writing worked and is valid or it didn't
                    # work.
                    try:
                        with open(test, 'r') as f:
                            data['tests'].update(json.load(f))
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
                        s.iterwrite(six.iteritems(json.load(n, object_pairs_hook=collections.OrderedDict)))

                    if metadata:
                        s.iterwrite(six.iteritems(metadata))

                    with s.subobject('tests') as t:
                        for test in file_list:
                            test = os.path.join(tests_dir, test)
                            if os.path.isfile(test):
                                try:
                                    with open(test, 'r') as f:
                                        a = json.load(f)
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
        for name in ['results.json.{}'.format(compression_), 'results.json']:
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

    return results.TestrunResult.from_dict(_update_results(testrun, filepath))


def set_meta(results):
    """Set json specific metadata on a TestrunResult."""
    results.results_version = CURRENT_JSON_VERSION


def _load(results_file):
    """Load a json results instance and return a TestrunResult.

    This function converts an existing, fully completed json run.

    """
    try:
        result = json.load(results_file, object_pairs_hook=collections.OrderedDict)
    except ValueError as e:
        raise exceptions.PiglitFatalError(
            'While loading json results file: "{}",\n'
            'the following error occurred:\n{}'.format(results_file.name,
                                                       six.text_type(e)))

    return result


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

    meta['tests'] = collections.OrderedDict()

    # Load all of the test names and added them to the test list
    tests_dir = os.path.join(results_dir, 'tests')
    file_list = sorted(
        (l for l in os.listdir(tests_dir) if l.endswith('.json')),
        key=lambda p: int(os.path.splitext(p)[0]))

    for file_ in file_list:
        with open(os.path.join(tests_dir, file_), 'r') as f:
            try:
                meta['tests'].update(json.load(f))
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
            7: _update_seven_to_eight,
            8: _update_eight_to_nine,
        }

        while results['results_version'] < CURRENT_JSON_VERSION:
            results = updates[results['results_version']](results)

        return results

    if results['results_version'] < MINIMUM_SUPPORTED_VERSION:
        raise exceptions.PiglitFatalError(
            'Unsupported version "{}", '
            'minimum supported version is "{}"'.format(
                results['results_version'], MINIMUM_SUPPORTED_VERSION))

    # If the results version is the current version there is no need to
    # update, just return the results
    if results['results_version'] == CURRENT_JSON_VERSION:
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


def _update_seven_to_eight(result):
    """Update json results from version 7 to 8.

    This update replaces the time attribute float with a TimeAttribute object,
    which stores a start time and an end time, and provides methods for getting
    total and delta.

    This value is used for both TestResult.time and TestrunResult.time_elapsed.

    """
    for test in compat.viewvalues(result['tests']):
        test['time'] = {'start': 0.0, 'end': float(test['time']),
                        '__type__': 'TimeAttribute'}

    result['time_elapsed'] = {'start': 0.0, 'end':
                              float(result['time_elapsed']),
                              '__type__': 'TimeAttribute'}

    result['results_version'] = 8

    return result


def _update_eight_to_nine(result):
    """Update json results from version 8 to 9.

    This changes the PID feild of the TestResult object to alist of Integers or
    null rather than a single integer or null.

    """
    for test in compat.viewvalues(result['tests']):
        if 'pid' in test:
            test['pid'] = [test['pid']]
        else:
            test['pid'] = []

    result['results_version'] = 9

    return result


REGISTRY = Registry(
    extensions=['.json'],
    backend=JSONBackend,
    load=load_results,
    meta=set_meta,
)
