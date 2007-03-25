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




// test.h:  Base class for all tests

// This class encapsulates base functionality used by all tests.  Some
// of this is fairly trivial (the test name, for example).  One of the
// most important nontrivial functions is the use of the constructor
// to build a linked list of test objects; this eliminates the need to
// maintain a separate table of tests.  This class also provides a
// flag for determining if a test has been run, which allows tests to
// invoke one another and make use of previous results without forcing
// tests to run multiple times.  Finally, it provides a basic
// framework for recording a vector of results (which typically will
// vary depending on the drawing surface configuration or the
// particular type of drawing surface used).

// It is possible to derive test classes directly from this class.
// Most people will find it more convenient to use the BaseTest
// template class.  See tbase.h for more information.



#ifndef __test_h__
#define __test_h__

using namespace std;

#include <string>
#include <vector>
#include <fstream>

namespace GLEAN {

class Environment;		// Mutually-recursive and forward references.
class DrawingSurfaceConfig;

// Base class for a single test result.  A test may have many results
// (for example, one per drawing surface configuration), so in general
// individual tests will have a vector of these objects.
class Result {
public:
	virtual void put(ostream& s) const = 0;
	virtual bool get(istream& s) = 0;
	Result() { }
	virtual ~Result() { }
};

class Test {
    public:
	Test(const char* testName, const char *descrip);
	Test(const char* testName, const char *descrip, Test** prereqs);
	virtual ~Test();

	string name;		// Test name.  Should avoid characters
				// that aren't universally available in
				// filenames, since it might be used to
				// construct such names.

	string description;	// Verbose description of test.

	Test** prereqs;		// Pointer to array of prerequisite tests.
				// These will always be run before the
				// current test.

	bool hasRun;		// True if test has been run.

	Environment* env;	// Environment in which runs or comparisons
				// will be performed.

	virtual void run(Environment& env) = 0;	// Run test, save results.

	virtual void compare(Environment& env) = 0;

	virtual void details(Environment& env) = 0;
				// Compare two previous runs.

	// Exceptions:
	struct Error { };	// Base class for all exceptions.
	struct CantOpenResultsFile: public Error {
		const string& testName;
		const string& dbName;
		CantOpenResultsFile(const string& test, const string& db):
			testName(test), dbName(db) { }
	};


	// OutputStream and Input*Stream objects provide convenient access
	// to the results database, and close the file streams automatically
	// when their destructors are executed.
	class OutputStream {	// Open an output stream for storing results.
	    public:
		ofstream* s;
		OutputStream(Test& t);
		~OutputStream();
		operator ofstream& ();
	};
	class Input1Stream {	// Open db #1 input stream for reading results.
	    public:
		ifstream* s;
	    	Input1Stream(Test& t);
		~Input1Stream();
		operator ifstream& ();
	};
	class Input2Stream {	// Open db #2 input stream for reading results.
	    public:
		ifstream* s;
	    	Input2Stream(Test& t);
		~Input2Stream();
		operator ifstream& ();
	};


	static Test* testList;	// List of all test objects.  Built by
				// constructor Test::Test(...).

	Test* nextTest;		// Link to next test object.

	static int testCount;	// Count of elements in testList.
}; // class Test

} // namespace GLEAN

#endif // __test_h__
