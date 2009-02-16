// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 1999-2000  Allen Akin   All Rights Reserved.
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

/*
tbase.h:  Base template class for (most) tests

In general, a glean test is an instance of a class derived from the
class Test.  It produces a vector of results, which are instances of a
class derived from the class Result.  Most glean tests are "portable"
in the sense that they don't contain OS- or window-system-specific
code; those things are abstracted by the Environment and WindowSystem
classes.

This file contains a template class and result class that serve as
bases for portable tests, and several macros that simplify test class
declarations.

The result class, BaseResult, includes utility functions that read and
write test results.  To use it, derive a new class from it, add
whatever variables you need to store your test results, and override
the getresults() and putresults() member functions to read and write
those variables from and to a stream.

The template class, BaseTest, is parameterized by the result class and
declares member functions and variables that are common to all
portable tests.  These include the member functions run() and
compare() which are invoked for each test by main().  BaseTest also
provides several variables which you might want to use when
constructing a test:

	A drawing surface filter string.  The test can be run on all
	the drawing surface configurations that are selected by the
	filter, and one result structure will be generated for each
	such configuration.

	A flag indicating whether the test is to be run on *all*
	drawing surface configurations, or just one.  For tests that
	take a long time to run, it is often sufficient to check just
	one drawing surface configuration rather than all of them.

	An extension filter string.  The test will only be run on
	contexts that support all the listed extensions.  Extension
	names in the string may be separated with non alphanumerics;
	whitespace and commas are used by convention.
	
	A description string.  This will be printed in the test log to
	describe the test.

	Default width and height for any windows to be created by the
	test.

	A pointer to an array of other tests that must be run before
	running the current test.  This makes the results of
	"prerequisite" tests available.

To use BaseTest, declare a new class derived from BaseTest,
parameterized by your result class.  Then override the runOne(),
compareOne(), and logOne() member functions.  runOne() runs a test and
generates a result.  compareOne() compares one result from a test run
with the same result from another test run.  logOne() generates a log
message summarizing the result of the test.

Your new test will need a few common declarations (such as
constructors).  To simplify writing them, this file provides a few
helper macros.  GLEAN_CLASS(TEST,RESULT) handles the declarations for
a test class named TEST and a result class named RESULT, using the
default values for window width and height and the run-once flag. 
GLEAN_CLASS_WH() and GLEAN_CLASS_WHO() allow you to specify the width,
height, and run-once flag if you choose.

Finally, declare an object using your new test class.  This object
must be global, so that its constructor will automatically add it to
the list of all tests.

You can find an example of this whole process in the files tbasic.h
and tbasic.cpp.

*/


#ifndef __tbase_h__
#define __tbase_h__

#ifdef __UNIX__
#include <unistd.h>
#endif

#include <iostream>
#include <fstream>
#include "dsconfig.h"
#include "dsfilt.h"
#include "dsurf.h"
#include "winsys.h"
#include "environ.h"
#include "rc.h"
#include "glutils.h"
#include "misc.h"

#include "test.h"

class GLEAN::DrawingSurfaceConfig;		// Forward reference.


// Macro for constructor for Glean test taking width, height and one config flag
#define GLEAN_CLASS_WHO(TEST, RESULT, WIDTH, HEIGHT, ONE)                     \
	TEST(const char* aName, const char* aFilter,                          \
	     const char* aDescription):                                       \
		BaseTest<RESULT>(aName, aFilter, aDescription) {              \
                fWidth  = WIDTH;                                              \
                fHeight = HEIGHT;                                             \
                testOne = ONE;                                                \
	}                                                                     \
	TEST(const char* aName, const char* aFilter, Test** thePrereqs,       \
	     const char* aDescription):                                       \
		BaseTest<RESULT>(aName, aFilter, thePrereqs, aDescription) {  \
                fWidth  = WIDTH;                                              \
                fHeight = HEIGHT;                                             \
                testOne = ONE;                                                \
	}                                                                     \
	TEST(const char* aName, const char* aFilter,                          \
	     const char* anExtensionList,                                     \
	     const char* aDescription):                                       \
		BaseTest<RESULT>(aName, aFilter, anExtensionList, aDescription) {     \
                fWidth  = WIDTH;                                              \
                fHeight = HEIGHT;                                             \
                testOne = ONE;                                                \
	}                                                                     \
	virtual ~TEST() {}                                                    \
                                                                              \
	virtual void runOne(RESULT& r, Window& w);                            \
	virtual void compareOne(RESULT& oldR, RESULT& newR);                  \
	virtual void logOne(RESULT& r)

