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
import string
from itertools import izip_longest
from shutil import copy
from json import loads
from mako.template import Template

import core

__all__ = [
    'Summary',
]


class Result(core.TestrunResult):
    """
    Object that opens, reads, and stores the data in a resultfile.
    """
    def __init__(self, resultfile):
        # Run the init from TestrunResult
        core.TestrunResult.__init__(self)

        # Load the json file, or if that fails assume that the locations given
        # is a folder containing a json file
        try:
            with open(resultfile, 'r') as file:
                result = self.parseFile(file)
        except IOError:
            with open(path.join(resultfile, 'main'), 'r') as file:
                result = self.parseFile(file)


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
                for i, j in izip_longest(open, close):
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
                    try:
                        self._groupResult(summary.fractions[each.name]
                                          [path.join(*currentDir)],
                                          summary.status[each.name]
                                          [path.join(*currentDir)])
                    except KeyError:
                        self._groupResult((0, 0), 'skip')

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

    def __init__(self, resultfiles):
        """
        Create an initial object with all of the result information rolled up
        in an easy to process form.

        The constructor of the summary class has an attribute for each HTML
        summary page, which are fed into the index.mako file to produce HTML
        files. resultfiles is a list of paths to JSON results generated by
        piglit-run.
        """

        def buildDictionary(summary):
            # Build a dictionary from test name to pass count/total count, i.e.
            # counts['spec/glsl/foo'] == (456, 800)
            counts = {}

            if not summary.tests:
                return {}

            lastGroup = ''

            # Build a dictionary of group stati, passing groupname = status.
            # This is the "worst" status of the group in descending order:
            # crash, skip, fail, warn, pass
            status = {}

            # currentStack is a stack containing numerical values that that
            # relate to a status output, 5 for crash, 4 for skip, 3 for fail, 2
            # for warn, 1 for pass
            currentStatus = []

            # Stack contains tuples like: (pass count, total count)
            stack = []

            def openGroup(name):
                stack.append((0, 0))

                # Since skip is the "lowest" status for HTML generation, if
                # there is another status it will replace skip
                currentStatus.append('skip')

            def closeGroup(group_name):
                # We're done with this group, record the number of pass/total
                # in the dictionary.
                (nr_pass, nr_total) = stack.pop()
                counts[group_name] = (nr_pass, nr_total)

                # Also add our pass/total count to the parent group's counts
                # (which are still being computed)
                (parent_pass, parent_total) = stack[-1]
                stack[-1] = (parent_pass + nr_pass, parent_total + nr_total)

                # Add the status back to the group hierarchy
                if status_to_number(currentStatus[-2]) < \
                        status_to_number(currentStatus[-1]):
                    currentStatus[-2] = currentStatus[-1]
                status[group_name] = currentStatus.pop()

            def status_to_number(status):
                """
                like status_to_number in the constructor, this function
                converts statuses into numbers so they can be comapared
                logically/mathematically. The only difference between this and
                init::status_to_number is the values assigned. The reason for
                this is that here we are looking for the 'worst' status, while
                in init::status_to_number we are looking for regressions in
                status.
                """
                if status == 'skip':
                    return 1
                elif status == 'pass':
                    return 2
                elif status in ['warn', 'dmesg-warn']:
                    return 3
                elif status in ['fail', 'dmesg-fail']:
                    return 4
                elif status == 'crash':
                    return 5

            openGroup('fake')
            openGroup('all')

            # fulltest is a full test name,
            # i.e. tests/shaders/glsl-algebraic-add-zero
            for fulltest in sorted(summary.tests):
                # same as fulltest.rpartition('/')
                group, test = path.split(fulltest)

                if group != lastGroup:
                    # We're in a different group now.  Close the old ones
                    # and open the new ones.
                    for x in path.relpath(group, lastGroup).split('/'):
                        if x != '..':
                            openGroup(x)
                        else:
                            closeGroup(lastGroup)
                            lastGroup = path.normpath(path.join(lastGroup,
                                                                ".."))

                    lastGroup = group

                # Add the current test
                (pass_so_far, total_so_far) = stack[-1]
                if summary.tests[fulltest]['result'] == 'pass':
                    pass_so_far += 1
                if summary.tests[fulltest]['result'] != 'skip':
                    total_so_far += 1
                stack[-1] = (pass_so_far, total_so_far)

                # compare the status
                if status_to_number(summary.tests[fulltest]['result']) > \
                        status_to_number(currentStatus[-1]):
                    currentStatus[-1] = summary.tests[fulltest]['result']

            # Work back up the stack closing groups as we go until we reach the
            # top, where we need to manually close this as "all"
            while len(stack) > 2:
                closeGroup(lastGroup)
                lastGroup = path.dirname(lastGroup)
            closeGroup("all")

            assert(len(stack) == 1)
            assert(len(currentStatus) == 1)

            return counts, status

        # Create a Result object for each piglit result and append it to the
        # results list
        self.results = [Result(i) for i in resultfiles]

        self.status = {}
        self.fractions = {}
        self.tests = {'all': [], 'changes': [], 'problems': [], 'skipped': [],
                      'regressions': [], 'fixes': []}

        for each in self.results:
            # Build a dict of the status output of all of the tests, with the
            # name of the test run as the key for that result, this will be
            # used to write pull the statuses later
            fraction, status = buildDictionary(each)
            self.fractions.update({each.name: fraction})
            self.status.update({each.name: status})

            # Create a list with all the test names in it
            self.tests['all'] = list(set(self.tests['all']) | set(each.tests))

    def __generate_lists(self, lists):
        """
        Private: Generate the lists of changes, problems, regressions, fixes,
        and skips

        lists is a list contianing any of the following: changes, problems,
        skips, fixes (which will also generate regressions)

        This method has different code paths to allow the exclusion of certain
        lists being generated. This is both useful for speeding up HTML
        generation when a page isn't needed (regressions with only one test
        file is provided), and for JUnit and text which only need a limited
        subset of these lists
        """
        def find_regressions(status):
            """
            Helper function to convert named statuses into number, since number
            can more easily be compared using logical/mathematical operators.
            The use of this is to look for regressions in status.
            """
            if status == 'pass':
                return 1
            elif status in ['warn', 'dmesg-warn']:
                return 2
            elif status in ['fail', 'dmesg-fail']:
                return 3
            elif status == 'skip':
                return 4
            elif status == 'crash':
                return 5
            elif status == 'special':
                return 0

        for test in self.tests['all']:
            status = []
            for each in self.results:
                try:
                    status.append(find_regressions(each.tests[test]['result']))
                except KeyError:
                    status.append(find_regressions("special"))

            if 'changes' in lists:
                # Check and append self.tests['changes']
                # A set cannot contain duplicate entries, so creating a set
                # out the list will reduce it's length to 1 if all entries
                # are the same, meaning it is not a change
                if len(set(status)) > 1:
                    self.tests['changes'].append(test)

            if 'problems' in lists:
                # If the result contains a value other than 1 (pass) or 4
                # (skip) it is a problem. Skips are not problems becasuse
                # they have Their own page.
                if [i for e in [2, 3, 5] for i in status if e is i]:
                    self.tests['problems'].append(test)

            if 'skipped' in lists:
                # Find all tests with a status of skip
                if 4 in status:
                    self.tests['skipped'].append(test)

            if 'fixes' in lists:
                # Find both fixes and regressions, and append them to the
                # proper lists
                for i in xrange(len(status) - 1):
                    if status[i] < status[i + 1] and status[i] != 0:
                        self.tests['regressions'].append(test)
                    if status[i] > 1 and status[i + 1] == 1:
                        self.tests['fixes'].append(test)

        # Remove duplicate entries from the status lists
        # If there are 4+ results can result in mutiple passes or regressions
        # and changes in other words: "pass fail pass fail" will result in a
        # regression, a fix, and a regression and it will be printed twice in
        # the summary. Turning them into sets remove duplicates
        for (result, value) in self.tests.items():
            self.tests[result] = set(value)

    def __find_totals(self):
        """
        Private: Find the total number of pass, fail, crash, skip, and warn in
        the *last* set of results stored in self.results.
        """
        self.totals = {'pass': 0, 'fail': 0, 'crash': 0, 'skip': 0, 'warn': 0,
                       'dmesg-warn': 0, 'dmesg-fail': 0}

        for test in self.results[-1].tests.values():
            self.totals[test['result']] += 1

    def generateHTML(self, destination, exclude):
        """
        Produce HTML summaries.

        Basically all this does is takes the information provided by the
        constructor, and passes it to mako templates to generate HTML files.
        The beauty of this approach is that mako is leveraged to do the
        heavy lifting, this method just passes it a bunch of dicts and lists
        of dicts, which mako turns into pretty HTML.
        """

        # Copy static files
        copy("templates/index.css", path.join(destination, "index.css"))
        copy("templates/result.css", path.join(destination, "result.css"))

        # Create the mako object for creating the test/index.html file
        testindex = Template(filename="templates/testrun_info.mako",
                             output_encoding="utf-8",
                             module_directory=".makotmp")

        # Create the mako object for the individual result files
        testfile = Template(filename="templates/test_result.mako",
                            output_encoding="utf-8",
                            module_directory=".makotmp")

        resultCss = path.join(destination, "result.css")
        index = path.join(destination, "index.html")

        # Iterate across the tests creating the various test specific files
        for each in self.results:
            os.mkdir(path.join(destination, each.name))

            file = open(path.join(destination, each.name, "index.html"), 'w')
            file.write(testindex.render(name=each.name,
                                        time=each.time_elapsed,
                                        options=each.options,
                                        glxinfo=each.glxinfo,
                                        lspci=each.lspci))
            file.close()

            # Then build the individual test results
            for key, value in each.tests.iteritems():
                tPath = path.join(destination, each.name, path.dirname(key))

                if value['result'] not in exclude:
                    # os.makedirs is very annoying, it throws an OSError if
                    # the path requested already exists, so do this check to
                    # ensure that it doesn't
                    if not path.exists(tPath):
                        os.makedirs(tPath)

                    file = open(path.join(destination,
                                          each.name,
                                          key + ".html"), 'w')
                    file.write(testfile.render(
                        testname=key,
                        status=value.get('result', 'None'),
                        returncode=value.get('returncode', 'None'),
                        time=value.get('time', 'None'),
                        info=value.get('info', 'None'),
                        traceback=value.get('traceback', 'None'),
                        command=value.get('command', 'None'),
                        dmesg=value.get('dmesg', 'None'),
                        css=path.relpath(resultCss, tPath),
                        index=path.relpath(index, tPath)))
                    file.close()

        # Finally build the root html files: index, regressions, etc
        index = Template(filename="templates/index.mako",
                         output_encoding="utf-8",
                         module_directory=".makotmp")

        empty_status = Template(filename="templates/empty_status.mako",
                                output_encoding="utf-8",
                                module_directory=".makotmp")

        # A list of pages to be generated
        # If there is only one set of results, then there cannot be changes,
        # regressions or fixes, so don't generate those pages.
        if len(self.results) > 1:
            pages = ['changes', 'problems', 'skipped', 'fixes', 'regressions']
        else:
            pages = ['problems', 'skipped']

        self.__generate_lists(pages)

        # Index.html is a bit of a special case since there is index, all, and
        # alltests, where the other pages all use the same name. ie,
        # changes.html, self.changes, and page=changes.
        file = open(path.join(destination, "index.html"), 'w')
        file.write(index.render(results=HTMLIndex(self, self.tests['all']),
                                page='all',
                                pages=pages,
                                colnum=len(self.results),
                                exclude=exclude))
        file.close()

        # Generate the rest of the pages
        for page in pages:
            file = open(path.join(destination, page + '.html'), 'w')
            # If there is information to display display it
            if self.tests[page]:
                file.write(index.render(results=HTMLIndex(self,
                                                          self.tests[page]),
                                        pages=pages,
                                        page=page,
                                        colnum=len(self.results),
                                        exclude=exclude))
            # otherwise provide an empty page
            else:
                file.write(empty_status.render(page=page, pages=pages))

            file.close()

    def generateText(self, diff, summary):
        self.__find_totals()

        # If there are more than one set of results we need to find changes
        if len(self.results) > 1:
            self.__generate_lists(['changes', 'fixes', 'regressions'])

        # Print the name of the test and the status from each test run
        if not summary:
            if diff:
                for test in self.tests['changes']:
                    print "%(test)s: %(statuses)s" % {'test': test, 'statuses':
                          ' '.join([i.tests.get(test, {'result': 'skip'})
                                    ['result'] for i in self.results])}
            else:
                for test in self.tests['all']:
                    print "%(test)s: %(statuses)s" % {'test': test, 'statuses':
                          ' '.join([i.tests.get(test, {'result': 'skip'})
                                    ['result'] for i in self.results])}

        # Print the summary
        print "summary:"
        print "       pass: %d" % self.totals['pass']
        print "       fail: %d" % self.totals['fail']
        print "      crash: %d" % self.totals['crash']
        print "       skip: %d" % self.totals['skip']
        print "       warn: %d" % self.totals['warn']
        print "       dmesg-warn: %d" % self.totals['dmesg-warn']
        print "       dmesg-fail: %d" % self.totals['dmesg-fail']
        if self.tests['changes']:
            print "    changes: %d" % len(self.tests['changes'])
            print "      fixes: %d" % len(self.tests['fixes'])
            print "regressions: %d" % len(self.tests['regressions'])

        print "      total: %d" % sum(self.totals.values())
