# Copyright 2013-2015 Intel Corporation
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

"""Module with tools for generating summaries from a set of results.

Currently supported in this module are a console based text summary and a html
based summary.

"""

from __future__ import print_function, absolute_import
import os
import os.path as path
import itertools
import shutil
import tempfile
import datetime
import re
import getpass
import sys
import errno
import textwrap
import operator

from mako.lookup import TemplateLookup

# a local variable status exists, prevent accidental overloading by renaming
# the module
import framework.status as so
from framework.core import lazy_property
from framework import grouptools, backends, exceptions

__all__ = [
    'console',
    'html',
]

_TEMP_DIR = path.join(
    tempfile.gettempdir(),
    "piglit-{}".format(getpass.getuser()),
    'version-{}'.format(sys.version.split()[0]))
_TEMPLATE_DIR = path.join(path.dirname(__file__), '..', 'templates')
_TEMPLATES = TemplateLookup(
    _TEMPLATE_DIR,
    output_encoding="utf-8",
    encoding_errors='replace',
    module_directory=path.join(_TEMP_DIR, "html-summary"))


class Results(object):  # pylint: disable=too-few-public-methods
    """Container object for results.

    Has the results, the names of status, and the counts of statuses.

    """
    def __init__(self, results):
        self.results = results
        self.names = Names(self)
        self.counts = Counts(self)

    def get_result(self, name):
        """Get all results for a single test.

        Replace any missing vaules with status.NOTRUN, correclty handles
        subtests.

        """
        results = []
        for res in self.results:
            try:
                results.append(res.tests[name].result)
            except KeyError:
                results.append(so.NOTRUN)
        if all(x == so.NOTRUN for x in results):
            # this is likely a subtest, see if that's the case
            name, test = grouptools.splitname(name)

            results = []
            for res in self.results:
                try:
                    results.append(res.tests[name].subtests[test])
                except KeyError:
                    results.append(so.NOTRUN)
        return results


class Names(object):
    """Class containing names of tests for various statuses.

    Members contain lists of sets of names that have a status.

    Each status is lazily evaluated and cached.

    """
    def __init__(self, tests):
        self.__results = tests.results

    def __diff(self, comparator, handler=None):
        """Helper for simplifying comparators using find_diffs."""
        ret = ['']
        if handler is None:
            ret.extend(find_diffs(self.__results, self.all, comparator))
        else:
            ret.extend(find_diffs(self.__results, self.all, comparator,
                                  handler=handler))
        return ret

    def __single(self, comparator):
        """Helper for simplifying comparators using find_single."""
        return find_single(self.__results, self.all, comparator)

    @lazy_property
    def all(self):
        """A set of all tests in all runs."""
        all_ = set()
        for res in self.__results:
            for key, value in res.tests.iteritems():
                if not value.subtests:
                    all_.add(key)
                else:
                    for subt in value.subtests.iterkeys():
                        all_.add(grouptools.join(key, subt))
        return all_

    @lazy_property
    def changes(self):
        return self.__diff(operator.ne)

    @lazy_property
    def problems(self):
        return self.__single(lambda x: x > so.PASS)

    @lazy_property
    def skips(self):
        # It is critical to use is not == here, otherwise so.NOTRUN will also
        # be added to this list
        return self.__single(lambda x: x is so.SKIP)

    @lazy_property
    def regressions(self):
        return self.__diff(operator.lt)

    @lazy_property
    def fixes(self):
        return self.__diff(operator.gt)

    @lazy_property
    def enabled(self):
        def handler(names, name, prev, cur):
            if name not in prev.tests and name in cur.tests:
                names.add(name)
        return self.__diff(
            lambda x, y: x is so.NOTRUN and y is not so.NOTRUN,
            handler=handler)

    @lazy_property
    def disabled(self):
        def handler(names, name, prev, cur):
            if name in prev.tests and name not in cur.tests:
                names.add(name)
        return self.__diff(
            lambda x, y: x is not so.NOTRUN and y is so.NOTRUN,
            handler=handler)

    @lazy_property
    def incomplete(self):
        return self.__single(lambda x: x is so.INCOMPLETE)

    @lazy_property
    def all_changes(self):
        if len(self.changes) > 1:
            return set.union(*self.changes[1:])
        else:
            return set()

    @lazy_property
    def all_disabled(self):
        if len(self.disabled) > 1:
            return set.union(*self.disabled[1:])
        else:
            return set()

    @lazy_property
    def all_enabled(self):
        if len(self.enabled) > 1:
            return set.union(*self.enabled[1:])
        else:
            return set()

    @lazy_property
    def all_fixes(self):
        if len(self.fixes) > 1:
            return set.union(*self.fixes[1:])
        else:
            return set()

    @lazy_property
    def all_regressions(self):
        if len(self.regressions) > 1:
            return set.union(*self.regressions[1:])
        else:
            return set()

    @lazy_property
    def all_incomplete(self):
        if len(self.incomplete) > 1:
            return set.union(*self.incomplete)
        else:
            return self.incomplete[0]

    @lazy_property
    def all_problems(self):
        if len(self.problems) > 1:
            return set.union(*self.problems)
        else:
            return self.problems[0]

    @lazy_property
    def all_skips(self):
        if len(self.skips) > 1:
            return set.union(*self.skips)
        else:
            return self.skips[0]


