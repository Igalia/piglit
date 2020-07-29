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
import yaml

from glob import glob
from os import path

from framework import core
from framework import status
from framework.replay import query_traces_yaml as qty
from framework.replay.download_utils import ensure_file
from framework.replay.dump_trace_images import dump_from_trace
from framework.replay.image_checksum import hexdigest_from_image
from framework.replay.options import OPTIONS


__all__ = ['from_yaml',
           'trace']


def _replay(trace_path, results_path):
    success = dump_from_trace(trace_path, results_path, [])

    if not success:
        print("[check_image] Trace {} couldn't be replayed. "
              "See above logs for more information.".format(trace_path))
        return None, None
    else:
        file_name = path.basename(trace_path)
        files = glob(path.join(results_path, file_name + '-*' + '.png'))
        assert(files)
        image_file = files[0]
        return hexdigest_from_image(image_file), image_file


def _check_trace(trace_path, expected_checksum):
    ensure_file(trace_path)

    result = {}
    result[trace_path] = {}
    result[trace_path]['expected'] = expected_checksum

    trace_dir = path.dirname(trace_path)
    dir_in_results = path.join('trace', OPTIONS.device_name, trace_dir)
    results_path = path.join(OPTIONS.results_path, dir_in_results)
    core.check_dir(results_path)

    checksum, image_file = _replay(path.join(OPTIONS.db_path, trace_path),
                                   results_path)

    result[trace_path]['actual'] = checksum or 'error'
    print('[check_image]\n'
          '    actual: {}\n'
          '  expected: {}'.format(result[trace_path]['actual'],
                                  expected_checksum))

    if checksum is None:
        return False, result
    if checksum == expected_checksum:
        if not OPTIONS.keep_image:
            os.remove(image_file)
        print('[check_image] Images match for:\n  {}\n'.format(trace_path))
        ok = True
    else:
        print('[check_image] Images differ for:\n  {}'.format(trace_path))
        print('[check_image] For more information see '
              'https://gitlab.freedesktop.org/'
              'mesa/mesa/blob/master/.gitlab-ci/tracie/README.md')
        ok = False

    if not ok or OPTIONS.keep_image:
        result[trace_path]['image'] = image_file

    result[trace_path]['actual'] = checksum

    return ok, result


def _write_results(results):
    core.check_dir(OPTIONS.results_path)
    results_file_path = path.join(OPTIONS.results_path, 'results.yml')
    with open(results_file_path, 'w') as f:
        yaml.safe_dump(results, f, default_flow_style=False)


def from_yaml(yaml_file):
    y = qty.load_yaml(yaml_file)

    OPTIONS.set_download_url(qty.download_url(y))

    all_ok = True
    results = {}
    t_list = qty.traces(y, device_name=OPTIONS.device_name, checksum=True)
    for t in t_list:
        ok, result = _check_trace(t['path'], t['checksum'])
        all_ok = all_ok and ok
        results.update(result)

    _write_results(results)

    return all_ok


def trace(trace_path, expected_checksum):
    ok, result = _check_trace(trace_path, expected_checksum)

    _write_results(result)

    return ok
