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
import requests

from os import path
from time import time


__all__ = ['ensure_file']


def ensure_file(download_url, file_path, destination):
    destination_file_path = path.join(destination, file_path)
    if download_url is None:
        assert path.exists(destination_file_path), (
            '{} missing'.format(destination_file_path))
        return

    os.makedirs(path.dirname(destination_file_path), exist_ok=True)

    if path.exists(destination_file_path):
        return

    print('[check_image] Downloading file {}'.format(
        file_path), end=' ', flush=True)
    download_time = time()
    with open(destination_file_path, 'wb') as file:
        with requests.get(download_url + file_path,
                          allow_redirects=True, stream=True) as r:
            r.raise_for_status()
            for chunk in r.iter_content(chunk_size=8194):
                if chunk:
                    file.write(chunk)
    print('took %ds.' % (time() - download_time), flush=True)