class Counts(object):
    """Number of tests in each catagory."""
    def __init__(self, tests):
        self.__names = tests.names

    @lazy_property
    def all(self):
        return len(self.__names.all)

    @lazy_property
    def changes(self):
        return [len(x) for x in self.__names.changes]

    @lazy_property
    def problems(self):
        return [len(x) for x in self.__names.problems]

    @lazy_property
    def skips(self):
        return [len(x) for x in self.__names.skips]

    @lazy_property
    def regressions(self):
        return [len(x) for x in self.__names.regressions]

    @lazy_property
    def fixes(self):
        return [len(x) for x in self.__names.fixes]

    @lazy_property
    def enabled(self):
        return [len(x) for x in self.__names.enabled]

    @lazy_property
    def disabled(self):
        return [len(x) for x in self.__names.disabled]

    @lazy_property
    def incomplete(self):
        return [len(x) for x in self.__names.incomplete]


def escape_filename(key):
    """Avoid reserved characters in filenames."""
    return re.sub(r'[<>:"|?*#]', '_', key)


def escape_pathname(key):
    """ Remove / and \\ from names """
    return re.sub(r'[/\\]', '_', key)


def time_as_delta(time):
    """Convert time to a time delta, or return None."""
    if time is not None:
        return datetime.timedelta(0, time)
    return None


def find_diffs(results, tests, comparator, handler=lambda *a: None):
    """Generate diffs between two or more sets of results.

    Arguments:
    results -- a list of results.TestrunResult instances
    tests -- an iterable of test names. Must be iterable more than once
    comparator -- a function with the signautre f(x, y), that returns True when
                  the test should be added to the set of diffs

    Keyword Arguemnts:
    handler -- a function with the signature f(names, name, prev, cur). in the
               event of a KeyError while comparing the results with comparator,
               handler will be passed the (<the set of names>, <the current
               test name>, <the previous result>, <the current result>). This
               can be used to add name even when a KeyError is expected (ie,
               enabled tests).
               Default: pass

    """
    diffs = [] # There can't be changes from nil -> 0
    for prev, cur in itertools.izip(results[:-1], results[1:]):
        names = set()
        for name in tests:
            try:
                if comparator(prev.tests[name].result, cur.tests[name].result):
                    names.add(name)
            except KeyError:
                handler(names, name, prev, cur)
        diffs.append(names)
    return diffs


def find_single(results, tests, func):
    """Find statuses in a single run."""
    statuses = []
    for res in results:
        names = set()
        for name in tests:
            try:
                if func(res.tests[name].result):
                    names.add(name)
            except KeyError:
                pass
        statuses.append(names)
    return statuses


