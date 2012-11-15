// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 1999  Allen Akin   All Rights Reserved.
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the
// Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT




// main.cpp:  main program for Glean

#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include "options.h"
#include "environ.h"
#include "test.h"
#include "version.h"
#include "lex.h"
#include "dsfilt.h"

using namespace std;

using namespace GLEAN;

char* mandatoryArg(int argc, char* argv[], int i);
void selectTests(Options& o, vector<string>& allTestNames, int argc,
        char* argv[], int i);
void usage(char* command);
void listTests(const Test *tests, bool verbose);

int
main(int argc, char* argv[]) {

	// Until someone gets around to writing a fancy GUI front-end,
	// we'll set options the old-fashioned way.
	Options o;
	bool visFilter = false;

        vector<string> allTestNames;
        for (Test* t = Test::testList; t; t = t->nextTest)
                allTestNames.push_back(t->name);
        sort(allTestNames.begin(), allTestNames.end());
        o.selectedTests = allTestNames;

	bool listTestsMode = false;

	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "--help")) {
			usage(argv[0]);
		} else if (!strcmp(argv[i], "-v")
		    || !strcmp(argv[i], "--verbose")) {
			++o.verbosity;
		} else if (!strcmp(argv[i], "-o")
		    || !strcmp(argv[i], "--overwrite")) {
			o.overwrite = true;
		} else if (!strcmp(argv[i], "--quick")) {
			o.quick = true;
		} else if (!strcmp(argv[i], "--visuals")) {
			visFilter = true;
			++i;
			o.visFilter = mandatoryArg(argc, argv, i);
		} else if (!strcmp(argv[i], "-t")
			   || !strcmp(argv[i], "--tests")) {
			++i;
			selectTests(o, allTestNames, argc, argv, i);
		} else if (!strcmp(argv[i], "--listtests")) {
			listTestsMode = true;
#	    if defined(__X11__)
		} else if (!strcmp(argv[i], "-display")
		    || !strcmp(argv[i], "--display")) {
			++i;
			o.dpyName = mandatoryArg(argc, argv, i);
#	    endif
		} else {
			usage(argv[0]);
		}
	}

	if (listTestsMode) {
		listTests(Test::testList, o.verbosity);
		exit(0);
	}

	if (o.quick && !visFilter) {
		// If we have --quick but not --visuals then limit testing to
		// a single RGB, Z, Stencil visual.
		o.visFilter = "rgb && z>0 && s>0 && conformant";
		o.maxVisuals = 1;
	}

	// Create the test environment, then invoke each test to generate
	// results.
	try {
		Environment e(o);
		for (Test* t = Test::testList; t; t = t->nextTest)
			if (binary_search(o.selectedTests.begin(),
			    o.selectedTests.end(), t->name))
				t->run(e);
	}
#if defined(__X11__)
	catch (WindowSystem::CantOpenDisplay) {
		cerr << "can't open display " << o.dpyName << "\n";
		exit(1);
	}
#endif
	catch (WindowSystem::NoOpenGL) {
		cerr << "display doesn't support OpenGL\n";
		exit(1);
	}
	catch (DrawingSurfaceFilter::Syntax e) {
		cerr << "Syntax error in visual selection criteria:\n"
			"'" << o.visFilter << "'\n";
		for (int i = 0; i < e.position; ++i)
			cerr << ' ';
		cerr << "^ " << e.err << '\n';
		exit(1);
	}
	catch (...) {
		cerr << "caught an unexpected error in main()\n";
		exit(1);
	}

	return 0;
} // main


char*
mandatoryArg(int argc, char* argv[], int i) {
	if (i < argc && argv[i][0] != '-')
		return argv[i];
	usage(argv[0]);
	/*NOTREACHED*/
	return 0;
} // mandatoryArg


void
selectTests(Options& o, vector<string>& allTestNames, int argc, char* argv[],
    int i) {
        if (i >= argc)
                usage(argv[0]);

        // At present, we deal with the following syntax:
        //      [+] testname {(+|-) testname}
        //              Assume we're running none of the tests, then include
        //              those preceded by "+" and exclude those preceded by
        //              "-".
        //      - testname {(+|-) testname}
        //              Assume we're running all of the tests, then exclude
        //              those preceded by "-" and include those preceded by
        //              "+".
        // XXX It would be nice to support the syntax "@filename" to mean
        // "the list of tests given in the named file."  This could be
        // preceded by "+" or "-" just like an ordinary test name, or maybe
        // the +/- should be required in the file itself.

        struct SyntaxError {
                int position;
                SyntaxError(int pos): position(pos) { }
                };

        Lex lex(argv[i]);
        try {
                lex.next();
                if (lex.token == Lex::MINUS)
                        o.selectedTests = allTestNames;
                else
                        o.selectedTests.resize(0);

                while (lex.token != Lex::END) {
                        bool inserting = true;
                        if (lex.token == Lex::MINUS) {
                                inserting = false;
                                lex.next();
                        } else if (lex.token == Lex::PLUS)
                                lex.next();

                        if (lex.token != Lex::ID)
                                throw SyntaxError(lex.position());

                        if (!binary_search(allTestNames.begin(),
                            allTestNames.end(), lex.id))
                                cerr << "Warning: " << lex.id << " ignored;"
                                        " not a valid test name.\n";
                        else {
                                vector<string>::iterator p =
                                    lower_bound(o.selectedTests.begin(),
                                        o.selectedTests.end(), lex.id);
                                if (inserting) {
                                        if (p == o.selectedTests.end()
                                          || *p != lex.id)
                                                o.selectedTests.insert(p,
                                                    lex.id);
                                } else {
                                        // removing
                                        if (p != o.selectedTests.end()
                                          && *p == lex.id)
                                                o.selectedTests.erase(p);
                                }
                        }
                        lex.next();
                }
        }
        catch (Lex::Lexical e) {
                cerr << "Lexical error in test inclusion/exclusion list:\n"
                        "'" << argv[i] << "'\n";
                for (int i = 0; i < e.position; ++i)
                        cerr << ' ';
                cerr << "^ " << e.err << "\n\n";
                usage(argv[0]);
        }
        catch (SyntaxError e) {
                cerr << "'" << argv[i] << "'\n";
                for (int i = 0; i < e.position; ++i)
                        cerr << ' ';
                cerr << "^ Syntax error in test inclusion/exclusion list\n\n";
                usage(argv[0]);
        }
} // selectTests


void
listTests(const Test *tests, bool verbose) {
	for (const Test *t = tests; t; t = t->nextTest) {
		cout << t->name << (verbose? ":" : "") << "\n";
		if (verbose) {
			cout << t->description;
			cout << '\n';
		}
	}
}


void
usage(char* command) {
	cerr << GLEAN::versionString << '\n';
	cerr << "Usage:  " << command << " [options]\n"
"\n"
"options:\n"
"       (-v|--verbose)             # each occurrence increases\n"
"                                  # verbosity of output\n"
"       (-o|--overwrite)           # overwrite existing results database\n"
"       --visuals 'filter-string'  # select subset of visuals (FBConfigs,\n"
"                                  # pixel formats) to test\n"
"       (-t|--tests) {(+|-)test}   # choose tests to include (+) or exclude (-)\n"
"       --quick                    # run fewer tests to reduce test time\n"
"       --listtests                # list test names and exit\n"
"       --help                     # display usage information\n"
#if defined(__X11__)
"       -display X11-display-name  # select X11 display to use\n"
"           (or --display)\n"
#elif defined(__WIN__)
#endif
		;
	exit(1);
} // usage
