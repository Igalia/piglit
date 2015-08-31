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
import posixpath
import errno
import textwrap
import operator

from mako.template import Template

# a local variable status exists, prevent accidental overloading by renaming
# the module
import framework.status as so
from framework.core import lazy_property
from framework import grouptools, backends, exceptions

__all__ = [
    'Summary',
    'console'
]


def escape_filename(key):
    """Avoid reserved characters in filenames."""
    return re.sub(r'[<>:"|?*#]', '_', key)


def escape_pathname(key):
    """ Remove / and \\ from names """
    return re.sub(r'[/\\]', '_', key)


def normalize_href(href):
    """Force backward slashes in URLs."""
    return href.replace('\\', '/')


class HTMLIndex(list):
    """
    Builds HTML output to be passed to the index mako template, which will be
    rendered into HTML pages. It does this by parsing the lists provided by the
    Summary object, and returns itself, an object with one accessor, a list of
    html strings that will be printed by the mako template.
    """

    def __init__(self, results, page):
        """
        Steps through the list of groups and tests from all of the results and
        generates a list of dicts that are passed to mako and turned into HTML
        """

        def returnList(open, close):
            """
            As HTMLIndex iterates through the groups and tests it uses this
            function to determine which groups to close (and thus reduce the
            depth of the next write) and which ones to open (thus increasing
            the depth)

            To that end one of two things happens, the path to the previous
            group (close) and the next group (open) are equal, in that event we
            don't want to open and close, becasue that will result in a
            sawtooth pattern of a group with one test followed by the same
            group with one test, over and over.  Instead we simply return two
            empty lists, which will result in writing a long list of test
            results.  The second option is that the paths are different, and
            the function determines any commonality between the paths, and
            returns the differences as close (the groups which are completly
            written) and open (the new groups to write).
            """
            common = []

            # Open and close are lists, representing the group hierarchy, open
            # being the groups that need are soon to be written, and close
            # representing the groups that have finished writing.
            if open == close:
                return [], []
            else:
                for i, j in itertools.izip_longest(open, close):
                    if i != j:
                        for k in common:
                            open.remove(k)
                            close.remove(k)
                        return open, close
                    else:
                        common.append(i)

        def group_result(result, group):
            """Get the worst status in a group."""
            if group not in result.totals:
                return so.NOTRUN

            return max([so.status_lookup(s) for s, v in
                        result.totals[group].iteritems() if v > 0])

        def group_fraction(result, group):
            """Get the fraction value for a group."""
            if group not in result.totals:
                return (0, 0)

            num = 0
            den = 0
            for k, v in result.totals[group].iteritems():
                if v > 0:
                    s = so.status_lookup(k)
                    num += s.fraction[0] * v
                    den += s.fraction[1] * v

            return (num, den)

        # set a starting depth of 1, 0 is used for 'all' so 1 is the
        # next available group
        depth = 1

        # Current dir is a list representing the groups currently being
        # written.
        currentDir = []

        # Add a new 'tab' for each result
        self._newRow()
        self.append({'type': 'other', 'text': '<td />'})
        for each in results.results:
            href = normalize_href(os.path.join(
                escape_pathname(each.name), "index.html"))
            self.append({'type': 'other',
                         'text': '<td class="head"><b>%(name)s</b><br />'
                                 '(<a href="%(href)s">info</a>)'
                                 '</td>' % {'name': each.name,
                                            'href': href}})
        self._endRow()

        # Add the toplevel 'all' group
        self._newRow()
        self._groupRow("head", 0, 'all')
        for each in results.results:
            self._groupResult(group_fraction(each, 'root'),
                              group_result(each, 'root'))
        self._endRow()

        # Add the groups and tests to the out list
        for key in sorted(page):

            # Split the group names and test names, then determine
            # which groups to close and which to open
            openList = key.split(grouptools.SEPARATOR)
            test = openList.pop()
            openList, closeList = returnList(openList, list(currentDir))

            # Close any groups in the close list
            # for each group closed, reduce the depth by one
            for i in reversed(closeList):
                currentDir.remove(i)
                depth -= 1

            # Open new groups
            for localGroup in openList:
                self._newRow()

                # Add the left-most column: the name of the group
                self._groupRow("head", depth, localGroup)

                # Add the group that we just opened to the currentDir, which
                # will then be used to add that group to the HTML list. If
                # there is a KeyError (the group doesn't exist), use (0, 0)
                # which will get skip. This sets the group coloring correctly
                currentDir.append(localGroup)
                for each in results.results:
                    # Decide which fields need to be updated
                    self._groupResult(
                        group_fraction(each, grouptools.join(*currentDir)),
                        group_result(each, grouptools.join(*currentDir)))

                # After each group increase the depth by one
                depth += 1
                self._endRow()

            # Add the tests for the current group
            self._newRow()

            # Add the left-most column: the name of the test
            self._testRow("group", depth, test)

            # Add the result from each test result to the HTML summary If there
            # is a KeyError (a result doesn't contain a particular test),
            # return Not Run, with clas skip for highlighting
            for each in results.results:
                # If the "group" at the top of the key heirachy contains
                # 'subtest' then it is really not a group, link to that page
                try:
                    if each.tests[grouptools.groupname(key)].subtests:
                        href = grouptools.groupname(key)
                except KeyError:
                    href = key

                href = escape_filename(href)

                try:
                    self._testResult(escape_pathname(each.name), href,
                                     each.tests[key].result)
                except KeyError:
                    self.append({'type': 'other',
                                 'text': '<td class="skip">Not Run</td>'})
            self._endRow()

    def _newRow(self):
        self.append({'type': 'newRow'})

    def _endRow(self):
        self.append({'type': 'endRow'})

    def _groupRow(self, cssclass, depth, groupname):
        """
        Helper function for appending new groups to be written out
        in HTML.

        This particular function is used to write the left most
        column of the summary. (the one with the indents)
        """
        self.append({'type': "groupRow",
                     'class': cssclass,
                     'indent': (1.75 * depth),
                     'text': groupname})

    def _groupResult(self, value, css):
        """
        Helper function for appending the results of groups to the
        HTML summary file.
        """
        # "Not Run" is not a valid css class replace it with skip
        if css == so.NOTRUN:
            css = 'skip'

        self.append({'type': "groupResult",
                     'class': css,
                     'text': "%s/%s" % (value[0], value[1])})

    def _testRow(self, cssclass, depth, groupname):
        """
        Helper function for appending new tests to be written out
        in HTML.

        This particular function is used to write the left most
        column of the summary. (the one with the indents)
        """
        self.append({'type': "testRow",
                     'class': cssclass,
                     'indent': (1.75 * depth),
                     'text': groupname})

    def _testResult(self, group, href, text):
        """
        Helper function for writing the results of tests

        This function writes the cells other than the left-most cell,
        displaying pass/fail/crash/etc and formatting the cell to the
        correct color.
        """
        # "Not Run" is not a valid class, if it apears set the class to skip
        if text == so.NOTRUN:
            css = 'skip'
            href = None
        else:
            css = text
            # Use posixpath for this URL because it maintains portability
            # between windows and *nix.
            href = posixpath.join(group, href + ".html")
            href = normalize_href(href)

        self.append({'type': 'testResult',
                     'class': css,
                     'href': href,
                     'text': text})