def console(results, mode):
    """ Write summary information to the console """
    assert mode in ['summary', 'diff', 'incomplete', 'all'], mode
    results = Results([backends.load(r) for r in results])

    def printer(list_):
        """Takes a list of test names to print and prints the name and
        result.

        """
        for test in sorted(list_):
            print("{test}: {statuses}".format(
                test='/'.join(test.split(grouptools.SEPARATOR)),
                statuses=' '.join(str(r) for r in results.get_result(test))))

    def print_summary():
        """print a summary."""
        template = textwrap.dedent("""\
            summary:
                   name: {names}
                   ----  {divider}
                   pass: {pass_}
                   fail: {fail}
                  crash: {crash}
                   skip: {skip}
                timeout: {timeout}
                   warn: {warn}
             incomplete: {incomplete}
             dmesg-warn: {dmesg_warn}
             dmesg-fail: {dmesg_fail}
                changes: {changes}
                  fixes: {fixes}
            regressions: {regressions}
                  total: {total}""")

        lens = [max(min(len(x.name), 20), 6) for x in results.results]
        print_template = ' '.join(
            (lambda x: '{: >' + '{0}.{0}'.format(x) + '}')(y) for y in lens)

        def status_printer(stat):
            totals = [str(x.totals['root'][stat]) for x in results.results]
            return print_template.format(*totals)

        print(template.format(
            names=print_template.format(*[r.name for r in results.results]),
            divider=print_template.format(*['-'*l for l in lens]),
            pass_=status_printer('pass'),
            crash=status_printer('crash'),
            fail=status_printer('fail'),
            skip=status_printer('skip'),
            timeout=status_printer('timeout'),
            warn=status_printer('warn'),
            incomplete=status_printer('incomplete'),
            dmesg_warn=status_printer('dmesg-warn'),
            dmesg_fail=status_printer('dmesg-fail'),
            changes=print_template.format(
                *[str(s) for s in results.counts.changes]),
            fixes=print_template.format(
                *[str(s) for s in results.counts.fixes]),
            regressions=print_template.format(
                *[str(s) for s in results.counts.regressions]),
            total=print_template.format(*[
                str(sum(x.totals['root'].itervalues()))
                for x in results.results])))

    # Print the name of the test and the status from each test run
    if mode == 'all':
        printer(results.names.all)
        print_summary()
    elif mode == 'diff':
        printer(results.names.all_changes)
        print_summary()
    elif mode == 'incomplete':
        printer(results.names.all_incomplete)
    elif mode == 'summary':
        print_summary()


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

    # Copy static files
    shutil.copy(path.join(_TEMPLATE_DIR, "index.css"),
                path.join(destination, "index.css"))
    shutil.copy(path.join(_TEMPLATE_DIR, "result.css"),
                path.join(destination, "result.css"))

    result_css = path.join(destination, "result.css")
    index = path.join(destination, "index.html")

    # Iterate across the tests creating the various test specific files
    for each in results.results:
        name = escape_pathname(each.name)
        try:
            os.mkdir(path.join(destination, name))
        except OSError as e:
            if e.errno == errno.EEXIST:
                raise exceptions.PiglitFatalError(
                    'Two or more of your results have the same "name" '
                    'attribute. Try changing one or more of the "name" '
                    'values in your json files.\n'
                    'Duplicate value: {}'.format(name))
            else:
                raise e

        with open(path.join(destination, name, "index.html"), 'w') as out:
            out.write(_TEMPLATES.get_template('testrun_info.mako').render(
                name=each.name,
                totals=each.totals['root'],
                time=time_as_delta(each.time_elapsed),
                options=each.options,
                uname=each.uname,
                glxinfo=each.glxinfo,
                lspci=each.lspci))

        # Then build the individual test results
        for key, value in each.tests.iteritems():
            html_path = path.join(destination, name,
                                  escape_filename(key + ".html"))
            temp_path = path.dirname(html_path)

            if value.result not in exclude:
                # os.makedirs is very annoying, it throws an OSError if
                # the path requested already exists, so do this check to
                # ensure that it doesn't
                if not path.exists(temp_path):
                    os.makedirs(temp_path)

                if value.time:
                    value.time = time_as_delta(value.time)

                with open(html_path, 'w') as out:
                    out.write(_TEMPLATES.get_template(
                        'test_result.mako').render(
                            testname=key,
                            value=value,
                            css=path.relpath(result_css, temp_path),
                            index=path.relpath(index, temp_path)))

    # Finally build the root html files: index, regressions, etc
    pages = frozenset(['changes', 'problems', 'skips', 'fixes',
                       'regressions', 'enabled', 'disabled'])

    # Index.html is a bit of a special case since there is index, all, and
    # alltests, where the other pages all use the same name. ie,
    # changes.html, changes, and page=changes.
    try:
        with open(path.join(destination, "index.html"), 'w') as out:
            out.write(_TEMPLATES.get_template('index.mako').render(
                results=results,
                page='all',
                pages=pages,
                exclude=exclude))

        # Generate the rest of the pages
        for page in pages:
            with open(path.join(destination, page + '.html'), 'w') as out:
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
    except:
        from mako.exceptions import text_error_template
        print(text_error_template().render())
        exit(1)
