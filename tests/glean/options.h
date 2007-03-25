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




// options.h:  global test options

// This class encapsulates global options that apply to the entire
// testing process -- things like the display name (for X11),
// constraints on the drawing surface configurations to be tested,
// locations of test results files, etc.

// We collect this information for two reasons.  First, it allows the
// (relatively) large number of parameters needed for creating an
// Environment to be passed cleanly to Environment's constructor.
// Second, it allows the process of gathering parameters (by parsing a
// command line, running a set of GUI dialogs, etc.) to be separated
// from the creation of the Environment.



#ifndef __options_h__
#define __options_h__

using namespace std;

#include <string>
#include <vector>

namespace GLEAN {

class Options {
    public:
	typedef enum {notSet, run, compare, listtests, listdetails} RunMode;
	RunMode mode;		// Indicates whether we're generating
				// results, or comparing two previous runs.

	int verbosity;		// Verbosity level.  0 == concise; larger
				// values imply more verbose output.

	string db1Name;		// Name of output database, or one of
				// the two databases being compared.
				// Typically the pathname of a directory,
				// provided on the command line.

	string db2Name;		// Name of the second database being
				// compared.

	string visFilter;	// Filter constraining the set of visuals
				// (FBConfigs, pixel formats) that will be
				// available for test.  See
				// DrawingSurfaceFilter for description of
				// contents.

	vector<string> selectedTests;
				// Sorted list of tests to be executed.

	bool overwrite;		// overwrite old results database if exists
	bool ignorePrereqs; // ignore prerequisite tests
#if defined(__X11__)
	string dpyName;		// Name of the X11 display providing the
				// OpenGL implementation to be tested.
#elif defined(__WIN__)
#endif

	Options();
}; // class Options

} // namespace GLEAN

#endif // __options_h__
