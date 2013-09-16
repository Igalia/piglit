#
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

import errno
import os
import subprocess
import shlex
import types
import re

from core import Test, testBinDir, TestResult


# Platform global variables
if 'PIGLIT_PLATFORM' in os.environ:
    PIGLIT_PLATFORM = os.environ['PIGLIT_PLATFORM']
else:
    PIGLIT_PLATFORM = ''


def read_dmesg():
    proc = subprocess.Popen('dmesg', stdout=subprocess.PIPE)
    return proc.communicate()[0].rstrip('\n')

def get_dmesg_diff(old, new):
    # Note that dmesg is a ring buffer, i.e. lines at the beginning may
    # be removed when new lines are added.

    # Get the last dmesg timestamp from the old dmesg as string.
    last = old.split('\n')[-1]
    ts = last[:last.find(']')+1]
    if ts == '':
        return ''

    # Find the last occurence of the timestamp.
    pos = new.find(ts)
    if pos == -1:
        return new # dmesg was completely overwritten by new messages

    while pos != -1:
        start = pos
        pos = new.find(ts, pos+len(ts))

    # Find the next line and return the rest of the string.
    nl = new.find('\n', start+len(ts))
    return new[nl+1:] if nl != -1 else ''


# ExecTest: A shared base class for tests that simply runs an executable.
class ExecTest(Test):
    def __init__(self, command):
        Test.__init__(self)
        self.command = command
        self.split_command = os.path.split(self.command[0])[1]
        self.env = {}

        if isinstance(self.command, basestring):
            self.command = shlex.split(str(self.command))

        self.skip_test = self.check_for_skip_scenario(command)

    def interpretResult(self, out, returncode, results, dmesg):
        raise NotImplementedError
        return out

    def run(self, env):
        """
        Run a test.  The return value will be a dictionary with keys
        including 'result', 'info', 'returncode' and 'command'.
        * For 'result', the value may be one of 'pass', 'fail', 'skip',
          'crash', or 'warn'.
        * For 'info', the value will include stderr/out text.
        * For 'returncode', the value will be the numeric exit code/value.
        * For 'command', the value will be command line program and arguments.
        """
        fullenv = os.environ.copy()
        for e in self.env:
            fullenv[e] = str(self.env[e])

        if self.command is not None:
            command = self.command

            if env.valgrind:
                command[:0] = ['valgrind', '--quiet', '--error-exitcode=1',
                               '--tool=memcheck']

            i = 0
            dmesg_diff = ''
            while True:
                if self.skip_test:
                    out = "PIGLIT: {'result': 'skip'}\n"
                    err = ""
                    returncode = None
                else:
                    if env.dmesg:
                        old_dmesg = read_dmesg()
                    (out, err, returncode) = \
                        self.get_command_result(command, fullenv)
                    if env.dmesg:
                        dmesg_diff = get_dmesg_diff(old_dmesg, read_dmesg())

                # https://bugzilla.gnome.org/show_bug.cgi?id=680214 is
                # affecting many developers.  If we catch it
                # happening, try just re-running the test.
                if out.find("Got spurious window resize") >= 0:
                    i = i + 1
                    if i >= 5:
                        break
                else:
                    break

            # proc.communicate() returns 8-bit strings, but we need
            # unicode strings.  In Python 2.x, this is because we
            # will eventually be serializing the strings as JSON,
            # and the JSON library expects unicode.  In Python 3.x,
            # this is because all string operations require
            # unicode.  So translate the strings into unicode,
            # assuming they are using UTF-8 encoding.
            #
            # If the subprocess output wasn't properly UTF-8
            # encoded, we don't want to raise an exception, so
            # translate the strings using 'replace' mode, which
            # replaces erroneous charcters with the Unicode
            # "replacement character" (a white question mark inside
            # a black diamond).
            out = out.decode('utf-8', 'replace')
            err = err.decode('utf-8', 'replace')

            results = TestResult()

            if self.skip_test:
                results['result'] = 'skip'
            else:
                results['result'] = 'fail'
                out = self.interpretResult(out, returncode, results, dmesg_diff)

            crash_codes = [
                # Unix: terminated by a signal
                -5,   # SIGTRAP
                -6,   # SIGABRT
                -8,   # SIGFPE  (Floating point exception)
                -10,  # SIGUSR1
                -11,  # SIGSEGV (Segmentation fault)
                # Windows:
                # EXCEPTION_ACCESS_VIOLATION (0xc0000005):
                -1073741819,
                # EXCEPTION_INT_DIVIDE_BY_ZERO (0xc0000094):
                -1073741676
            ]

            if returncode in crash_codes:
                results['result'] = 'crash'
            elif returncode != 0:
                results['note'] = 'Returncode was {0}'.format(returncode)

            if env.valgrind:
                # If the underlying test failed, simply report
                # 'skip' for this valgrind test.
                if results['result'] != 'pass':
                    results['result'] = 'skip'
                elif returncode == 0:
                    # Test passes and is valgrind clean.
                    results['result'] = 'pass'
                else:
                    # Test passed but has valgrind errors.
                    results['result'] = 'fail'

            env = ''
            for key in self.env:
                env = env + key + '="' + self.env[key] + '" '
            if env:
                results['environment'] = env

            results['info'] = unicode("Returncode: {0}\n\nErrors:\n{1}\n\n"
                                      "Output:\n{2}").format(returncode,
                                                             err, out)
            results['returncode'] = returncode
            results['command'] = ' '.join(self.command)
            results['dmesg'] = dmesg_diff

            self.handleErr(results, err)

        else:
            results = TestResult()
            if 'result' not in results:
                results['result'] = 'skip'

        return results

    def check_for_skip_scenario(self, command):
        global PIGLIT_PLATFORM
        if PIGLIT_PLATFORM == 'gbm':
            if 'glean' == self.split_command:
                return True
            if self.split_command.startswith('glx-'):
                return True
        return False

    def get_command_result(self, command, fullenv):
        try:
            proc = subprocess.Popen(command,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE,
                                    env=fullenv,
                                    universal_newlines=True)
            out, err = proc.communicate()
            returncode = proc.returncode
        except OSError as e:
            # Different sets of tests get built under
            # different build configurations.  If
            # a developer chooses to not build a test,
            # Piglit should not report that test as having
            # failed.
            if e.errno == errno.ENOENT:
                out = "PIGLIT: {'result': 'skip'}\n" \
                    + "Test executable not found.\n"
                err = ""
                returncode = None
            else:
                raise e
        return out, err, returncode


