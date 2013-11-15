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

import os
import os.path as path
import itertools
import shutil
import collections
import tempfile
from mako.template import Template

# a local variable status exists, prevent accidental overloading by renaming
# the module
import status as so
import core


__all__ = [
    'Summary',
]


class HTMLIndex(list):
    """
    Builds HTML output to be passed to the index mako template, which will be
    rendered into HTML pages. It does this by parsing the lists provided by the
    Summary object, and returns itself, an object with one accessor, a list of
    html strings that will be printed by the mako template.
    """

    def __init__(self, summary, page):
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

        # set a starting depth of 1, 0 is used for 'all' so 1 is the
        # next available group
        depth = 1

        # Current dir is a list representing the groups currently being
        # written.
        currentDir = []

        # Add a new 'tab' for each result
        self._newRow()
        self.append({'type': 'other', 'text': '<td />'})
        for each in summary.results:
            self.append({'type': 'other',
                         'text': '<td class="head"><b>%(name)s</b><br />'
                                 '(<a href="%(href)s">info</a>)'
                                 '</td>' % {'name': each.name,
                                            'href': path.join(each.name,
                                                              "index.html")}})
        self._endRow()

        # Add the toplevel 'all' group
        self._newRow()
        self._groupRow("head", 0, 'all')
        for each in summary.results:
            self._groupResult(summary.fractions[each.name]['all'],
                              summary.status[each.name]['all'])
        self._endRow()

        # Add the groups and tests to the out list
        for key in sorted(page):

            # Split the group names and test names, then determine
            # which groups to close and which to open
            openList = key.split('/')
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
                for each in summary.results:
                    # Decide which fields need to be updated
                    self._groupResult(
                        summary.fractions[each.name][path.join(*currentDir)],
                        summary.status[each.name][path.join(*currentDir)])

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
            for each in summary.results:
                try:
                    self._testResult(each.name, key, each.tests[key]['result'])
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
        if isinstance(css, so.NotRun):
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
        self.append({'type': 'testResult',
                     'class': text,
                     'href': path.join(group, href + ".html"),
                     'text': text})


