# encoding=utf-8
# Copyright © 2016 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

"""Generate ARBvp tests.

Based on a bash generator that performs the same function written by Ian
Romanick.

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os

from templates import template_dir
from modules import utils

TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))


def main():
    """Main function. Generates tests."""
    dirname = os.path.join('asmparsertest', 'shaders', 'ARBvp1.0')
    utils.safe_makedirs(dirname)

    targets_1 = ["1D", "2D", "3D", "CUBE", "RECT", "SHADOW1D", "SHADOW2D"]

    # The ordering in this loop is wierd, it's an artifact of the growth of the
    # original script and is required to ensure that the names of the tests
    # don't change
    for inst in ["TEX", "TXB", "TXD", "TXF", "TXL", "TXP", "TXQ"]:
        i = 1
        template = TEMPLATES.get_template('arbvp.mako')
        for target in targets_1:
            fname = os.path.join(dirname,
                                 "{}-{:0>2d}.txt".format(inst.lower(), i))
            with open(fname, 'w') as f:
                f.write(template.render_unicode(target=target, inst=inst))
            print(fname)
            i += 1

        template = TEMPLATES.get_template('nvvp3.mako')
        for target in targets_1:
            fname = os.path.join(dirname,
                                 "{}-{:0>2d}.txt".format(inst.lower(), i))
            with open(fname, 'w') as f:
                f.write(template.render_unicode(target=target, inst=inst))
            print(fname)
            i += 1

        template = TEMPLATES.get_template('nvvp3_2.mako')
        for target in ["CUBE", "RECT"]:
            fname = os.path.join(dirname,
                                 "{}-{:0>2d}.txt".format(inst.lower(), i))
            with open(fname, 'w') as f:
                f.write(template.render_unicode(target=target, inst=inst))
            print(fname)
            i += 1

        template = TEMPLATES.get_template('nvvp3.mako')
        fname = os.path.join(dirname, "{}-{:0>2d}.txt".format(inst.lower(), i))
        with open(fname, 'w') as f:
            f.write(template.render_unicode(target="SHADOWRECT", inst=inst))
        print(fname)
        i += 1

        template = TEMPLATES.get_template('nvvp3_2.mako')
        for target in ["SHADOW1D", "SHADOW2D", "SHADOWRECT"]:
            fname = os.path.join(dirname,
                                 "{}-{:0>2d}.txt".format(inst.lower(), i))
            with open(fname, 'w') as f:
                f.write(template.render_unicode(target=target, inst=inst))
            print(fname)
            i += 1


if __name__ == '__main__':
    main()
