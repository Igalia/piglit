# coding=utf-8
#
# Copyright (c) 2015-2016, 2019 Intel Corporation
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

"""Stores global replay options.

This is as close to a true global function as python gets.

"""

import os
import sys

from urllib.parse import urlparse

__all__ = ['OPTIONS']

# pylint: disable=too-few-public-methods


def _safe_urlparse(url):
    if url:
        try:
            parsed_url = urlparse(url)
            return parsed_url
        except Exception as e:
            print(e, file=sys.stderr)

    return None


class _Options(object):  # pylint: disable=too-many-instance-attributes
    """Contains all options for a replay run.

    This is used as a sort of global state object.

    Options are as follows:
    device_name -- The device against we are replaying and checking.
    keep_image -- Whether to always keep the dumped images or not.
    db_path -- The path to the objects db or where it will be created.
    results_path -- The path in which to place the results.
    download.url -- The URL from which to download the files.
    download.force -- Forces downloading even if the destination file already
                      exists.
    download.minio_host -- Name of MinIO server from which to download traces
    download.minio_bucket -- Name of bucket in MinIO server containing the traces
    download.role_session_name -- Role session name for authentication with MinIO
    download.jwt -- JWT token for authentication with MinIO
    """

    def __init__(self):
        self.device_name = None
        self.keep_image = False
        self.db_path = None
        self.results_path = None
        self.download = {'url': None,
                         'force': False,
                         'minio_host': '',
                         'minio_bucket': '',
                         'role_session_name': '',
                         'jwt': ''
        }

    def clear(self):
        """Reinitialize all values to defaults."""
        self.__init__()

    def set_download_url(self, url):
        """Safely set the parsed download url."""
        self.download['url'] = _safe_urlparse(url)

    def __iter__(self):
        for key, values in self.__dict__.items():
            if not key.startswith('_'):
                yield key, values


OPTIONS = _Options()
