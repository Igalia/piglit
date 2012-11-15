// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 2000  Allen Akin   All Rights Reserved.
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

// tbinding.cpp:  Test functions in the window-system binding

#include "tbinding.h"
#include "image.h"
#include "rand.h"
#include <cmath>

#if 0
#ifdef __UNIX__
#include <unistd.h>
#endif

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include "dsconfig.h"
#include "dsfilt.h"
#include "dsurf.h"
#include "winsys.h"
#include "environ.h"
#include "rc.h"
#include "glutils.h"
#include "stats.h"
#include "tbinding.h"
#include "misc.h"
#endif

namespace {

bool
makeCurrentOK(GLEAN::DrawingSurfaceConfig& config) {
	using namespace GLEAN;
	float expected[4];
	glClear(GL_COLOR_BUFFER_BIT);
	glGetFloatv(GL_COLOR_CLEAR_VALUE, expected);
	Image probe(1, 1, GL_RGBA, GL_FLOAT);
	probe.read(drawingSize/2, drawingSize/2);
	const float* actual = reinterpret_cast<float*>(probe.pixels());
	double maxError = ErrorBits(fabs(expected[0] - actual[0]), config.r);
	maxError = max(maxError,
		ErrorBits(fabs(expected[1] - actual[1]), config.g));
	maxError = max(maxError,
		ErrorBits(fabs(expected[2] - actual[2]), config.b));
	return maxError <= 1.0;
} // makeCurrentOK

} // anonymous namespace

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////
void
MakeCurrentTest::runOne(MakeCurrentResult& r, Window& w) {

	DrawingSurfaceConfig& config = *(r.config);
	WindowSystem& ws = env->winSys;

	// The rendering contexts to be used:
	vector<RenderingContext*> rcs;

	RandomBitsDouble rRand(config.r, 712105);
	RandomBitsDouble gRand(config.g, 63230);
	RandomBitsDouble bRand(config.b, 912167);

	// Create rendering contexts to be used with the test window.
	// Note that the first context (at index 0) is always the
	// null context.

	rcs.push_back(0);
	r.descriptions.push_back("Null context");
	ws.makeCurrent();
	r.testSequence.push_back(static_cast<int>(rcs.size()) - 1);

	rcs.push_back(new RenderingContext(env->winSys, config, 0, true));
	r.descriptions.push_back("Direct-rendering context");
	ws.makeCurrent(*rcs.back(), w);
	r.testSequence.push_back(static_cast<int>(rcs.size()) - 1);
	glDisable(GL_DITHER);
	glClearColor(rRand.next(), gRand.next(), bRand.next(), 1.0);
	if (!makeCurrentOK(config))
		goto failed;

	rcs.push_back(new RenderingContext(env->winSys, config, 0, false));
	r.descriptions.push_back("Indirect-rendering context");
	ws.makeCurrent(*rcs.back(), w);
	r.testSequence.push_back(static_cast<int>(rcs.size()) - 1);
	glDisable(GL_DITHER);
	glClearColor(rRand.next(), gRand.next(), bRand.next(), 1.0);
	if (!makeCurrentOK(config))
		goto failed;

	// Now run through all the pairs of rendering contexts, making
	// them current in sequence and checking that rendering looks
	// correct.  Don't worry about the redundant sequences; we want
	// to check those, too!

	int i;
	for (i = 0; i < static_cast<int>(rcs.size()); ++i)
		for (int j = 0; j < static_cast<int>(rcs.size()); ++j) {
			r.testSequence.push_back(i);
			if (rcs[i] == 0)
				ws.makeCurrent();
			else {
				ws.makeCurrent(*rcs[i], w);
				if (!makeCurrentOK(config))
					goto failed;
			}
			r.testSequence.push_back(j);
			if (rcs[j] == 0)
				ws.makeCurrent();
			else {
				ws.makeCurrent(*rcs[j], w);
				if (!makeCurrentOK(config))
					goto failed;
			}
		}
	r.pass = true;
	goto cleanup;

failed:
	r.pass = false;
cleanup:
	for (i = 0; i < static_cast<int>(rcs.size()); ++i)
		if (rcs[i]) {
			// We need to make sure that no GL commands are
			// pending when the window is destroyed, or we
			// risk a GLXBadCurrentWindow error at some
			// indeterminate time in the future when
			// glXMakeCurrent() is executed.
			// In theory, if glReadPixels() is the last
			// command executed by a test, then an implicit
			// flush has occurred, and the command queue is
			// empty.  In practice, we have to protect
			// against the possibility that the implicit
			// flush is not enough to avoid the error.
			ws.makeCurrent(*rcs[i], w);
			glFinish();
			ws.makeCurrent();

			delete rcs[i];
		}
} // MakeCurrentTest::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
MakeCurrentTest::logOne(MakeCurrentResult& r) {
	logPassFail(r);
	logConcise(r);
	if (!r.pass) {
	    env->log << "\tSequence of MakeCurrent operations was:\n";
	    for (int k = 0; k < static_cast<int>(r.testSequence.size()); ++k)
		env->log << "\t\t"
			 << r.descriptions[r.testSequence[k]]
			 << '\n';
	}
} // MakeCurrentTestTest::logOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
MakeCurrentTest makeCurrentTest("makeCurrent", "window, rgb",

	"This test sanity-checks the ability to use multiple rendering\n"
	"contexts.  It creates several contexts with differing\n"
	"characteristics (e.g., some are direct-rendering and some\n"
	"are indirect-rendering, if the window system binding supports\n"
	"that distinction).  Then it runs through all pairs of contexts,\n"
	"making each one \"current\" in turn and verifying that simple\n"
	"rendering succeeds.\n"

	);

} // namespace GLEAN
