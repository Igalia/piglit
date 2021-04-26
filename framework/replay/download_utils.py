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
import hashlib
import hmac
import requests
import xml.etree.ElementTree as ET

from os import path
from time import time
from email.utils import formatdate
from requests.utils import requote_uri

from framework import core, exceptions
from framework.replay.options import OPTIONS


__all__ = ['ensure_file']

minio_credentials = None

def sign_with_hmac(key, message):
    key = key.encode("UTF-8")
    message = message.encode("UTF-8")

    signature = hmac.new(key, message, hashlib.sha1).digest()

    return base64.encodebytes(signature).strip().decode()

def get_minio_credentials(url):
    global minio_credentials

    if minio_credentials is not None:
        return (minio_credentials['AccessKeyId'],
                minio_credentials['SecretAccessKey'],
                minio_credentials['SessionToken'])

    minio_credentials = {}

    params = {'Action': 'AssumeRoleWithWebIdentity',
              'Version': '2011-06-15',
              'RoleArn': 'arn:aws:iam::123456789012:role/FederatedWebIdentityRole',
              'RoleSessionName': OPTIONS.download['role_session_name'],
              'DurationSeconds': 3600,
              'WebIdentityToken': OPTIONS.download['jwt']}
    r = requests.post('https://%s' % OPTIONS.download['minio_host'], params=params)
    if r.status_code >= 400:
        print(r.text)
    r.raise_for_status()

    root = ET.fromstring(r.text)
    for attr in root.iter():
        if attr.tag == '{https://sts.amazonaws.com/doc/2011-06-15/}AccessKeyId':
            minio_credentials['AccessKeyId'] = attr.text
        elif attr.tag == '{https://sts.amazonaws.com/doc/2011-06-15/}SecretAccessKey':
            minio_credentials['SecretAccessKey'] = attr.text
        elif attr.tag == '{https://sts.amazonaws.com/doc/2011-06-15/}SessionToken':
            minio_credentials['SessionToken'] = attr.text

    return (minio_credentials['AccessKeyId'],
            minio_credentials['SecretAccessKey'],
            minio_credentials['SessionToken'])

def get_authorization_headers(url, resource):
    minio_key, minio_secret, minio_token = get_minio_credentials(url)

    content_type = 'application/octet-stream'
    date = formatdate(timeval=None, localtime=False, usegmt=True)
    to_sign = "GET\n\n\n%s\nx-amz-security-token:%s\n/%s/%s" % (date,
                                                                minio_token,
                                                                OPTIONS.download['minio_bucket'],
                                                                requote_uri(resource))
    signature = sign_with_hmac(minio_secret, to_sign)

    headers = {'Host': OPTIONS.download['minio_host'],
               'Date': date,
               'Authorization': 'AWS %s:%s' % (minio_key, signature),
               'x-amz-security-token': minio_token}
    return headers

def ensure_file(file_path):
    destination_file_path = path.join(OPTIONS.db_path, file_path)
    if OPTIONS.download['url'] is None:
        if not path.exists(destination_file_path):
            raise exceptions.PiglitFatalError(
                '{} missing'.format(destination_file_path))
        return

    url = OPTIONS.download['url'].geturl()

    core.check_dir(path.dirname(destination_file_path))

    if not OPTIONS.download['force'] and path.exists(destination_file_path):
        return

    print('[check_image] Downloading file {}'.format(
        file_path), end=' ', flush=True)

    if OPTIONS.download['minio_host']:
        assert OPTIONS.download['minio_bucket']
        assert OPTIONS.download['role_session_name']
        assert OPTIONS.download['jwt']
        headers = get_authorization_headers(url, file_path)
    else:
        headers = None

    download_time = time()
    with open(destination_file_path, 'wb') as file:
        with requests.get(url + file_path,
                          allow_redirects=True, stream=True,
                          headers=headers) as r:
            if r.status_code >= 400:
                print(r.text)
            r.raise_for_status()
            for chunk in r.iter_content(chunk_size=8194):
                if chunk:
                    file.write(chunk)
    print('took %ds.' % (time() - download_time), flush=True)
