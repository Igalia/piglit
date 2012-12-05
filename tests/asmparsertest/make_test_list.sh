#!/bin/bash
#
# Copyright © 2009 Intel Corporation
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
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

# Generate a test list file from all the shaders in shaders/**
#
# Authors: Ian Romanick <ian.d.romanick@intel.com>

function emit_test_list
{
    dir="$1"
    subdir=$(basename $dir)

    cd shaders

    find $dir -type f -name '*.txt' | sort |\
    while read f
    do
	printf "asmparsertest['%s'] = PlainExecTest([testBinDir + 'asmparsertest', '-auto', '%s', 'tests/asmparsertest/shaders/%s'])\n" "$f" "$subdir" "$f"
    done

    cd ..
}


echo "asmparsertest = Group()"
emit_test_list ARBfp1.0
echo
emit_test_list ARBvp1.0
