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
try:
    import simplejson as json
except ImportError:
    import json

from enum import Enum, unique
from glob import glob
from os import path

from framework import core
from framework import status
from framework.replay import backends
from framework.replay import query_traces_yaml as qty
from framework.replay.download_utils import ensure_file
from framework.replay.image_checksum import hexdigest_from_image
from framework.replay.options import OPTIONS


__all__ = ['Result',
           'from_yaml',
           'trace']


@unique
class Result(Enum):
    FAILURE = -1
    MATCH = 0
    DIFFER = 1


def _replay(trace_path, results_path):
    try:
        success = backends.dump(trace_path, results_path, [])
    except (backends.DumpBackendNotImplementedError,
            backends.DumpBackendError) as e:
        print(e)
        success = False

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

    yaml_result = {}
    yaml_result[trace_path] = {}
    yaml_result[trace_path]['expected'] = expected_checksum

    trace_dir = path.dirname(trace_path)
    dir_in_results = path.join('trace', OPTIONS.device_name, trace_dir)
    results_path = path.join(OPTIONS.results_path, dir_in_results)
    core.check_dir(results_path)

    checksum, image_file = _replay(path.join(OPTIONS.db_path, trace_path),
                                   results_path)

    yaml_result[trace_path]['actual'] = checksum or 'error'
    print('[check_image]\n'
          '    actual: {}\n'
          '  expected: {}'.format(yaml_result[trace_path]['actual'],
                                  expected_checksum))

    if checksum is None:
        return Result.FAILURE, yaml_result
    if checksum == expected_checksum:
        if not OPTIONS.keep_image:
            os.remove(image_file)
        print('[check_image] Images match for:\n  {}\n'.format(trace_path))
        result = Result.MATCH
    else:
        print('[check_image] Images differ for:\n  {}'.format(trace_path))
        print('[check_image] For more information see '
              'https://gitlab.freedesktop.org/'
              'mesa/mesa/blob/master/.gitlab-ci/tracie/README.md')
        result = Result.DIFFER

    if result is not Result.MATCH or OPTIONS.keep_image:
        yaml_result[trace_path]['image'] = image_file

    yaml_result[trace_path]['actual'] = checksum

    return result, yaml_result


def _print_result(result, trace_path, yaml_result):
    output = 'PIGLIT: '
    json_result = {}
    if result is Result.FAILURE:
        json_result['result'] = str(status.CRASH)
    elif result is Result.DIFFER:
        json_result['result'] = str(status.FAIL)
        json_result['images'] = [
            {'image_desc': trace_path,
             'image_ref': yaml_result[trace_path]['expected'] + '.png',
             'image_render': yaml_result[trace_path]['image']}]
    else:
        json_result['result'] = str(status.PASS)

    output += json.dumps(json_result)
    print(output)


def _write_results(yaml_results):
    core.check_dir(OPTIONS.results_path)
    results_file_path = path.join(OPTIONS.results_path, 'results.yml')
    with open(results_file_path, 'w') as f:
        yaml.safe_dump(yaml_results, f, default_flow_style=False)


def from_yaml(yaml_file):
    y = qty.load_yaml(yaml_file)

    OPTIONS.set_download_url(qty.download_url(y))

    global_result = Result.MATCH
    yaml_results = {}
    t_list = qty.traces(y, device_name=OPTIONS.device_name, checksum=True)
    for t in t_list:
        result, yaml_result = _check_trace(t['path'], t['checksum'])
        if result is not Result.MATCH and global_result is not Result.FAILURE:
            global_result = result
        yaml_results.update(yaml_result)
        # TODO: print in subtest format
        # _print_result(result, t['path'], yaml_result)

    _write_results(yaml_results)

    return global_result


def trace(trace_path, expected_checksum):
    result, yaml_result = _check_trace(trace_path, expected_checksum)
    _print_result(result, trace_path, yaml_result)

    _write_results(yaml_result)

    return result
