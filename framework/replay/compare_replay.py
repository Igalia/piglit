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

import os
import shutil
import yaml

from glob import glob
from os import path

from framework import status
from framework.replay import query_traces_yaml as qty
from framework.replay.download_utils import ensure_file
from framework.replay.dump_trace_images import dump_from_trace
from framework.replay.image_checksum import hexdigest_from_image
from framework.replay.upload_utils import upload_file


__all__ = ['from_yaml',
           'trace']

TRACES_DB_PATH = './traces-db/'
RESULTS_PATH = './results/'

def replay(trace_path, device_name):
    success = dump_from_trace(trace_path, [], device_name)

    if not success:
        print("[check_image] Trace {} couldn't be replayed. "
              "See above logs for more information.".format(trace_path))
        return None, None, None
    else:
        test_path = path.join(path.dirname(trace_path), 'test', device_name)
        file_name = path.basename(trace_path)
        files = glob(path.join(test_path, file_name + '-*' + '.png'))
        assert(files)
        image_file = files[0]
        files = glob(path.join(test_path, file_name + '.log'))
        assert(files)
        log_file = files[0]
        return (hexdigest_from_image(image_file), image_file, log_file)


def check_trace(download_url, device_name, trace_path, expected_checksum):
    ensure_file(download_url, trace_path, TRACES_DB_PATH)

    result = {}
    result[trace_path] = {}
    result[trace_path]['expected'] = expected_checksum

    trace_dir = path.dirname(trace_path)
    dir_in_results = path.join(trace_dir, 'test', device_name)
    results_path = path.join(RESULTS_PATH, dir_in_results)
    os.makedirs(results_path, exist_ok=True)

    checksum, image_file, log_file = replay(path.join(TRACES_DB_PATH,
                                                      trace_path),
                                            device_name)

    result[trace_path]['actual'] = checksum or 'error'

    shutil.move(log_file, path.join(results_path, path.basename(log_file)))
    if checksum is None:
        return False, result
    if checksum == expected_checksum:
        if os.environ.get('TRACIE_STORE_IMAGES', '0') == '1':
            shutil.move(image_file, path.join(results_path, image_name))
        print('[check_image] Images match for:\n  {}\n'.format(trace_path))
        ok = True
    else:
        upload_file(image_file, 'image/png', device_name)
        print('[check_image] Images differ for '
              '%s (expected: %s, actual: %s)'.format(
              trace_path, expected_checksum, checksum))
        print('[check_image] For more information see '
              'https://gitlab.freedesktop.org/'
              'mesa/mesa/blob/master/.gitlab-ci/tracie/README.md')
        ok = False

    if not ok or os.environ.get('TRACIE_STORE_IMAGES', '0') == '1':
        result[trace_path]['image'] = image_file

    result[trace_path]['actual'] = checksum

    return ok, result


def write_results(results):
    os.makedirs(RESULTS_PATH, exist_ok=True)
    results_file_path = path.join(RESULTS_PATH, 'results.yml')
    with open(results_file_path, 'w') as f:
        yaml.safe_dump(results, f, default_flow_style=False)
    upload_file(results_file_path, 'text/yaml', device_name)


def from_yaml(yaml_file, device_name):
    y = qty.load_yaml(yaml_file)

    download_url = qty.download_url(y)

    all_ok = True
    results = {}
    t_list = qty.traces(y, device_name=device_name, checksum=True)
    for t in t_list:
        ok, result = check_trace(download_url,
                                 device_name,
                                 t['path'], t['checksum'])
        all_ok = all_ok and ok
        results.update(result)

    os.makedirs(RESULTS_PATH, exist_ok=True)
    with open(os.path.join(RESULTS_PATH, 'results.yml'), 'w') as f:
        yaml.safe_dump(results, f, default_flow_style=False)
    if os.environ.get('TRACIE_UPLOAD_TO_MINIO', '0') == '1':
        upload_artifact(os.path.join(RESULTS_PATH, 'results.yml'),
                        'text/yaml', device_name)

    return all_ok


def trace(download_url, device_name, trace_path, expected_checksum):
    ok, result = check_trace(download_url, device_name, trace_path,
                             expected_checksum)

    write_results(result)

    return ok
