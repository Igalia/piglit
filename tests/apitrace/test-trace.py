#!/usr/bin/env python
# coding=utf-8

# Copyright (c) 2016 Broadcom
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

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import argparse
import re
import sys
import os
import subprocess

def piglit_report_result(result):
    str = 'PIGLIT: {{"result": "{}" }}\n'.format(result)
    if result == 'warn' or result == 'fail':
        sys.stderr.write(str)
        exit(1)
    else:
        sys.stdout.write(str)
        exit(0)

def piglit_merge_result(result, sub_result):
    if result == 'skip':
        return sub_result
    elif sub_result == 'pass':
        return result
    elif sub_result == 'fail':
        return sub_result
    else:
        print('unknown sub result {}'.format(sub_result))
        piglit_report_result('fail')

def run_and_collect_output(filename):
    # Save off the path to the trace, minus '.trace'
    trace_prefix = os.path.splitext(filename)[0]

    # Place our generated .pngs in the directory of the trace, named
    # "tracename.out.drawcall.png".
    trace_png_prefix='{}.out.'.format(trace_prefix)

    command = [
        'apitrace',
        'replay',
        '-b', # No performance debugging or glGetError()s, please.
        '-v', # Get the "Wrote ..." lines.
        '-s', # Write pngs with the following prefix.
        trace_png_prefix,
        filename
    ]

    try:
        p = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)
        (stdout, stderr) = p.communicate()
        apitrace_output = (stdout + stderr).decode("utf-8")
    except KeyboardInterrupt:
        exit(1)
    except Exception:
        piglit_report_result('fail')

    return apitrace_output

def compare_images(out, expected):
    """Uses apitrace diff-images to compare two images, returning a

    piglit status string.
    """
    command = [
        'apitrace',
        'diff-images',
        '-a', # alpha channel counts
        out,
        expected,
    ]

    retcode = subprocess.call(command)
    if retcode == 0:
        return 'pass'
    elif retcode == 1:
        return 'fail'
    else:
        print('unknown apitrace diff-images return code {}'.format(retcode))
        piglit_report_result('fail')

def run_trace(filename, driver_categories):
    apitrace_output = run_and_collect_output(filename)

    result = 'skip'
    for line in apitrace_output.splitlines():
        m = re.match(r'Wrote (.*)\.out\.(.*).png', line)
        if m is None:
            continue

        prefix = m.group(1)
        drawcall = m.group(2)
        out_file = '{}.out.{}.png'.format(prefix, drawcall)

        # Go through the list of categories our driver
        for category in driver_categories:
            expected_file = '{}.expected.{}.{}.png'.format(prefix,
                                                           category,
                                                           drawcall)
            print('looking for {}'.format(expected_file))
            if not os.path.isfile(expected_file):
                continue

            sub_result = compare_images(out_file, expected_file)

            result = piglit_merge_result(result, sub_result)
            break

    piglit_report_result(result)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("filename",
                        metavar="<file.trace>",
                        help="Path to apitrace file")
    parser.add_argument("driver_categories",
                        metavar="<driver,categories>",
                        help="Driver categories to look for expected images in, in least-specific to most-specific order, separated by commas")
    args = parser.parse_args()

    run_trace(args.filename, args.driver_categories.split(','))

if __name__ == "__main__":
    main()
