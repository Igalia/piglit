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
import hashlib
import os
import shutil
import sys
import yaml

from pathlib import Path
from PIL import Image

import dump_trace_images
from download_utils import ensure_file
from image_checksum import hexdigest_from_image
from upload_utils import upload_file

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


def gitlab_check_trace(project_url,
                       device_name, trace_path, expected_checksum):
    ensure_file(project_url, trace_path, TRACES_DB_PATH)

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

def run(filename, device_name):

    with open(filename, 'r') as f:
        y = yaml.safe_load(f)

    if "traces-db" in y:
        project_url = y["traces-db"]["download-url"]
    else:
        project_url = None

    traces = y['traces'] or []
    all_ok = True
    results = {}
    for trace in traces:
        for expectation in trace['expectations']:
            if expectation['device'] == device_name:
                ok, result = gitlab_check_trace(project_url,
                                                device_name,
                                                trace['path'],
                                                expectation['checksum'])
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
    parser.add_argument('--file', required=True,
                        help=('the name of the traces.yml file listing traces '
                              'and their checksums for each device'))
    parser.add_argument('--device-name', required=True,
                        help=('the name of the graphics device '
                              'used to replay traces'))

    args = parser.parse_args(args)
    return run(args.file, args.device_name)

if __name__ == "__main__":
    all_ok = main(sys.argv[1:])
    sys.exit(0 if all_ok else 1)
