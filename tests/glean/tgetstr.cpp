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
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
GetStringTest::compareOne(GetStringResult& oldR, GetStringResult& newR) {
	if (oldR.vendor == newR.vendor && oldR.renderer == newR.renderer
	 && oldR.version == newR.version && oldR.extensions == newR.extensions){
		if (env->options.verbosity)
			env->log << name << ":  SAME " <<
				newR.config->conciseDescription() << '\n';
	} else {
		env->log << name <<  ":  DIFF "
			<< newR.config->conciseDescription() << '\n';
		if (oldR.vendor != newR.vendor) {
			env->log << '\t' << env->options.db1Name
				<< " vendor: " << oldR.vendor;
			env->log << '\t' << env->options.db2Name
				<< " vendor: " << newR.vendor;
		}
		if (oldR.renderer != newR.renderer) {
			env->log << '\t' << env->options.db1Name
				<< " renderer: " << oldR.renderer;
			env->log << '\t' << env->options.db2Name
				<< " renderer: " << newR.renderer;
		}
		if (oldR.version != newR.version) {
			env->log << '\t' << env->options.db1Name
				<< " version: " << oldR.version;
			env->log << '\t' << env->options.db2Name
				<< " version: " << newR.version;
		}
		if (oldR.extensions != newR.extensions) {
			vector<string> oldExts;
			Lex oldLex(oldR.extensions.c_str());
			for (;;) {
				oldLex.next();
				if (oldLex.token == Lex::ID)
					oldExts.push_back(oldLex.id);
				else
					break;
			}
			sort(oldExts.begin(), oldExts.end());

			vector<string> newExts;
			Lex newLex(newR.extensions.c_str());
			for (;;) {
				newLex.next();
				if (newLex.token == Lex::ID)
					newExts.push_back(newLex.id);
				else
					break;
			}
			sort(newExts.begin(), newExts.end());

			vector<string> d(max(oldExts.size(), newExts.size()));
			vector<string>::iterator dEnd;

			dEnd = set_difference(oldExts.begin(), oldExts.end(),
				newExts.begin(), newExts.end(), d.begin());
			if (dEnd != d.begin()) {
				env->log << "\tExtensions in " <<
					env->options.db1Name << " but not in "
					<< env->options.db2Name << ":\n";
				for (vector<string>::iterator p = d.begin();
				    p != dEnd; ++p)
					env->log << "\t\t" << *p << '\n';
			}

			dEnd = set_difference(newExts.begin(), newExts.end(),
				oldExts.begin(), oldExts.end(), d.begin());
			if (dEnd != d.begin()) {
				env->log << "\tExtensions in " <<
					env->options.db2Name << " but not in "
					<< env->options.db1Name << ":\n";
				for (vector<string>::iterator p = d.begin();
				    p != dEnd; ++p)
					env->log << "\t\t" << *p << '\n';
			}

			dEnd = set_intersection(newExts.begin(), newExts.end(),
				oldExts.begin(), oldExts.end(), d.begin());
			if (dEnd != d.begin()) {
				env->log << "\tExtensions in both " <<
					env->options.db2Name << " and in "
					<< env->options.db1Name << ":\n";
				for (vector<string>::iterator p = d.begin();
				    p != dEnd; ++p)
					env->log << "\t\t" << *p << '\n';
			}
		}
	}
} // GetStringTest::compareOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
GetStringTest getStringTest("getString", "window",
	"This test checks the contents of the strings returned by\n"
	"glGetString():  the vendor name, renderer name, version, and\n"
	"extensions.  It is run on every OpenGL-capable drawing surface\n"
	"configuration that supports creation of a window.\n");

} // namespace GLEAN
