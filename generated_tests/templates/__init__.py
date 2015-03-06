# Copyright (c) 2014 Intel Corporation

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

"""Provides a few simple helpers for working with mako templates."""

from __future__ import absolute_import
import os
import sys
import getpass
import tempfile

from mako.template import Template
from mako.lookup import TemplateLookup


# Based on a similar setup in framework/summary
MAKO_TEMP_DIR = os.path.join(tempfile.gettempdir(),
                             'piglit-{0}'.format(getpass.getuser()),
                             'version-{0}'.format(sys.version.split()[0]),
                             'generators', 'templates')

TEMPLATE_DIR = os.path.abspath(os.path.dirname(__file__))

# Future functions to be used in all templates
_FUTURES = ['division']


def template_file(generator, template):
    """Get a single template file from the templates directory.

    If the generator uses more than one template use template_dir instead.

    Arguments:
    generator -- the name of the generator, which coresponds to the subdir of
                 the templates folder.
    template -- the name of the template to get. If getting a single template
                it's probably template.*.mako, where * is shader_test,
                glsl_parser_test, vert, frag, geom, etc.

    """
    assert os.path.sep not in generator, \
        'generator should be a filename, not a path'
    assert not generator.endswith('.py'), \
        'generator should not have a file extension'

    return Template(filename=os.path.join(TEMPLATE_DIR, generator, template),
                    module_directory=os.path.join(MAKO_TEMP_DIR, generator),
                    future_imports=_FUTURES,
                    output_encoding='utf-8')


def template_dir(generator):
    """Get a TemplateLookup object for a generator.

    This is useful if a generator uses more than one template.

    Arguments:
    generator -- the name of the generator, which coresponds to the subdir of
                 the templates folder.

    """
    assert os.path.sep not in generator, \
        'generator should be a filename, not a path'
    assert not generator.endswith('.py'), \
        'generator should not have a file extension'

    return TemplateLookup(
        directories=[os.path.join(TEMPLATE_DIR, generator)],
        module_directory=os.path.join(MAKO_TEMP_DIR, generator),
        future_imports=_FUTURES,
        output_encoding='utf-8')
