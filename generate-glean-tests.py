#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright © 2010 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
# Authors:
#    Eric Anholt <eric@anholt.net>

from getopt import getopt, GetoptError

import os
import re
import sys


def usage():
    USAGE = """\
Usage %(progName) [cppfile] [add_prefix]

cppfile: path to glean cppfile to parse
add_suffix: prefix to have in test name i.e. glsl1 -> add_glsl1
"""
    print USAGE % {'progName': sys.argv[0]}
    sys.exit(1)


def main():
    try:
        options, args = getopt(sys.argv[1:], "hdt:n:x:",
                               ["help", "dry-run", "tests=", "name=",
                                "exclude-tests="])
    except GetoptError:
        usage()

    if len(args) != 2:
        usage()

    suffix = args[1]

    fileIN = open(args[0], 'r')
    line = fileIN.readline()
    next_is_name = False

    while line:
        if next_is_name:
            name = line.lstrip("    \",")
            name = name.rstrip("\n")
            if re.match(r'GLint stat', name):
                break
            if not re.match(r'//', name):
                name = re.sub(r'".*', r'', name)
                print "add_" + suffix + "('" + name + "')"
                next_is_name = False
        if line == "    {\n":
            next_is_name = True
        line = fileIN.readline()


if __name__ == "__main__":
    main()
