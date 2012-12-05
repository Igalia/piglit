# Copyright (C) 2012 Intel Corporation
#
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

import os
import os.path as path

from exectest import PlainExecTest

def add_shader_test_dir(group, dirpath, recursive=False):
    """Add all shader tests in a directory to the given group."""
    for filename in os.listdir(dirpath):
        filepath = path.join(dirpath, filename)
        if path.isdir(filepath):
            if not recursive:
                continue
            if not filename in group:
                group[filename] = Group()
            add_shader_test_dir(group[filename], filepath, recursive)
        else:
            ext = filename.rsplit('.')[-1]
            if ext != 'shader_test':
                continue
            testname = filename[0:-(len(ext) + 1)] # +1 for '.'
            group[testname] = concurrent_test('shader_runner ' + filepath)

