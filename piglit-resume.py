#!/usr/bin/env python
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

import sys
import os
import os.path as path
import argparse

import framework.core as core


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("results_path",
                        type=path.realpath,
                        metavar="<Results Path>",
                        help="Path to results folder")
    args = parser.parse_args()
    
    results = core.load_results(args.results_path)
    env = core.Environment(concurrent=results.options['concurrent'],
                           exclude_filter=results.options['exclude_filter'],
                           include_filter=results.options['filter'],
                           execute=results.options['execute'],
                           valgrind=results.options['valgrind'],
                           dmesg=results.options['dmesg'])
    
    # Change working directory to the piglit directory
    os.chdir(path.dirname(path.realpath(sys.argv[0])))
    
    results_path = path.join(args.results_path, "main")
    json_writer = core.JSONWriter(open(results_path, 'w+'))
    json_writer.open_dict()
    json_writer.write_dict_key("options")
    json_writer.open_dict()
    for key, value in results.options.iteritems():
        json_writer.write_dict_item(key, value)
    json_writer.close_dict()
    
    json_writer.write_dict_item('name', results.name)
    for (key, value) in env.collectData().items():
        json_writer.write_dict_item(key, value)
    
    json_writer.write_dict_key('tests')
    json_writer.open_dict()
    for key, value in results.tests.iteritems():
        json_writer.write_dict_item(key, value)
        env.exclude_tests.add(key)
    json_writer.close_dict()
    
    profile = core.loadTestProfile(results.options['profile'])
    # This is resumed, don't bother with time since it wont be accurate anyway
    profile.run(env, json_writer)
    
    json_writer.close_dict()
    json_writer.close_dict()
    json_writer.file.close()
    
    print("\n"
          "Thank you for running Piglit!\n"
          "Results have ben wrriten to {0}".format(results_path))
    
if __name__ == "__main__":
    main()