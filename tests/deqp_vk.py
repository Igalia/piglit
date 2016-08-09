# Copyright 2014-2016 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

"""Piglit integrations for the official Khronos Vulkan CTS.

upstream: https://github.com/KhronosGroup/Vulkan-CTS

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import re

from framework.test import deqp

__all__ = ['profile']

# Path to the deqp-gles3 executable.
_DEQP_VK_BIN = deqp.get_option('PIGLIT_DEQP_VK_BIN',
                               ('deqp-vk', 'bin'),
                               required=True)

_EXTRA_ARGS = deqp.get_option('PIGLIT_DEQP_VK_EXTRA_ARGS',
                              ('deqp-vk', 'extra_args'),
                              default='').split()

_DEQP_ASSERT = re.compile(
    r'deqp-vk: external/vulkancts/.*: Assertion `.*\' failed.')


class _VulkanMixin(object):
    """Mixin class for vulkan specific bits."""

    deqp_bin = _DEQP_VK_BIN
    @property
    def extra_args(self):
        return super(_VulkanMixin, self).extra_args + \
            [x for x in _EXTRA_ARGS if not x.startswith('--deqp-case')]

    def interpret_result(self):
        if 'Failed to compile shader at vkGlslToSpirV' in self.result.out:
            self.result.result = 'skip'
            self.result.out += \
                '\n\nMarked as skip because GLSLang failed to compile shaders'
        elif _DEQP_ASSERT.search(self.result.err):
            self.result.result = 'skip'
            self.result.out += \
                '\n\nMarked as skip because of a internal dEQP assertion'
        else:
            super(_VulkanMixin, self).interpret_result()


class DEQPVKSingleTest(_VulkanMixin, deqp.DEQPSingleTest):
    """Test representation for Khronos Vulkan CTS."""
    timeout = 60


class DEQPVKGroupTest(_VulkanMixin, deqp.DEQPGroupAsteriskTest):
    """Test representation for Khronos Vulkan CTS."""
    timeout = 240

    def _is_cherry(self):
        """If there's a resource error mark it as crash and start rerunning.

        There are two reasons for not trying this test again. First, there's
        no easy way to track that we're trying the test again, and without
        tracking we can get into an infinite loop. Second this is either a
        driver bug or a test suite bug and we don't want to paper over that.
        """
        return (self.result.returncode == 0 and
                'VK_ERROR_OUT_OF_DEVICE_MEMORY' not in self.result.out)


profile = deqp.make_profile(  # pylint: disable=invalid-name
    deqp.iter_deqp_test_cases(
        deqp.gen_caselist_txt(_DEQP_VK_BIN, 'dEQP-VK-cases.txt',
                              _EXTRA_ARGS)),
    single=DEQPVKSingleTest, group=DEQPVKGroupTest)
