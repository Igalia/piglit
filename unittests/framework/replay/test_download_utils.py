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


"""Tests for replayer's download_utils module."""

import pytest

import os
import requests
import requests_mock
from urllib.parse import urlparse

from os import path

from framework import exceptions
from framework.replay import download_utils
from framework.replay.options import OPTIONS

ASSUME_ROLE_RESPONSE = '''<?xml version="1.0" encoding="UTF-8"?>
    <AssumeRoleWithWebIdentityResponse
        xmlns="https://sts.amazonaws.com/doc/2011-06-15/">
        <AssumeRoleWithWebIdentityResult>
            <Credentials>
                <AccessKeyId>Key</AccessKeyId>
                <SecretAccessKey>Secret</SecretAccessKey>
                <Expiration>2021-03-25T13:59:58Z</Expiration>
                <SessionToken>token</SessionToken>
            </Credentials>
        </AssumeRoleWithWebIdentityResult>
    </AssumeRoleWithWebIdentityResponse>
'''

class TestDownloadUtils(object):
    """Tests for download_utils methods."""

    @pytest.fixture(autouse=True)
    def setup(self, requests_mock, tmpdir):
        self.url = 'https://unittest.piglit.org/'
        self.trace_path = 'KhronosGroup-Vulkan-Tools/amd/polaris10/vkcube.gfxr'
        self.full_url = self.url + self.trace_path
        self.trace_file = tmpdir.join(self.trace_path)
        OPTIONS.set_download_url(self.url)
        OPTIONS.download['force'] = False
        OPTIONS.db_path = tmpdir.strpath
        requests_mock.get(self.full_url, text='remote')

    @staticmethod
    def check_same_file(path_local, expected_content, expected_mtime=None):
        assert path_local.read() == expected_content
        if expected_mtime is not None:
            m = path_local.mtime()
            assert m == expected_mtime

    def test_ensure_file_exists(self):
        """download_utils.ensure_file: Check an existing file doesn't get overwritten"""

        os.makedirs(path.dirname(self.trace_file), exist_ok=True)
        self.trace_file.write("local")
        m = self.trace_file.mtime()
        download_utils.ensure_file(self.trace_path)
        TestDownloadUtils.check_same_file(self.trace_file, "local", m)

    def test_ensure_file_not_exists(self):
        """download_utils.ensure_file: Check a non existing file gets downloaded"""

        assert not self.trace_file.check()
        download_utils.ensure_file(self.trace_path)
        TestDownloadUtils.check_same_file(self.trace_file, "remote")

    def test_ensure_file_exists_force_download(self):
        """download_utils.ensure_file: Check an existing file gets overwritten when forced"""

        OPTIONS.download['force'] = True
        os.makedirs(path.dirname(self.trace_file), exist_ok=True)
        self.trace_file.write("local")
        m = self.trace_file.mtime()
        download_utils.ensure_file(self.trace_path)
        TestDownloadUtils.check_same_file(self.trace_file, "remote")

    @pytest.mark.raises(exception=exceptions.PiglitFatalError)
    def test_ensure_file_not_exists_no_url(self):
        """download_utils.ensure_file: Check an exception raises when not passing an URL for a non existing file"""

        OPTIONS.set_download_url("")
        assert not self.trace_file.check()
        download_utils.ensure_file(self.trace_path)

    @pytest.mark.raises(exception=requests.exceptions.HTTPError)
    def test_ensure_file_not_exists_404(self, requests_mock):
        """download_utils.ensure_file: Check an exception raises when an URL returns a 404"""

        requests_mock.get(self.full_url, text='Not Found', status_code=404)
        assert not self.trace_file.check()
        download_utils.ensure_file(self.trace_path)

    @pytest.mark.raises(exception=requests.exceptions.ConnectTimeout)
    def test_ensure_file_not_exists_timeout(self, requests_mock):
        """download_utils.ensure_file: Check an exception raises when an URL returns a Connect Timeout"""

        requests_mock.get(self.full_url, exc=requests.exceptions.ConnectTimeout)
        assert not self.trace_file.check()
        download_utils.ensure_file(self.trace_path)

    def test_minio_authorization(self, requests_mock):
        """download_utils.ensure_file: Check we send the authentication headers to MinIO"""
        requests_mock.post(self.url, text=ASSUME_ROLE_RESPONSE)
        OPTIONS.download['minio_host'] = urlparse(self.url).netloc
        OPTIONS.download['minio_bucket'] = 'minio_bucket'
        OPTIONS.download['role_session_name'] = 'role_session_name'
        OPTIONS.download['jwt'] = 'jwt'

        assert not self.trace_file.check()
        download_utils.ensure_file(self.trace_path)
        TestDownloadUtils.check_same_file(self.trace_file, "remote")

        post_request = requests_mock.request_history[0]
        assert(post_request.method == 'POST')

        get_request = requests_mock.request_history[1]
        assert(get_request.method == 'GET')
        assert(requests_mock.request_history[1].headers['Authorization'].startswith('AWS Key'))
