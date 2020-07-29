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

import base64
import hmac
import os
import requests

from email.utils import formatdate
from hashlib import sha1
from os import path
from urllib.parse import urljoin, urlparse
try:
    import simplejson as json
except ImportError:
    import json


__all__ = ['upload_file']


def _sign_with_hmac(key, message):
    key = key.encode('UTF-8')
    message = message.encode('UTF-8')

    signature = hmac.new(key, message, sha1).digest()

    return base64.encodebytes(signature).strip().decode()


def upload_file(file_path, content_type, device_name):
    if os.environ.get('TRACIE_UPLOAD_TO_MINIO', '0') != '1':
        return

    resource = ('/artifacts/%s/%s/%s/%s'
                % (os.environ['CI_PROJECT_PATH'], os.environ['CI_PIPELINE_ID'],
                   device_name, path.basename(file_path)))
    date = formatdate(timeval=None, localtime=False, usegmt=True)
    url = 'https://minio-packet.freedesktop.org%s' % (resource)
    headers = {'Host': 'minio-packet.freedesktop.org',
               'Date': date,
               'Content-Type': content_type}

    with open('.minio_credentials', 'r') as f:
        credentials = json.load(f)["minio-packet.freedesktop.org"]
        minio_key = credentials['AccessKeyId']
        minio_secret = credentials['SecretAccessKey']
        minio_token = credentials['SessionToken']
        to_sign = 'PUT\n\n{}\n{}\nx-amz-security-token:{}\n{}'.format(
            content_type, date, minio_token, url.path)
        signature = _sign_with_hmac(minio_secret, to_sign)
        headers.update(
            {'Authorization': 'AWS {}:{}'.format(minio_key, signature),
             'x-amz-security-token': minio_token})

    with open(file_path, 'rb') as data:
        print('Uploading file to {}'.format(url.geturl()))
        r = requests.put(url.geturl(), headers=headers, data=data)
        #print(r.text)
        r.raise_for_status()