// Macro for constructor for Glean test taking width, height
#define GLEAN_CLASS_WH(TEST, RESULT, WIDTH, HEIGHT) \
        GLEAN_CLASS_WHO(TEST, RESULT, WIDTH, HEIGHT, false)

// Macro for constructor for Glean test taking only test class and result class
#define GLEAN_CLASS(TEST, RESULT) \
        GLEAN_CLASS_WHO(TEST, RESULT, 258, 258, false)



namespace GLEAN {

class BaseResult : public Result {
	// Class for a single test result.  All basic tests have a
	// drawing surface configuration, plus other information
	// that's specific to the test.
public:
	DrawingSurfaceConfig* config;

	virtual void putresults(ostream& s) const = 0;
	virtual bool getresults(istream& s) = 0;
	
	virtual void put(ostream& s) const {
		s << config->canonicalDescription() << '\n';
		putresults(s);
	}
	
	virtual bool get(istream& s) {
		SkipWhitespace(s);
		string configDesc;
		if (!getline(s, configDesc)) return false;
		config = new DrawingSurfaceConfig(configDesc);
		return getresults(s);
	}
};


// The BaseTest class is a templatized class taking a ResultType as a parameter
template <class ResultType> class BaseTest: public Test {
public:
	BaseTest(const char* aName, const char* aFilter,
		 const char* aDescription): Test(aName, aDescription) {
		filter      = aFilter;
		extensions  = 0;
		description = aDescription;
		fWidth      = 258;
		fHeight     = 258;
		testOne     = false; 
	}
	BaseTest(const char* aName, const char* aFilter, Test** thePrereqs,
		 const char* aDescription):
			Test(aName, aDescription, thePrereqs) {
		filter      = aFilter;
		extensions  = 0;
		description = aDescription;
		fWidth      = 258;
		fHeight     = 258;
		testOne     = false; 
	}
	BaseTest(const char* aName, const char* aFilter,
		 const char* anExtensionList,
		 const char* aDescription): Test(aName, aDescription) {
		filter      = aFilter;
		extensions  = anExtensionList;
		description = aDescription;
		fWidth      = 258;
		fHeight     = 258;
		testOne     = false;
	}

	virtual ~BaseTest() {}

	const char*         filter;	 // Drawing surface config filter.
	const char*         extensions;  // Required extensions.
	int                 fWidth;	 // Drawing surface width.
	int                 fHeight;     // Drawing surface height.
	bool                testOne;     // Test only one config?
	vector<ResultType*> results;     // Test results.

	virtual void runOne(ResultType& r, Window& w) = 0;
	virtual void compareOne(ResultType& oldR, ResultType& newR) = 0;
	virtual void logOne(ResultType& r) = 0;

	virtual vector<ResultType*> getResults(istream& s) {
		vector<ResultType*> v;
		while (s.good()) {
			ResultType* r = new ResultType();
			if (r->get(s))
				v.push_back(r);
			else {
				delete r;
				break;
			}
		}
		return v;
	}
	
	virtual void logDescription() {
		if (env->options.verbosity)
			env->log <<
"----------------------------------------------------------------------\n"
				 << description
				 << '\n';
	}

	// This method allows a test to indicate that it's not applicable.
	// For example, the GL version is too low.
	virtual bool isApplicable() const {
		return true;
	}

	virtual void run(Environment& environment) {
		if (hasRun)
			return; // no multiple invocations

		// Invoke the prerequisite tests, if any:
		for (Test** t = prereqs; t != 0 && *t != 0; ++t)
			(*t)->run(environment);
		env = &environment; // make environment available
		logDescription();   // log invocation
		WindowSystem& ws = env->winSys;

		try {
			OutputStream os(*this);	// open results file

			// Select the drawing configurations for testing
			DrawingSurfaceFilter f(filter);
			vector<DrawingSurfaceConfig*>
				configs(f.filter(ws.surfConfigs));

			// Test each config
			for (vector<DrawingSurfaceConfig*>::const_iterator
				     p = configs.begin();
			     p < configs.end();
			     ++p) {
				Window w(ws, **p, fWidth, fHeight);
				RenderingContext rc(ws, **p);
				if (!ws.makeCurrent(rc, w)) {
					// XXX need to throw exception here
				}

				// Check if test is applicable to this context
				if (!isApplicable())
					continue;

				// Check for all prerequisite extensions.  Note
				// that this must be done after the rendering
				// context has been created and made current!
				if (!GLUtils::haveExtensions(extensions))
					continue;

				// Create a result object and run the test:
				ResultType* r = new ResultType();
				r->config = *p;
				runOne(*r, w);
				logOne(*r);
				
				// Save the result
				results.push_back(r);
				r->put(os);

				// if testOne, skip remaining surface configs
				if (testOne)
					break;
			}
		}
		catch (DrawingSurfaceFilter::Syntax e) {
			env->log << "Syntax error in test's drawing-surface"
				 << " selection criteria:\n'"
				 << filter
				 << "'\n";
			for (int i = 0; i < e.position; ++i)
				env->log << ' ';
			env->log << "^ " << e.err << '\n';
		}
		catch (RenderingContext::Error) {
			env->log << "Could not create a rendering context\n";
		}
		env->log << '\n';
		
		hasRun = true;	// Note that we've completed the run
	}
	
	virtual void compare(Environment& environment) {
		env = &environment; // Save the environment
		logDescription();
		// Read results from previous runs:
		Input1Stream is1(*this);
		vector<ResultType*> oldR(getResults(is1));
		Input2Stream is2(*this);
		vector<ResultType*> newR(getResults(is2));

		// Construct a vector of surface configurations from the
		// old run.  (Later we'll find the best match in this
		// vector for each config in the new run.)
		vector<DrawingSurfaceConfig*> oldConfigs;
		for (typename vector<ResultType*>::const_iterator p = oldR.begin();
		     p < oldR.end();
		     ++p)
			oldConfigs.push_back((*p)->config);

		// Compare results:
		for (typename vector<ResultType*>::const_iterator newP = newR.begin();
		     newP < newR.end();
		     ++newP) {

			// Find the drawing surface config that most
			// closely matches the config for this result:
			int c = (*newP)->config->match(oldConfigs);

			// If there was a match, compare the results:
			if (c < 0)
				env->log << name
					 << ":  NOTE no matching config for "
					 << (*newP)
					->config->conciseDescription()
					 << '\n';
			else
				compareOne(*(oldR[c]), **newP);
		}

		// Get rid of the results; we don't need them for future
		// comparisons.
		for (typename vector<ResultType*>::iterator np = newR.begin();
		     np < newR.end();
		     ++np)
			delete *np;
		for (typename vector<ResultType*>::iterator op = oldR.begin();
		     op < oldR.end();
		     ++op)
			delete *op;
	}

	// comparePassFail is a helper function for tests that have a
	// boolean result as all or part of their ResultType
	virtual void comparePassFail(ResultType& oldR, ResultType& newR) {
		if (oldR.pass == newR.pass) {
			if (env->options.verbosity)
				env->log << name
					 << ":  SAME "
					 << newR.config->conciseDescription()
					 << '\n'
					 << (oldR.pass
					     ? "\tBoth PASS\n"
					     : "\tBoth FAIL\n");
		} else {
			env->log << name
				 << ":  DIFF "
				 << newR.config->conciseDescription()
				 << '\n'
				 << '\t'
				 << env->options.db1Name
				 << (oldR.pass? " PASS, ": " FAIL, ")
				 << env->options.db2Name
				 << (newR.pass? " PASS\n": " FAIL\n");
		}
	}

	virtual void logPassFail(ResultType& r) {
		env->log << name << (r.pass ? ":  PASS ": ":  FAIL ");
	}

	virtual void logConcise(ResultType& r) {
		env->log << r.config->conciseDescription() << '\n';
	}
}; // class BaseTest

} // namespace GLEAN

#endif // __tbasic_h__
