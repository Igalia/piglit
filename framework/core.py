# Copyright (c) 2014-2016 Intel Corporation

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

"""A collection of various bits that don't seem to belong anywhere else.

This is the classic catchall "utils" module from most projects, that for
historically reasons is called "core" in piglit.

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import errno
import os
import subprocess

from six.moves import configparser

from framework import exceptions

__all__ = [
    'PIGLIT_CONFIG',
    'PLATFORMS',
    'PiglitConfig',
    'collect_system_info',
    'parse_listfile',
]

PLATFORMS = ["glx", "x11_egl", "wayland", "gbm", "mixed_glx_egl", "wgl"]


class PiglitConfig(configparser.SafeConfigParser):
    """Custom Config parser that provides a few extra helpers."""
    def __init__(self, *args, **kwargs):
        # In Python2 the ConfigParser classes are old style, you can't use
        # super() on them. sigh
        configparser.SafeConfigParser.__init__(self, *args, **kwargs)
        self.filename = None

    def readfp(self, fp, filename=None):
        # In Python2 the ConfigParser classes are old style, you can't use
        # super() on them. sigh
        configparser.SafeConfigParser.readfp(self, fp, filename)
        self.filename = os.path.abspath(filename or fp.name)

    def safe_get(self, section, option, fallback=None, **kwargs):
        """A version of self.get that doesn't raise NoSectionError or
        NoOptionError.

        This is equivalent to passing if the option isn't found. It will return
        None if an error is caught

        """
        try:
            return self.get(section, option, **kwargs)
        except (configparser.NoOptionError, configparser.NoSectionError):
            return fallback

    def required_get(self, section, option, **kwargs):
        """A version fo self.get that raises PiglitFatalError.

        If self.get returns NoSectionError or NoOptionError then this will
        raise a PiglitFatalException, aborting the program.

        """
        try:
            return self.get(section, option, **kwargs)
        except configparser.NoSectionError:
            raise exceptions.PiglitFatalError(
                'No Section "{}" in file "{}".\n'
                'This section is required.'.format(
                    section, self.filename))
        except configparser.NoOptionError:
            raise exceptions.PiglitFatalError(
                'No option "{}"  from section "{}" in file "{}".\n'
                'This option is required.'.format(
                    option, section, self.filename))


PIGLIT_CONFIG = PiglitConfig(allow_no_value=True)


def get_config(arg=None):
    if arg:
        PIGLIT_CONFIG.readfp(arg)
    else:
        # Load the piglit.conf. First try looking in the current directory,
        # then trying the XDG_CONFIG_HOME, then $HOME/.config/, finally try the
        # piglit root dir
        for d in ['.',
                  os.environ.get('XDG_CONFIG_HOME',
                                 os.path.expandvars('$HOME/.config')),
                  os.path.join(os.path.dirname(__file__), '..')]:
            try:
                with open(os.path.join(d, 'piglit.conf'), 'r') as f:
                    PIGLIT_CONFIG.readfp(f)
                break
            except IOError:
                pass


def check_dir(dirname, failifexists=False, handler=None):
    """Check for the existence of a directory and create it if possible.

    This function will check for the existance of a directory. If that
    directory doesn't exist it will try to create it. If the directory does
    exist than it does one of two things.
    1) If "failifexists" is False (default): it will just return
    2) If "failifexists" is True it will raise an PiglitException, it is the
    job of the caller using failifexists=True to handle this exception

    Both failifexists and handler can be passed, but failifexists will have
    precedence.

    Arguments:
    dirname -- the name of the directory to check

    Keyword Arguments:
    failifexists -- If True and the directory exists then PiglitException will
                    be raised (default: False)
    handler -- a callable that is passed dirname if the thing to check exists.

    """
    try:
        os.stat(dirname)
    except OSError as e:
        # If the error is not "no file or directory" or "not a dir", then
        # either raise an exception, call the handler function, or return
        if e.errno not in [errno.ENOENT, errno.ENOTDIR]:
            if failifexists:
                raise exceptions.PiglitException
            elif handler is not None:
                handler(dirname)

    try:
        # makedirs is expensive, so check before # calling it.
        if not os.path.exists(dirname):
            os.makedirs(dirname)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise


def collect_system_info():
    """ Get relavent information about the system running piglit

    This method runs through a list of tuples, where element 1 is the name of
    the program being run, and elemnt 2 is a command to run (in a form accepted
    by subprocess.Popen)

    """
    progs = [('wglinfo', ['wglinfo']),
             ('glxinfo', ['glxinfo']),
             ('uname', ['uname', '-a']),
             ('clinfo', ['clinfo']),
             ('lspci', ['lspci', '-nn'])]

    result = {}

    for name, command in progs:
        try:
            out = subprocess.check_output(command, stderr=subprocess.STDOUT)
            result[name] = out.decode('utf-8')
        except OSError as e:
            # If we get the 'no file or directory' error then pass, that means
            # that the binary isn't installed or isn't relavent to the system.
            # If it's any other OSError, then raise
            if e.errno != errno.ENOENT:
                raise
        except subprocess.CalledProcessError:
            # If the binary is installed by doesn't work on the window system
            # (glxinfo) it will raise this error. go on
            pass

    return result


def parse_listfile(filename):
    """
    Parses a newline-seperated list in a text file and returns a python list
    object. It will expand tildes on Unix-like system to the users home
    directory.

    ex file.txt:
        ~/tests1
        ~/tests2/main
        /tmp/test3

    returns:
        ['/home/user/tests1', '/home/users/tests2/main', '/tmp/test3']
    """
    with open(filename, 'r') as file:
        return [os.path.expanduser(i.strip()) for i in file.readlines()]


class lazy_property(object):  # pylint: disable=invalid-name,too-few-public-methods
    """Descriptor that replaces the function it wraps with the value generated.

    This makes a property that is truly lazy, it is calculated once on demand,
    and stored. This makes this very useful for values that you might want to
    calculate and reuse, but they cannot change.

    This works by very cleverly shadowing itself with the calculated value. It
    adds the value to the instance, which pushes itself up the MRO and will
    never be quired again.

    """
    def __init__(self, func):
        self.__func = func

    def __get__(self, instance, cls):
        value = self.__func(instance)
        setattr(instance, self.__func.__name__, value)
        return value
