#!/usr/bin/env python3
# coding=utf-8
#
# Copyright (c) 2020 Collabora Ltd
# Copyright © 2020 Valve Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# SPDX-License-Identifier: MIT

import argparse
import glob
import os
import shutil
import sys
import yaml

from pathlib import Path
from PIL import Image

import framework.replay.dump_trace_images
import framework.replay.parsers
import framework.replay.query_traces_yaml as qty
from framework.replay.download_utils import ensure_file
from framework.replay.image_checksum import hexdigest_from_image
from framework.replay.upload_utils import upload_file

TRACES_DB_PATH = "./traces-db/"
RESULTS_PATH = "./results/"


def replay(trace_path, device_name):
    success = dump_trace_images.dump_from_trace(Path(trace_path),
                                                [], device_name)

    if not success:
        print("[check_image] Trace %s couldn't be replayed. "
              "See above logs for more information." % trace_path)
        return None, None, None
    else:
        test_path = os.path.join(os.path.dirname(trace_path), "test",
                                 device_name)
        file_name = os.path.basename(trace_path)
        files = glob.glob(os.path.join(test_path, file_name + "-*" + ".png"))
        assert(files)
        image_file = files[0]
        files = glob.glob(os.path.join(test_path, file_name + ".log"))
        assert(files)
        log_file = files[0]
        return (hexdigest_from_image(image_file), image_file, log_file)


def gitlab_check_trace(download_url,
                       device_name, trace_path, expected_checksum):
    ensure_file(download_url, trace_path, TRACES_DB_PATH)

    result = {}
    result[trace_path] = {}
    result[trace_path]['expected'] = expected_checksum

    checksum, image_file, log_file = replay(TRACES_DB_PATH + trace_path,
                                            device_name)
    if checksum is None:
        result[trace_path]['actual'] = 'error'
        return False, result
    elif checksum == expected_checksum:
        print("[check_image] Images match for %s" % (trace_path))
        ok = True
    else:
        print("[check_image] Images differ for %s (expected: %s, actual: %s)" %
              (trace_path, expected_checksum, checksum))
        print("[check_image] For more information see "
              "https://gitlab.freedesktop.org/"
              "mesa/mesa/blob/master/.gitlab-ci/tracie/README.md")
        ok = False

    trace_dir = os.path.dirname(trace_path)
    dir_in_results = os.path.join(trace_dir, "test", device_name)
    results_path = os.path.join(RESULTS_PATH, dir_in_results)
    os.makedirs(results_path, exist_ok=True)
    shutil.move(log_file, os.path.join(results_path,
                                       os.path.basename(log_file)))
    if not ok and os.environ.get('TRACIE_UPLOAD_TO_MINIO', '0') == '1':
        upload_file(image_file, 'image/png', device_name)
    if not ok or os.environ.get('TRACIE_STORE_IMAGES', '0') == '1':
        image_name = os.path.basename(image_file)
        shutil.move(image_file, os.path.join(results_path, image_name))
        result[trace_path]['image'] = os.path.join(dir_in_results, image_name)

    result[trace_path]['actual'] = checksum

    return ok, result


def write_results(results):
    os.makedirs(RESULTS_PATH, exist_ok=True)
    with open(os.path.join(RESULTS_PATH, 'results.yml'), 'w') as f:
        yaml.safe_dump(results, f, default_flow_style=False)
    if os.environ.get('TRACIE_UPLOAD_TO_MINIO', '0') == '1':
        upload_file(os.path.join(RESULTS_PATH, 'results.yml'), 'text/yaml',
                    device_name)


def from_yaml(args):
    y = qty.load_yaml(args.yaml_file)

    download_url = qty.download_url(y)

    all_ok = True
    results = {}
    t_list = qty.traces(y, '', args.device_name, True)
    for t in t_list:
        ok, result = gitlab_check_trace(download_url,
                                        args.device_name,
                                        t['path'],
                                        t['checksum'])
        all_ok = all_ok and ok
        results.update(result)

    os.makedirs(RESULTS_PATH, exist_ok=True)
    with open(os.path.join(RESULTS_PATH, 'results.yml'), 'w') as f:
        yaml.safe_dump(results, f, default_flow_style=False)
    if os.environ.get('TRACIE_UPLOAD_TO_MINIO', '0') == '1':
        upload_artifact(os.path.join(RESULTS_PATH, 'results.yml'),
                        'text/yaml', device_name)

    return all_ok


def main(args):
    parser = argparse.ArgumentParser()

    # Add a destination due to
    # https://github.com/python/cpython/pull/3027#issuecomment-330910633
    subparsers = parser.add_subparsers(dest='command', required=True)

    parser_yaml = subparsers.add_parser('yaml', parents=[parsers.DEVICE,
                                                         parsers.YAML])
    parser_yaml.set_defaults(func=from_yaml)

    parser_trace = subparsers.add_parser('trace', parents=[parsers.DEVICE])
    parser_trace.add_argument(
        '-u', '--download-url', default=None,
        help=('the URL from which to download the traces'))
    parser_trace.add_argument(
        '-p', '--path', required=True,
        help='the path to the trace file in the traces-db repository')
    parser_trace.add_argument(
        '-e', '--expected-checksum', required=True,
        help=('the expected checksum value to obtain '
              'when replaying a trace in a specific device'))
    parser_trace.set_defaults(func=trace)

    parser_query = subparsers.add_parser(
        'query',
        add_help=False,
        help=('Queries for specific information '
              'from a traces description file listing traces '
              'and their checksums for each device.'))
    parser_query.set_defaults(func=qty.query)

    # Parse the known arguments (tracie.py yaml or tracie.py query for
    # example), and then pass the arguments that this parser doesn't
    # know about to that executable
    parsed, args = parser.parse_known_args(input_)
    try:
        runner = parsed.func
    except AttributeError:
        parser.print_help()
        return False
    return runner(args or parsed)


if __name__ == "__main__":
    all_ok = main(sys.argv[1:])
    sys.exit(0 if all_ok else 1)
