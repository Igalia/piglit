# coding=utf-8
# Copyright (c) 2015-2016 Intel Corporation

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

"""Shared parsers for use in multiple modules.

Much of this module is based on taking advantage of ArgumentParser's parent
argument and it's parse_known_args() method. The idea is that some parts of
parsers can be shared to reduce code duplication, either by parsing the
argumetns early and acting on them, or by inheriting from a parent object.

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import argparse

from framework import core

# parse the config file before any other options, this allows the config file
# to be used to set default values for the parser.
CONFIG = argparse.ArgumentParser(add_help=False)
CONFIG.add_argument("-f", "--config",
                    dest="config_file",
                    type=argparse.FileType("r"),
                    help="override piglit.conf search path")


def parse_config(input_):
    """Convenience method for the CONFIG parser.

    This returns a two element tuple, the first element is a namespace with the
    known arguments in it (in this case the config_file), and the second is the
    remaining unparsed arguments. These remaining arguments should be passed to
    a new ArgumentParser instance.

    This will also call core.get_config on the config file. The parsed options
    are passed to ensure API compatibility

    """
    parsed, unparsed = CONFIG.parse_known_args(input_)

    # Read the config file
    # We want to read this before we finish parsing since some default options
    # are set in the config file.
    core.get_config(parsed.config_file)

    return parsed, unparsed
