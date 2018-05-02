# encoding=utf-8
# Copyright © 2018 Intel Corporation

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

"""Script that finds all static tests of one kind or another."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import argparse
import io
import os

import six


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('sourcedir')
    parser.add_argument(
        'mode',
        choices=['shader', 'glslparser', 'asmparser', 'program'])
    parser.add_argument('output')
    args = parser.parse_args()

    if args.mode == 'asmparser':
        exts = ['.txt']
        directory = os.path.join(args.sourcedir, 'asmparsertest', 'shaders')
    elif args.mode == 'glslparser':
        exts = ['.frag', '.vert', '.goem', '.tess', 'tesc', '.comp']
        directory = args.sourcedir
    elif args.mode == 'shader':
        exts = ['.shader_test']
        directory = args.sourcedir
    elif args.mode == 'program':
        exts = ['.program_test']
        directory = args.sourcedir

    files = []
    for dirpath, _, filenames in os.walk(directory):
        for filename in filenames:
            if os.path.splitext(filename)[1] in exts:
                name = os.path.join(dirpath, filename)
                if six.PY2:
                    # This might not be correct, but it's fine. As long as the
                    # two files are the same it'll work, and utf-8 is what
                    # everyone *should* be using, and as a superset of ascii
                    # *should* cover most people
                    name = name.decode('utf-8', 'replace')
                files.append(name)

    if os.path.exists(args.output):
        with io.open(args.output, 'rt', encoding='utf-8') as f:
            existing = f.read().rstrip().split('\n')
    else:
        existing = []

    if sorted(files) != sorted(existing):
        with io.open(args.output, 'wt', encoding='utf-8') as f:
            for filename in files:
                f.write(filename)
                f.write('\n')


if __name__ == '__main__':
    main()
