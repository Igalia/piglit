# Copyright 2014 Intel Corporation
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


"""
Generate C source code from Khronos XML.
"""

from __future__ import print_function

import argparse
import mako.runtime
import mako.template
import os.path
import re
import sys

from collections import namedtuple
from textwrap import dedent

PIGLIT_TOP_DIR = os.path.join(os.path.dirname(__file__), '..', '..')
sys.path.append(PIGLIT_TOP_DIR)

import registry.gl
from registry.gl import OrderedKeyedSet, ImmutableOrderedKeyedSet


debug = False


def log_debug(msg):
    if debug:
        prog_name = os.path.basename(sys.argv[0])
        print('debug: {0}: {1}'.format(prog_name, msg), file=sys.stderr)


def main():
    global debug

    argparser = argparse.ArgumentParser()
    argparser.add_argument('-o', '--out-dir', required=True)
    argparser.add_argument('-d', '--debug', action='store_true', default=False)
    args = argparser.parse_args()

    debug = args.debug
    registry.gl.debug = debug

    gl_registry = registry.gl.parse()
    DispatchCode.emit(args.out_dir, gl_registry)


class DispatchCode(object):

    H_TEMPLATE = 'piglit-dispatch-gen.h.mako'
    C_TEMPLATE = 'piglit-dispatch-gen.c.mako'

    Api = namedtuple('DispatchApi',
                     ('name', 'base_version_int', 'c_piglit_token'))

    APIS = {
        'gl':    Api('gl',    10, 'PIGLIT_DISPATCH_GL'),
        'gles1': Api('gles1', 11, 'PIGLIT_DISPATCH_ES1'),
        'gles2': Api('gles2', 20, 'PIGLIT_DISPATCH_ES2'),
    }
    APIS['glcore'] = APIS['gl']

    assert(set(APIS.keys()) == set(registry.gl.VALID_APIS))

    @classmethod
    def emit(cls, out_dir, gl_registry):
        assert(isinstance(gl_registry, registry.gl.Registry))
        context_vars = dict(dispatch=cls, gl_registry=gl_registry)
        render_template(cls.H_TEMPLATE, out_dir, **context_vars)
        render_template(cls.C_TEMPLATE, out_dir, **context_vars)


def render_template(filename, out_dir, **context_vars):
    assert(filename.endswith('.mako'))
    template_filepath = os.path.join(os.path.dirname(__file__), filename)
    out_filepath = os.path.join(out_dir, os.path.splitext(filename)[0])

    warning = 'DO NOT EDIT! Script {0!r} generated this file from {1!r}'
    warning = warning.format(os.path.basename(__file__), filename)

    fake_alignment = re.compile(r'\.*\n\.+', flags=re.MULTILINE)
    fake_tab = re.compile(r'>-------')

    def fake_whitespace(proto_text):
        if debug:
            print('fake whitespace: before: {0!r}'.format(proto_text))
        text = unicode(proto_text)
        text = fake_alignment.sub('', text)
        text = fake_tab.sub('\t', text)
        if debug:
            print('fake whitespace:  after: {0!r}'.format(text))
        return text

    with open(out_filepath, 'w') as out_file:
        template = mako.template.Template(
            filename=template_filepath,
            strict_undefined=True)
        ctx = mako.runtime.Context(
            buffer=out_file,
            warning=warning,
            fake_whitespace=fake_whitespace,
            **context_vars)
        template.render_context(ctx)


if __name__ == '__main__':
    main()
