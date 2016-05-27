#encoding=utf-8
# Copyright (c) 2014-2016 Intel Coporation

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

"""Piglit specific helpers

These helpers are all piglit specific tools.

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
from contextlib import contextmanager
import copy
import functools
import os
import tempfile as tempfile_

try:
    import simplejson as json
except ImportError:
    import json

from framework import test, backends, core, results

core.get_config()

_WRITE_MODE = 'w'
_READ_MODE = 'r'


class _Tree(dict):
    """Private helper to make JSON_DATA easier to work with."""
    def __getitem__(self, key):
        try:
            return super(_Tree, self).__getitem__(key)
        except KeyError:
            ret = self[key] = _Tree()
            return ret


JSON_DATA = {
    "options": {
        "profile": "tests/fake.py",
        "filter": [],
        "exclude_filter": []
    },
    "results_version": backends.json.CURRENT_JSON_VERSION,
    "name": "fake-tests",
    "lspci": "fake",
    "glxinfo": "fake",
    "tests": _Tree({
        "sometest": {
            'result': 'pass',
            'time': 1.2,
        }
    })
}

_SAVED_COMPRESSION = os.environ.get('PIGLIT_COMPRESSION')


class Test(test.Test):
    """A basic dmmmy Test class that can be used in places a test is required.

    This provides dummy version of abstract methods in
    framework.test.base.Test, which allows it to be initialized and run, but is
    unlikely to be useful for running.

    This is mainly intended for testing where a Test class is required, but
    doesn't need to be run.

    """
    def interpret_result(self):
        pass


@contextmanager
def resultfile():
    """ Create a stringio with some json in it and pass that as results """
    data = copy.deepcopy(JSON_DATA)
    data['tests']['sometest'] = results.TestResult('pass')
    data['tests']['sometest'].time = 1.2
    data = results.TestrunResult.from_dict(data)
    with tempfile_.NamedTemporaryFile(mode=_WRITE_MODE, delete=False) as f:
        json.dump(data, f, default=backends.json.piglit_encoder)

    try:
        yield f
    finally:
        os.remove(f.name)


def set_piglit_conf(*values):
    """Decorator that sets and then usets values from core.PIGLIT_CONF.

    This decorator takes arguments for sections and options to overwrite in
    piglit.conf. It will first backup any options to be overwritten, and store
    any options that don't exist. Then it will set those options, run the test,
    and finally restore any options overwritten, and delete any new options
    added. If value is set to NoneType the option will be removed.

    Arguments:
    Values -- tuples containing a section, option, and value in the form:
    (<section>, <option>, <value>)

    """
    def _decorator(func):
        """The actual decorator."""

        @functools.wraps(func)
        def _inner(*args, **kwargs):
            """The function returned by the decorator."""
            backup = set()
            remove = set()

            for section, key, value in values:
                get = core.PIGLIT_CONFIG.safe_get(section, key)
                # If there is a value, save that value to restore it, if there
                # is not a value AND if there is a value to set (IE: we're not
                # clearing a value if it exsists), the add it to remove
                if get is not None:
                    backup.add((section, key, get))
                elif value is not None:
                    remove.add((section, key))

                # set any new values, and remove any values that are set to
                # None
                if value is not None:
                    if not core.PIGLIT_CONFIG.has_section(section):
                        core.PIGLIT_CONFIG.add_section(section)
                    core.PIGLIT_CONFIG.set(section, key, value)
                elif (core.PIGLIT_CONFIG.has_section(section) and
                      core.PIGLIT_CONFIG.has_option(section, key)):
                    core.PIGLIT_CONFIG.remove_option(section, key)

            try:
                func(*args, **kwargs)
            finally:
                # Restore all values
                for section, key, value in backup:
                    core.PIGLIT_CONFIG.set(section, key, value)
                for section, key in remove:
                    core.PIGLIT_CONFIG.remove_option(section, key)

        return _inner
    return _decorator


def set_compression(mode):
    """lock piglit compression mode to specifed value.

    Meant to be called from setup_module function.

    """
    # The implimentation details of this is that the environment value is the
    # first value looked at for setting compression, so if it's set all other
    # values will be ignored.
    os.environ['PIGLIT_COMPRESSION'] = mode


def unset_compression():
    """Restore compression to the origonal value.

    Counterpart to set_compression. Should be called from teardown_module.

    """
    if _SAVED_COMPRESSION is not None:
        os.environ['PIGLIT_COMPRESSION'] = _SAVED_COMPRESSION
    else:
        del os.environ['PIGLIT_COMPRESSION']
