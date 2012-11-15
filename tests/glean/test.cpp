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




// test.cpp:  implementation of base class for tests
#ifdef __UNIX__
#include <unistd.h>
#endif

#include <iostream>
#include "dsconfig.h"
#include "dsurf.h"
#include "winsys.h"
#include "environ.h"
#include "test.h"

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// Class variables for automatic construction of list of all tests
///////////////////////////////////////////////////////////////////////////////
Test* Test::testList;		// Guaranteed initialized to zero at startup,
				// before any constructors are invoked.
				// (See discussion in section 10.4.9,
				// page 252, of ``C++ Programming Language''
				// (third edition).)

int Test::testCount;		// Also initialized to zero.

///////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor:
///////////////////////////////////////////////////////////////////////////////
Test::Test(const char* testName, const char *descrip):
    name(testName), description(descrip) {
	prereqs = 0;
	hasRun = false;
	env = NULL;
	nextTest = testList;
	testList = this;
	++testCount;
} // Test::Test()

Test::Test(const char* testName, const char *descrip, Test** thePrereqs):
    name(testName), description(descrip) {
	prereqs = thePrereqs;
	hasRun = false;
	env = NULL;
	nextTest = testList;
	testList = this;
	++testCount;
} // Test::Test()

Test::~Test() {
} // Test::~Test

} // namespace GLEAN
