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
import time


def ensure_file(project_url, file_path, destination):
    destination_file_path = destination + file_path
    if project_url is None:
        assert os.path.exists(destination_file_path), (
            "{} missing".format(destination_file_path))
        return

    os.makedirs(os.path.dirname(destination_file_path), exist_ok=True)

    if os.path.exists(destination_file_path):
        return

    print("[check_image] Downloading trace %s"
          % (trace['path']), end=" ", flush=True)
    download_time = time.time()
    r = requests.get(project_url + trace['path'])
    open(trace_path, "wb").write(r.content)
    print("took %ds." % (time.time() - download_time), flush=True)
