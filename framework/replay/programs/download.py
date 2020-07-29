# coding=utf-8
#
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

from framework import exceptions
from framework.replay import download_utils
from framework.replay import options
from . import parsers


__all__ = ['download']


def _ensure_file(args):
    options.OPTIONS.set_download_url(args.download_url)
    options.OPTIONS.download['force'] = args.force_download
    options.OPTIONS.db_path = args.db_path

    return download_utils.ensure_file(args.file_path)


@exceptions.handler
def download(input_):
    """ Parser for replayer download command """
    parser = argparse.ArgumentParser(parents=[parsers.DOWNLOAD_URL,
                                              parsers.DOWNLOAD_FORCE,
                                              parsers.DB_PATH])
    parser.add_argument(
        'file_path',
        help=('the path to the file '
              'at the provided URL from which to download'))
    parser.set_defaults(func=_ensure_file)

    args = parser.parse_args(input_)

    args.func(args)
