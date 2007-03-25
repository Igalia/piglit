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

// trgbtris.cpp:  example image-based test to show use of TIFF images

#include "trgbtris.h"
#include "stats.h"
#include "rand.h"
#include "geomutil.h"
#include "image.h"

#if 0
#if defined __UNIX__
#include <unistd.h>
#endif

#include <iostream>
#include <fstream>
#include <algorithm>
#include "dsconfig.h"
#include "dsfilt.h"
#include "dsurf.h"
#include "winsys.h"
#include "environ.h"
#include "rc.h"
#include "glutils.h"
#include "trgbtris.h"
#include "misc.h"
#endif

namespace {

void
logStats(GLEAN::BasicStats& stats, GLEAN::Environment* env) {
	env->log << "\t\tmin = " << stats.min() << ", max = " << stats.max()
		<< "\n\t\tmean = " << stats.mean() << ", standard deviation = "
		<< stats.deviation() << '\n';
}

} // anonymous namespace

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////
void
RGBTriStripTest::runOne(RGBTriStripResult& r, Window& w) {
	static int this_config = 0;
	r.imageNumber = ++this_config;
	
	GLUtils::useScreenCoords(drawingSize + 2, drawingSize + 2);

	int nPoints = 20;	// Exact value doesn't really matter.
	RandomDouble vRand(142857);
	RandomMesh2D v(1.0, drawingSize, nPoints, 1.0, drawingSize, nPoints,
		vRand);

	RandomDouble cRand(271828);

	glClear(GL_COLOR_BUFFER_BIT);
	glShadeModel(GL_SMOOTH);

	for (int row = 0; row < nPoints - 1; ++row) {
		glBegin(GL_TRIANGLE_STRIP);
			for (int col = 0; col < nPoints; ++col) {
				float r = cRand.next();
				float g = cRand.next();
				float b = cRand.next();
				glColor3f(r, g, b);
				glVertex2fv(v(row, col));
				r = cRand.next();
				g = cRand.next();
				b = cRand.next();
				glColor3f(r, g, b);
				glVertex2fv(v(row + 1, col));
			}
		glEnd();
	}
	w.swap();

	Image image(drawingSize + 2, drawingSize + 2, GL_RGB, GL_FLOAT);
	image.read(0, 0);	// Invoke glReadPixels to read the image.
	image.writeTIFF(env->imageFileName(name, r.imageNumber));

	r.pass = true;
} // RGBTriStripTest::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
RGBTriStripTest::logOne(RGBTriStripResult& r) {
	env->log << name << ":  NOTE "
		 << r.config->conciseDescription() << '\n'
		 << "\tImage number " << r.imageNumber << '\n';
	if (env->options.verbosity)
		env->log <<
		   "\tThis test does not check its result.  Please view\n"
		   "\tthe image to verify that the result is correct, or\n"
		   "\tcompare it to a known-good result from a different\n"
		   "\trun of glean.\n";
} // RGBTriStripTest::logOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
RGBTriStripTest::compareOne(RGBTriStripResult& oldR, RGBTriStripResult& newR) {
	// Fetch the old and new images:
	Image oldI;
	oldI.readTIFF(env->image1FileName(name, oldR.imageNumber));
	Image newI;
	newI.readTIFF(env->image2FileName(name, newR.imageNumber));

	// Register the images, and gather statistics about the differences
	// for each color channel:
	Image::Registration reg(oldI.reg(newI));

	// Compute worst-case tolerance (1 LSB in the shallowest drawing
	// surface configuration) for each color channel:
	double rTol = 1.0 / (1 << min(oldR.config->r, newR.config->r));
	double gTol = 1.0 / (1 << min(oldR.config->g, newR.config->g));
	double bTol = 1.0 / (1 << min(oldR.config->b, newR.config->b));

	// We'll conclude that the images are the ``same'' if the maximum
	// absolute error is no more than 1 LSB (in the shallowest config):
	if (reg.stats[0].max() <= rTol && reg.stats[1].max() <= gTol
	 && reg.stats[2].max() <= bTol) {
		if (env->options.verbosity) {
			env->log << name << ": SAME "
				<< newR.config->conciseDescription() << '\n';
			if (reg.stats[0].max() == 0 && reg.stats[1].max() == 0
			 && reg.stats[1].max() == 0)
				env->log << "\tImages are exactly equal\n";
			else
				env->log << "\tImages are approximately equal\n";
		}
	} else {
		env->log << name << ":  DIFF "
			<< newR.config->conciseDescription() << '\n'
			<< "\tDifference exceeds 1 LSB in at least one "
			   "color channel\n";
	}
	if (env->options.verbosity) {
		env->log << "\tred:\n";
		logStats(reg.stats[0], env);
		env->log << "\tgreen:\n";
		logStats(reg.stats[1], env);
		env->log << "\tblue:\n";
		logStats(reg.stats[2], env);
	}
} // RGBTriStripTest::compareOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
RGBTriStripTest rgbTriStripTest("rgbTriStrip", "window, rgb",

	"The best approach when designing a test is to make it\n"
	"self-checking; that is, the test itself should determine\n"
	"whether the image or other data structure that it produces is\n"
	"correct.  However, some tests are difficult to design in this\n"
	"way, and for some other tests (like stress tests or regression\n"
	"tests concerning previously-reported bugs) it may be\n"
	"unnecessary.  For such tests, glean provides mechanisms to\n"
	"save images and compare them to images generated from other\n"
	"runs.  This test simply exercises those mechanisms.\n");


} // namespace GLEAN
