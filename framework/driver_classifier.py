# coding=utf-8
# Copyright (c) 2016 Broadcom
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import errno
import re
import subprocess

__all__ = [
    'DriverClassifier',
]

class DriverClassifier(object):
    def __init__(self):
        self.categories = []

        self.collect_driver_info()
        self.find_categories()


    def collect_driver_info(self):
        """Method for collecting the GL driver renderer string.

        Currently only glxinfo is used.
        """
        self.renderer = ''

        self.collect_glxinfo()


    def collect_glxinfo(self):
        """Calls glxinfo and parses the output to find the GL driver

        vendor/renderer strings.
        """

        try:
            output = subprocess.check_output(
                ['glxinfo'], stderr=subprocess.STDOUT).decode('utf-8')
        except OSError as e:
            if e.errno not in [errno.ENOENT, errno.EACCES]:
                raise
            return
        except subprocess.CalledProcessError:
            return

        for line in output.splitlines():
            m = re.match('OpenGL renderer string: (.*)', line)
            if m is not None:
                self.renderer = m.group(1)
                break


    def find_categories(self):
        """Parses the vendor/renderer strings to decide what categories

        the driver falls under.
        """
        if self.renderer.startswith(('Mesa ', 'Gallium ')):
            self.categories.append('mesa')

            m = re.match('.* VC4(.*)', self.renderer)
            if m is not None:
                self.categories.append('vc4')
                m = re.match(' V3D ([0-9])+\.([0-9])+', m.group(1))
                if m is not None:
                    self.categories.append('vc4-{}.{}'.format(m.group(1),
                                                              m.group(2)))

            m = re.match('Mesa DRI R200 ', self.renderer)
            if m is not None:
                self.categories.append('r200')

            m = re.match('Mesa DRI Intel[^ ]* (.*)', self.renderer)
            if m is not None:
                tail = m.group(1)

                i965_chipdict = {
                    '965': 'brw',
                    '946': 'brw',
                    '.*[GQ]4[35]': 'g4x',
                    'Ironlake': 'ilk',
                    'Sandybridge': 'snb',
                    'Ivybridge': 'ivb',
                    'Haswell': 'hsw',
                    'Baytrail': 'byt',
                    'Broadwell': 'bdw',
                    'Skylake': 'skl',
                    'HD Graphics .* \(Skylake': 'skl',
                    'Kabylake': 'kbl',
                    '.*Cherryview': 'chv',
                    '.*Broxton': 'bxt',
                }

                for chip, abbrev in i965_chipdict.items():
                    m = re.match(chip, tail)
                    if m is not None:
                        self.categories.append('i965')
                        self.categories.append('i965-{}'.format(abbrev))
                        break

        self.categories.reverse()