class PlainExecTest(ExecTest):
    """
    PlainExecTest: Run a "native" piglit test executable

    Expect one line prefixed PIGLIT: in the output, which contains a result
    dictionary. The plain output is appended to this dictionary
    """
    def __init__(self, command):
        ExecTest.__init__(self, command)
        # Prepend testBinDir to the path.
        self.command[0] = testBinDir + self.command[0]

    def interpretResult(self, out, returncode, results, dmesg):
        outlines = out.split('\n')
        outpiglit = map(lambda s: s[7:],
                        filter(lambda s: s.startswith('PIGLIT:'), outlines))

        if dmesg != '':
            outpiglit = map(lambda s: s.replace("'pass'", "'dmesg-warn'"), outpiglit)
            outpiglit = map(lambda s: s.replace("'warn'", "'dmesg-warn'"), outpiglit)
            outpiglit = map(lambda s: s.replace("'fail'", "'dmesg-fail'"), outpiglit)

        if len(outpiglit) > 0:
            try:
                for piglit in outpiglit:
                    if piglit.startswith('subtest'):
                        if not 'subtest' in results:
                            results['subtest'] = {}
                        results['subtest'].update(eval(piglit[7:]))
                    else:
                        results.update(eval(piglit))
                out = '\n'.join(filter(lambda s: not s.startswith('PIGLIT:'),
                                       outlines))
            except:
                results['result'] = 'fail'
                results['note'] = 'Failed to parse result string'

        if 'result' not in results:
            results['result'] = 'fail'
        return out