class Summary:
    """
    This Summary class creates an initial object containing lists of tests
    including all, changes, problems, skips, regressions, and fixes. It then
    uses methods to generate various kinds of output. The reference
    implementation is HTML output through mako, aptly named generateHTML().
    """
    TEMP_DIR = path.join(tempfile.gettempdir(), "piglit/html-summary")
    TEMPLATE_DIR = path.join(os.environ['PIGLIT_SOURCE_DIR'], 'templates')

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
        self.results = [core.load_results(i) for i in resultfiles]

        self.status = {}
        self.fractions = {}
        self.totals = {}
        self.tests = {'all': set(), 'changes': set(), 'problems': set(),
                      'skipped': set(), 'regressions': set(), 'fixes': set()}

        def fgh(test, result):
            """ Helper for updating the fractions and status lists """
            fraction[test] = tuple(
                [sum(i) for i in zip(fraction[test], result.fraction)])
            if result != so.Skip() and status[test] < result:
                status[test] = result

        for results in self.results:
            # Create a set of all of the tset names across all of the runs
            self.tests['all'] = set(self.tests['all'] | set(results.tests))

            # Create two dictionaries that have a default factory: they return
            # a default value instead of a key error.
            # This default key must be callable
            self.fractions[results.name] = collections.defaultdict(lambda: (0, 0))
            self.status[results.name] = collections.defaultdict(so.NotRun)

            # short names
            fraction = self.fractions[results.name]
            status = self.status[results.name]

            for key, value in results.tests.iteritems():
                #FIXME: Add subtest support

                # Walk the test name as if it was a path, at each level update
                # the tests passed over the total number of tests (fractions),
                # and update the status of the current level if the status of
                # the previous level was worse, but is not skip
                while key != '':
                    fgh(key, value['result'])
                    key = path.dirname(key)

                # when we hit the root update the 'all' group and stop
                fgh('all', value['result'])

        # Create the lists of statuses like problems, regressions, fixes,
        # changes and skips
        for test in self.tests['all']:
            status = []
            for each in self.results:
                try:
                    status.append(each.tests[test]['result'])
                except KeyError:
                    status.append(so.NotRun())

            # Problems include: warn, dmesg-warn, fail, dmesg-fail, and crash.
            # Skip does not go on this page, it has the 'skipped' page
            if so.Skip() > max(status) > so.Pass():
                self.tests['problems'].add(test)

            # Find all tests with a status of skip
            if so.Skip() in status:
                self.tests['skipped'].add(test)

            # find fixes, regressions, and changes
            for i in xrange(len(status) - 1):
                first = status[i]
                last = status[i + 1]
                if first < last and so.NotRun() not in (first, last):
                    self.tests['regressions'].add(test)
                if first > last and so.NotRun() not in (first, last):
                    self.tests['fixes'].add(test)
                # Changes cannot be added in the fixes and regressions passes
                # becasue NotRun is a change, but not a regression or fix
                if first != last:
                    self.tests['changes'].add(test)

    def __find_totals(self):
        """
        Private: Find the total number of pass, fail, crash, skip, and warn in
        the *last* set of results stored in self.results.
        """
        self.totals = {'pass': 0, 'fail': 0, 'crash': 0, 'skip': 0, 'warn': 0,
                       'dmesg-warn': 0, 'dmesg-fail': 0}

        for test in self.results[-1].tests.values():
            self.totals[str(test['result'])] += 1

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
        testindex = Template(filename=path.join(self.TEMPLATE_DIR, "testrun_info.mako"),
                             output_encoding="utf-8",
                             module_directory=self.TEMP_DIR)

        # Create the mako object for the individual result files
        testfile = Template(filename=path.join(self.TEMPLATE_DIR, "test_result.mako"),
                            output_encoding="utf-8",
                            module_directory=self.TEMP_DIR)

        result_css = path.join(destination, "result.css")
        index = path.join(destination, "index.html")

        # Iterate across the tests creating the various test specific files
        for each in self.results:
            os.mkdir(path.join(destination, each.name))

            with open(path.join(destination, each.name, "index.html"), 'w') as out:
                out.write(testindex.render(name=each.name,
                                           time=each.time_elapsed,
                                           options=each.options,
                                           glxinfo=each.glxinfo,
                                           lspci=each.lspci))

            # Then build the individual test results
            for key, value in each.tests.iteritems():
                temp_path = path.join(destination, each.name, path.dirname(key))

                if value['result'] not in exclude:
                    # os.makedirs is very annoying, it throws an OSError if
                    # the path requested already exists, so do this check to
                    # ensure that it doesn't
                    if not path.exists(temp_path):
                        os.makedirs(temp_path)

                    with open(path.join(destination, each.name, key + ".html"),
                              'w') as out:
                        out.write(testfile.render(
                            testname=key,
                            status=value.get('result', 'None'),
                            # Returning a NoneType (instaed of 'None') prevents
                            # this field from being generated.setting the
                            # environment to run tests is ugly, and should
                            # disapear at somepoint
                            env=value.get('environment', None),
                            returncode=value.get('returncode', 'None'),
                            time=value.get('time', 'None'),
                            info=value.get('info', 'None'),
                            traceback=value.get('traceback', 'None'),
                            command=value.get('command', 'None'),
                            dmesg=value.get('dmesg', 'None'),
                            css=path.relpath(result_css, temp_path),
                            index=path.relpath(index, temp_path)))

        # Finally build the root html files: index, regressions, etc
        index = Template(filename=path.join(self.TEMPLATE_DIR, "index.mako"),
                         output_encoding="utf-8",
                         module_directory=self.TEMP_DIR)

        empty_status = Template(filename=path.join(self.TEMPLATE_DIR, "empty_status.mako"),
                                output_encoding="utf-8",
                                module_directory=self.TEMP_DIR)

        pages = ('changes', 'problems', 'skipped', 'fixes', 'regressions')

        # Index.html is a bit of a special case since there is index, all, and
        # alltests, where the other pages all use the same name. ie,
        # changes.html, self.changes, and page=changes.
        with open(path.join(destination, "index.html"), 'w') as out:
            out.write(index.render(results=HTMLIndex(self, self.tests['all']),
                                   page='all',
                                   pages=pages,
                                   colnum=len(self.results),
                                   exclude=exclude))

        # Generate the rest of the pages
        for page in pages:
            with open(path.join(destination, page + '.html'), 'w') as out:
            # If there is information to display display it
                if self.tests[page]:
                    out.write(index.render(results=HTMLIndex(self,
                                                             self.tests[page]),
                                           pages=pages,
                                           page=page,
                                           colnum=len(self.results),
                                           exclude=exclude))
                # otherwise provide an empty page
                else:
                    out.write(empty_status.render(page=page, pages=pages))

    def generate_text(self, diff, summary):
        """ Write summary information to the console """
        self.__find_totals()

        # Print the name of the test and the status from each test run
        if not summary:
            if diff:
                for test in self.tests['changes']:
                    print "%(test)s: %(statuses)s" % {'test': test, 'statuses':
                          ' '.join([str(i.tests.get(test, {'result': so.Skip()})
                                    ['result']) for i in self.results])}
            else:
                for test in self.tests['all']:
                    print "%(test)s: %(statuses)s" % {'test': test, 'statuses':
                          ' '.join([str(i.tests.get(test, {'result': so.Skip()})
                                    ['result']) for i in self.results])}

        # Print the summary
        print "summary:"
        print "       pass: %d" % self.totals['pass']
        print "       fail: %d" % self.totals['fail']
        print "      crash: %d" % self.totals['crash']
        print "       skip: %d" % self.totals['skip']
        print "       warn: %d" % self.totals['warn']
        print " dmesg-warn: %d" % self.totals['dmesg-warn']
        print " dmesg-fail: %d" % self.totals['dmesg-fail']
        if self.tests['changes']:
            print "    changes: %d" % len(self.tests['changes'])
            print "      fixes: %d" % len(self.tests['fixes'])
            print "regressions: %d" % len(self.tests['regressions'])

        print "      total: %d" % sum(self.totals.values())