class Summary:
    """
    This Summary class creates an initial object containing lists of tests
    including all, changes, problems, skips, regressions, and fixes. It then
    uses methods to generate various kinds of output. The reference
    implementation is HTML output through mako, aptly named generateHTML().
    """
    TEMP_DIR = path.join(tempfile.gettempdir(),
                         "piglit-{}".format(getpass.getuser()),
                         'version-{}'.format(sys.version.split()[0]),
                         "html-summary")
    TEMPLATE_DIR = path.abspath(
        path.join(path.dirname(__file__), '..', 'templates'))

    def __init__(self, resultfiles):
        """
        Create an initial object with all of the result information rolled up
        in an easy to process form.

        The constructor of the summary class has an attribute for each HTML
        summary page, which are fed into the index.mako file to produce HTML
        files. resultfiles is a list of paths to JSON results generated by
        piglit-run.
        """

        # Create a Result object for each piglit result and append it to the
        # results list
        self.results = Results([backends.load(i) for i in resultfiles])

    def generate_html(self, destination, exclude):
        """
        Produce HTML summaries.

        Basically all this does is takes the information provided by the
        constructor, and passes it to mako templates to generate HTML files.
        The beauty of this approach is that mako is leveraged to do the
        heavy lifting, this method just passes it a bunch of dicts and lists
        of dicts, which mako turns into pretty HTML.
        """
        # Copy static files
        shutil.copy(path.join(self.TEMPLATE_DIR, "index.css"),
                    path.join(destination, "index.css"))
        shutil.copy(path.join(self.TEMPLATE_DIR, "result.css"),
                    path.join(destination, "result.css"))

        # Create the mako object for creating the test/index.html file
        testindex = Template(filename=path.join(self.TEMPLATE_DIR,
                                                "testrun_info.mako"),
                             output_encoding="utf-8",
                             encoding_errors='replace',
                             module_directory=self.TEMP_DIR)

        # Create the mako object for the individual result files
        testfile = Template(filename=path.join(self.TEMPLATE_DIR,
                                               "test_result.mako"),
                            output_encoding="utf-8",
                            encoding_errors='replace',
                            module_directory=self.TEMP_DIR)

        result_css = path.join(destination, "result.css")
        index = path.join(destination, "index.html")

        # Iterate across the tests creating the various test specific files
        for each in self.results.results:
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

            if each.time_elapsed is not None:
                time = datetime.timedelta(0, each.time_elapsed)
            else:
                time = None

            with open(path.join(destination, name, "index.html"), 'w') as out:
                out.write(testindex.render(
                    name=each.name,
                    totals=each.totals['root'],
                    time=time,
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
                        value.time = datetime.timedelta(0, value.time)

                    with open(html_path, 'w') as out:
                        out.write(testfile.render(
                            testname=key,
                            value=value,
                            css=path.relpath(result_css, temp_path),
                            index=path.relpath(index, temp_path)))

        # Finally build the root html files: index, regressions, etc
        index = Template(filename=path.join(self.TEMPLATE_DIR, "index.mako"),
                         output_encoding="utf-8",
                         encoding_errors='replace',
                         module_directory=self.TEMP_DIR)

        empty_status = Template(filename=path.join(self.TEMPLATE_DIR,
                                                   "empty_status.mako"),
                                output_encoding="utf-8",
                                encoding_errors='replace',
                                module_directory=self.TEMP_DIR)

        pages = frozenset(['changes', 'problems', 'skips', 'fixes',
                           'regressions', 'enabled', 'disabled'])

        # Index.html is a bit of a special case since there is index, all, and
        # alltests, where the other pages all use the same name. ie,
        # changes.html, self.changes, and page=changes.
        with open(path.join(destination, "index.html"), 'w') as out:
            out.write(index.render(
                results=HTMLIndex(self.results, self.results.names.all),
                page='all',
                pages=pages,
                colnum=len(self.results.results),
                exclude=exclude))

        # Generate the rest of the pages
        for page in pages:
            with open(path.join(destination, page + '.html'), 'w') as out:
                # If there is information to display display it
                if sum(getattr(self.results.counts, page)) > 0:
                    out.write(index.render(
                        results=HTMLIndex(
                            self.results,
                            getattr(self.results.names, 'all_' + page)),
                        pages=pages,
                        page=page,
                        colnum=len(self.results.results),
                        exclude=exclude))
                # otherwise provide an empty page
                else:
                    out.write(empty_status.render(page=page, pages=pages))


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
