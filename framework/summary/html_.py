# Copyright 2013-2016 Intel Corporation
# Copyright 2013, 2014 Advanced Micro Devices
# Copyright 2014 VMWare

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

"""Genrate html summaries."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import errno
import getpass
import os
import shutil
import sys
import tempfile
import traceback

import mako
from mako.lookup import TemplateLookup
import six

# a local variable status exists, prevent accidental overloading by renaming
# the module
from framework import backends, exceptions, core

from .common import Results, escape_filename, escape_pathname
from .feature import FeatResults

__all__ = [
    'html',
    'feat'
]

_TEMP_DIR = os.path.join(
    tempfile.gettempdir(),
    getpass.getuser(),
    'python-{}'.format(sys.version.split()[0]),
    'mako-{}'.format(mako.__version__),
    'summary',
    'html')

_TEMPLATE_DIR = os.path.join(os.path.dirname(__file__), '../..', 'templates')

# To ease the bytes/str/uincode between python 2 and python 3 the
# output_encoding keyword is set below. This means that in both python 2 and 3
# bytes are returned. This means that the files need to be opened in bytes mode
# ('wb').
_TEMPLATES = TemplateLookup(
    _TEMPLATE_DIR,
    output_encoding='utf-8',
    module_directory=os.path.join(_TEMP_DIR, "html-summary"))


def _copy_static_files(destination):
    """Copy static files into the results directory."""
    shutil.copy(os.path.join(_TEMPLATE_DIR, "index.css"),
                os.path.join(destination, "index.css"))
    shutil.copy(os.path.join(_TEMPLATE_DIR, "result.css"),
                os.path.join(destination, "result.css"))


def _make_testrun_info(results, destination, exclude=None):
    """Create the pages for each results file."""
    exclude = exclude or {}
    result_css = os.path.join(destination, "result.css")
    index = os.path.join(destination, "index.html")

    for each in results.results:
        name = escape_pathname(each.name)
        try:
            core.check_dir(os.path.join(destination, name), True)
        except exceptions.PiglitException:
            raise exceptions.PiglitFatalError(
                'Two or more of your results have the same "name" '
                'attribute. Try changing one or more of the "name" '
                'values in your json files.\n'
                'Duplicate value: {}'.format(name))

        with open(os.path.join(destination, name, "index.html"), 'wb') as out:
            out.write(_TEMPLATES.get_template('testrun_info.mako').render(
                name=each.name,
                totals=each.totals['root'],
                time=each.time_elapsed.delta,
                options=each.options,
                uname=each.uname,
                glxinfo=each.glxinfo,
                clinfo=each.clinfo,
                lspci=each.lspci))

        # Then build the individual test results
        for key, value in six.iteritems(each.tests):
            html_path = os.path.join(destination, name,
                                     escape_filename(key + ".html"))
            temp_path = os.path.dirname(html_path)

            if value.result not in exclude:
                core.check_dir(temp_path)

                try:
                    with open(html_path, 'wb') as out:
                        out.write(_TEMPLATES.get_template(
                            'test_result.mako').render(
                                testname=key,
                                value=value,
                                css=os.path.relpath(result_css, temp_path),
                                index=os.path.relpath(index, temp_path)))
                except OSError as e:
                    traceback.print_exc()


def _make_comparison_pages(results, destination, exclude):
    """Create the pages of comparisons."""
    pages = frozenset(['changes', 'problems', 'skips', 'fixes',
                       'regressions', 'enabled', 'disabled'])

    # Index.html is a bit of a special case since there is index, all, and
    # alltests, where the other pages all use the same name. ie,
    # changes.html, changes, and page=changes.
    with open(os.path.join(destination, "index.html"), 'wb') as out:
        out.write(_TEMPLATES.get_template('index.mako').render(
            results=results,
            page='all',
            pages=pages,
            exclude=exclude))

    # Generate the rest of the pages
    for page in pages:
        with open(os.path.join(destination, page + '.html'), 'wb') as out:
            # If there is information to display display it
            if sum(getattr(results.counts, page)) > 0:
                out.write(_TEMPLATES.get_template('index.mako').render(
                    results=results,
                    pages=pages,
                    page=page,
                    exclude=exclude))
            # otherwise provide an empty page
            else:
                out.write(
                    _TEMPLATES.get_template('empty_status.mako').render(
                        page=page, pages=pages))


def _make_feature_info(results, destination):
    """Create the feature readiness page."""

    with open(os.path.join(destination, "feature.html"), 'wb') as out:
        out.write(_TEMPLATES.get_template('feature.mako').render(
            results=results))


def html(results, destination, exclude):
    """
    Produce HTML summaries.

    Basically all this does is takes the information provided by the
    constructor, and passes it to mako templates to generate HTML files.
    The beauty of this approach is that mako is leveraged to do the
    heavy lifting, this method just passes it a bunch of dicts and lists
    of dicts, which mako turns into pretty HTML.
    """
    results = Results([backends.load(i) for i in results])

    _copy_static_files(destination)
    _make_testrun_info(results, destination, exclude)
    _make_comparison_pages(results, destination, exclude)


def feat(results, destination, feat_desc):
    """Produce HTML feature readiness summary."""

    feat_res = FeatResults([backends.load(i) for i in results], feat_desc)

    _copy_static_files(destination)
    _make_testrun_info(feat_res, destination)
    _make_feature_info(feat_res, destination)
