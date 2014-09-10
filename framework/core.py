
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# This permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR(S) BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

# Piglit core

from __future__ import print_function
import errno
import os
import re
import subprocess
import sys
# TODO: ConfigParser is known as configparser in python3
import ConfigParser

__all__ = [
    'PIGLIT_CONFIG',
    'PLATFORMS',
    'Options',
    'collect_system_info',
    'parse_listfile',
]


PLATFORMS = ["glx", "x11_egl", "wayland", "gbm", "mixed_glx_egl"]
PIGLIT_CONFIG = ConfigParser.SafeConfigParser(allow_no_value=True)

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


# Ensure the given directory exists
def checkDir(dirname, failifexists):
    exists = True
    try:
        os.stat(dirname)
    except OSError as e:
        if e.errno == errno.ENOENT or e.errno == errno.ENOTDIR:
            exists = False

    if exists and failifexists:
        print("%(dirname)s exists already.\nUse --overwrite if "
              "you want to overwrite it.\n" % locals(), file=sys.stderr)
        exit(1)

    try:
        os.makedirs(dirname)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise


class Options(object):
    """ Contains options for a piglit run

    Options are as follows:
    concurrent -- True if concurrency is to be used
    execute -- False for dry run
    filter -- list of compiled regex which include exclusively tests that match
    exclude_filter -- list of compiled regex which exclude tests that match
    valgrind -- True if valgrind is to be used
    dmesg -- True if dmesg checking is desired. This forces concurrency off
    env -- environment variables set for each test before run

    """
    def __init__(self, concurrent=True, execute=True, include_filter=None,
                 exclude_filter=None, valgrind=False, dmesg=False, sync=False):
        self.concurrent = concurrent
        self.execute = execute
        self.filter = [re.compile(x) for x in include_filter or []]
        self.exclude_filter = [re.compile(x) for x in exclude_filter or []]
        self.exclude_tests = set()
        self.valgrind = valgrind
        self.dmesg = dmesg
        self.sync = sync

        # env is used to set some base environment variables that are not going
        # to change across runs, without sending them to os.environ which is
        # fickle and easy to break
        self.env = {
            'PIGLIT_SOURCE_DIR': os.environ.get('PIGLIT_SOURCE_DIR',
                 os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
        }

    def __iter__(self):
        for key, values in self.__dict__.iteritems():
            # If the values are regex compiled then yield their pattern
            # attribute, which is the original plaintext they were compiled
            # from, otherwise yield them normally.
            if key in ['filter', 'exclude_filter']:
                yield (key, [x.pattern for x in values])
            else:
                yield (key, values)


def collect_system_info():
    """ Get relavent information about the system running piglit

    This method runs through a list of tuples, where element 1 is the name of
    the program being run, and elemnt 2 is a command to run (in a form accepted
    by subprocess.Popen)

    """
    progs = [('wglinfo', ['wglinfo']),
             ('glxinfo', ['glxinfo']),
             ('uname', ['uname', '-a']),
             ('lspci', ['lspci'])]

    result = {}

    for name, command in progs:
        try:
            result[name] = subprocess.check_output(command,
                                                   stderr=subprocess.STDOUT)
        except OSError as e:
            # If we get the 'no file or directory' error then pass, that means
            # that the binary isn't installed or isn't relavent to the system
            if e.errno != 2:
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
