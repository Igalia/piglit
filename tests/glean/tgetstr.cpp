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

// tgetstr.cpp:  implementation of OpenGL glGetString() tests

#include "tgetstr.h"
#include <algorithm>

using namespace std;

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////
void
GetStringTest::runOne(GetStringResult& r, Window&) {
	r.vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
	r.renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
	r.version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	r.extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
	r.pass = true;
} // GetStringTest::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
GetStringTest::logOne(GetStringResult& r) {
	logPassFail(r);
	logConcise(r);
	if (env->options.verbosity) {
		env->log << "\tvendor:     " << r.vendor << '\n';
		env->log << "\trenderer:   " << r.renderer << '\n';
		env->log << "\tversion:    " << r.version << '\n';
		env->log << "\textensions: " << r.extensions << '\n';
	}
} // GetStringTest::logOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
GetStringTest getStringTest("getString", "window",
	"This test checks the contents of the strings returned by\n"
	"glGetString():  the vendor name, renderer name, version, and\n"
	"extensions.  It is run on every OpenGL-capable drawing surface\n"
	"configuration that supports creation of a window.\n");

} // namespace GLEAN
