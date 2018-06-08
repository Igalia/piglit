# encoding=utf-8
# Copyright © 2016 Intel Corporation

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

"""Shared data for backend tests."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

from framework.options import OPTIONS


INITIAL_METADATA = {
    'name': 'name',
    'options': dict(OPTIONS),
    'info': {
        'system': {}
    }
}

# This is current JSON data, in raw form with only the minimum required
# changes. This does not contain piglit specifc objects, only strings, floats,
# ints, and Nones (instead of JSON's null)
JSON = {
    "results_version": 10,
    "time_elapsed": {
        "start": 1469638791.2351687,
        "__type__": "TimeAttribute",
        "end": 1469638791.4387212
    },
    "tests": {
        "spec@!opengl 1.0@gl-1.0-readpixsanity": {
            "dmesg": "",
            "traceback": None,
            "err": "piglit: error: waffle_display_connect failed due to "
                   "WAFFLE_ERROR_UNKNOWN: open drm file for gbm failed\n",
            "subtests": {
                "__type__": "Subtests"
            },
            "out": "",
            "exception": None,
            "command": "/home/user/source/piglit/bin/gl-1.0-readpixsanity "
                       "-auto -fbo",
            "time": {
                "start": 1469638791.2383287,
                "__type__": "TimeAttribute",
                "end": 1469638791.2439244
            },
            "pid": [11768],
            "__type__": "TestResult",
            "returncode": 1,
            "result": "fail",
            "environment": ("PIGLIT_SOURCE_DIR=\"/home/user/source/piglit\" "
                            " PIGLIT_PLATFORM=\"gbm\"")
        }
    },
    "options": {
        "dmesg": False,
        "concurrent": "some",
        "include_filter": [],
        "monitored": False,
        "execute": True,
        "valgrind": False,
        "profile": [
            "sanity"
        ],
        "log_level": "quiet",
        "env": {
            "PIGLIT_SOURCE_DIR": "/home/user/source/piglit",
            "PIGLIT_PLATFORM": "gbm"
        },
        "platform": "gbm",
        "sync": False,
        "exclude_tests": [],
        "exclude_filter": []
    },
    "name": "foo",
    "__type__": "TestrunResult",
    "info": {
        "system": {
            "lspci": "00:00.0 Host bridge...",
        },
    },
    "totals": {
        "spec": {
            '__type__': 'Totals',
            "warn": 0,
            "timeout": 0,
            "skip": 0,
            "crash": 0,
            "pass": 0,
            "fail": 1,
            "dmesg-warn": 0,
            "incomplete": 0,
            "notrun": 0,
            "dmesg-fail": 0
        },
        "": {
            '__type__': 'Totals',
            "warn": 0,
            "timeout": 0,
            "skip": 0,
            "crash": 0,
            "pass": 0,
            "fail": 1,
            "dmesg-warn": 0,
            "incomplete": 0,
            "notrun": 0,
            "dmesg-fail": 0
        },
        "spec@!opengl 1.0": {
            '__type__': 'Totals',
            "warn": 0,
            "timeout": 0,
            "skip": 0,
            "crash": 0,
            "pass": 0,
            "fail": 1,
            "dmesg-warn": 0,
            "incomplete": 0,
            "notrun": 0,
            "dmesg-fail": 0
        },
        "root": {
            '__type__': 'Totals',
            "warn": 0,
            "timeout": 0,
            "skip": 0,
            "crash": 0,
            "pass": 0,
            "fail": 1,
            "dmesg-warn": 0,
            "incomplete": 0,
            "notrun": 0,
            "dmesg-fail": 0
        }
    }
}
