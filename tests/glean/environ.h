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




// environ.h:  global test environment

// This class provides a facade for all the operating-system and
// window-system services that we need to run ``portable'' tests. 
// Examples include logging services, opening streams to read or write
// database files, and gaining access to the window system.


#ifndef __environ_h__
#define __environ_h__

#include <iostream>
#include "options.h"
#include "winsys.h"

namespace GLEAN {

class Image;			// Forward and mutually-recursive references.

class Environment {
    public:
    	// Constructors:
	Environment(Options& opt);

	// Exceptions:
	struct Error { };	// Base class for all errors.
	struct DBExists: public Error {		// Output DB already exists.
	};
	struct DBCantOpen: public Error {	// Can't open a DB.
		const string* db;
		DBCantOpen(string& s) {db = &s;}
	};

	// Members:
	Options options;	// Global testing options.

	ostream& log;		// Output stream used for logging results.

	WindowSystem winSys;	// The window system providing the OpenGL
				// implementation under test.

	string resultFileName(string& dbName, string& testName);
				// Return name of results file for given
				// test.  Suitable for opening a stream.
				// XXX Creates results directory as a side
				// effect.  Should separate this.
	inline string resultFileName(string& testName) {
		return resultFileName(options.db1Name, testName);
	}
}; // class Environment

} // namespace GLEAN

#endif // __environ_h__
