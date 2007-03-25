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

// torthopos.cpp:  Test positioning of primitives in orthographic projection.
// Some applications use OpenGL extensively for 2D rendering:  portable
// GUI toolkits, heads-up display generators, etc.  These apps require
// primitives to be drawn with reliable position and size in orthographic
// projections.  There are some potential pitfalls; for a good discussion,
// see the OpenGL Programming Guide (the Red Book).  In the second edition,
// see the OpenGL Correctness Tips on page 601.

#include "torthpos.h"
#include "image.h"
#include "rand.h"
#include "geomutil.h"

#if 0
#ifdef __UNIX__
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
#include "torthpos.h"
#include "misc.h"
#endif


namespace {

void
logStats1(const char* title, GLEAN::OPResult& r,
    GLEAN::Environment* env) {
	env->log << '\t' << title << ": ";
	if (r.hasGaps || r.hasOverlaps || r.hasBadEdges) {
		env->log << (r.hasGaps? " Gaps.": "")
			<< (r.hasOverlaps? " Overlaps.": "")
			<< (r.hasBadEdges? " Incorrect edges.": "")
			<< '\n';
	} else {
		env->log << " No gaps, overlaps, or incorrect edges.\n";
	}
} // logStats1

void
diffHeader(bool& same, const string& name,
    GLEAN::DrawingSurfaceConfig* config, GLEAN::Environment* env) {
	if (same) {
		same = false;
		env->log << name << ":  DIFF "
			<< config->conciseDescription() << '\n';
	}
} // diffHeader

void
failHeader(bool& pass, const string& name,
    GLEAN::DrawingSurfaceConfig* config, GLEAN::Environment* env) {
	if (pass) {
		pass = false;
		env->log << name << ":  FAIL "
			<< config->conciseDescription() << '\n';
	}
} // failHeader

void
doComparison(const GLEAN::OPResult& oldR,
    const GLEAN::OPResult& newR,
    GLEAN::DrawingSurfaceConfig* config,
    bool& same,
    const string& name,
    GLEAN::Environment* env,
    const char* title) {
	if (newR.hasGaps != oldR.hasGaps) {
		diffHeader(same, name, config, env);
		env->log << '\t' << env->options.db1Name
			<< ' ' << title << ' '
			<< (oldR.hasGaps? " has": " does not have")
			<< " gaps\n";
		env->log << '\t' << env->options.db2Name
			<< ' ' << title << ' '
			<< (newR.hasGaps? " has": " does not have")
			<< " gaps\n";
	}
	if (newR.hasOverlaps != oldR.hasOverlaps) {
		diffHeader(same, name, config, env);
		env->log << '\t' << env->options.db1Name
			<< ' ' << title << ' '
			<< (oldR.hasOverlaps? " has": " does not have")
			<< " overlaps\n";
		env->log << '\t' << env->options.db2Name
			<< ' ' << title << ' '
			<< (newR.hasOverlaps? " has": " does not have")
			<< " overlaps\n";
	}
	if (newR.hasBadEdges != oldR.hasBadEdges) {
		diffHeader(same, name, config, env);
		env->log << '\t' << env->options.db1Name
			<< ' ' << title << ' '
			<< (oldR.hasBadEdges? " has": " does not have")
			<< " incorrect edges\n";
		env->log << '\t' << env->options.db2Name
			<< ' ' << title << ' '
			<< (newR.hasBadEdges? " has": " does not have")
			<< " incorrect edges\n";
	}
} // doComparison

GLubyte
logicalSum(GLubyte* start, int stride, int count) {
	GLubyte* p = start;
	GLubyte sum = 0;
	for (int i = 0; i < count; ++i) {
		sum |= p[0];
		sum |= p[1];
		sum |= p[2];
		p += stride;
	}
	return sum;
}

void
verifyOrthPos(GLEAN::Window& w, bool& passed, string& name,
    GLEAN::DrawingSurfaceConfig* config, GLEAN::OPResult& res,
    GLEAN::Environment* env, const char* title) {

	GLEAN::Image img(windowSize, windowSize, GL_RGB, GL_UNSIGNED_BYTE);
	img.read(0, 0);
	w.swap();	// give the user something to watch

	// All of the tests in this group are constructed so that the
	// "correct" image covers a square of exactly drawingSize by
	// drawingSize pixels, embedded in a window that's two pixels
	// larger in both dimensions.  The border consists of pixels
	// with all components set to zero.  Within the image, all
	// pixels should be either red (only the red component is
	// nonzero) or green (only the green component is nonzero).  If
	// any pixels with all zero components are found, that indicates
	// the presence of gaps.  If any pixels with both red and green
	// nonzero components are found, that indicates the presence of
	// overlaps.

	res.hasGaps = false;
	res.hasOverlaps = false;
	res.hasBadEdges = false;

	GLubyte* row0 = reinterpret_cast<GLubyte*>(img.pixels());
	GLubyte* row1 = row0 + img.rowSizeInBytes();
	GLubyte* rowLast = row0 + (windowSize - 1) * img.rowSizeInBytes();
	GLubyte* rowNextLast = rowLast - img.rowSizeInBytes();

	// Check the bottom horizontal edge; it must be all zero.
	if (logicalSum(row0, 3, windowSize)) {
		failHeader(passed, name, config, env);
		env->log << '\t' << title
			<< ":  bottom border (at Y==0) was touched\n";
		res.hasBadEdges = true;
	}
	// Repeat the process for the top horizontal edge.
	if (logicalSum(rowLast, 3, windowSize)) {
		failHeader(passed, name, config, env);
		env->log << '\t' << title
			<< ":  top border (at Y==" << windowSize - 1
			<< ") was touched\n";
		res.hasBadEdges = true;
	}
	// Check the second row; there must be at least one nonzero
	// pixel in the "drawn" region (excluding the first and last
	// column).
	if (!logicalSum(row1 + 3/*skip first pixel's RGB*/, 3, drawingSize)) {
		failHeader(passed, name, config, env);
		env->log << '\t' << title
			<< ":  first row (at Y==1) was not drawn\n";
		res.hasBadEdges = true;
	}
	// Repeat the process for the last row.
	if (!logicalSum(rowNextLast + 3, 3, drawingSize)) {
		failHeader(passed, name, config, env);
		env->log << '\t' << title
			<< ":  last row (at Y==" << windowSize - 2
			<< ") was not drawn\n";
		res.hasBadEdges = true;
	}

	// Check the left-hand vertical edge; it must be all zero.
	if (logicalSum(row0, img.rowSizeInBytes(), windowSize)) {
		failHeader(passed, name, config, env);
		env->log << '\t' << title
			<< ":  left border (at X==0) was touched\n";
		res.hasBadEdges = true;
	}
	// Repeat for the right-hand vertical edge.
	if (logicalSum(row0 + 3 * (windowSize - 1), img.rowSizeInBytes(),
	    windowSize)) {
		failHeader(passed, name, config, env);
		env->log << '\t' << title
			<< ":  right border (at X==" << windowSize - 1
			<< ") was touched\n";
		res.hasBadEdges = true;
	}
	// Check the left-hand column; something must be nonzero.
	if (!logicalSum(row1 + 3, img.rowSizeInBytes(), drawingSize)) {
		failHeader(passed, name, config, env);
		env->log << '\t' << title
			<< ":  first column (at X==1) was not drawn\n";
		res.hasBadEdges = true;
	}
	// And repeat for the right-hand column:
	if (!logicalSum(row1 + 3 * (drawingSize - 1), img.rowSizeInBytes(),
	    drawingSize)) {
		failHeader(passed, name, config, env);
		env->log << '\t' << title
			<< ":  last column (at X==" << windowSize - 2
			<< ") was not drawn\n";
		res.hasBadEdges = true;
	}
	
	// Scan the drawing area.  Anytime we find a pixel with all zero
	// components, that's a gap.  Anytime we find a pixel with both
	// red and green components nonzero, that's an overlap.
	GLubyte* row = row1 + 3;	// lower-left pixel in drawing area
	for (int i = 0; i < drawingSize; ++i) {
		GLubyte* p = row;
		for (int j = 0; j < drawingSize; ++j) {
			if (!p[0] && !p[1] && !p[2]) {
				if (!res.hasGaps) {
					failHeader(passed, name, config, env);
					env->log << '\t' << title
						<< ":  found first gap at X=="
						<< j + 1 << ", Y==" << i + 1
						<< '\n';
					res.hasGaps = true;
				}
			}
			if (p[0] && p[1]) {
				if (!res.hasOverlaps) {
					failHeader(passed, name, config, env);
					env->log << '\t' << title
						<< ":  found first overlap at "
						<< "X==" << j + 1 << ", Y=="
						<< i + 1 << '\n';
					res.hasOverlaps = true;
				}
			}
			p += 3;
		}
		row += img.rowSizeInBytes();
	}

} // verifyOrthPos

void
subdivideRects(int minX, int maxX, int minY, int maxY,
    GLEAN::RandomDouble& rand, bool splitHoriz, bool drawInRed) {
	// Basically we're just splitting the input rectangle
	// recursively.  At each step we alternate between splitting
	// horizontally (dividing along Y) or vertically (along X).  We
	// also toggle colors (between red and green) at various times,
	// in order to give us some adjacent edges of different colors
	// that we can check for overlaps.  Recursion bottoms out when
	// the axis of interest drops below 30 pixels in length.
	//
	int min = splitHoriz? minY: minX;
	int max = splitHoriz? maxY: maxX;
	if (min + 30 > max) {
		glColor4f(drawInRed? 1.0: 0.0, drawInRed? 0.0: 1.0,
			0.0, 0.5);
		glBegin(GL_QUADS);
		glVertex2i(minX, minY);
		glVertex2i(maxX, minY);
		glVertex2i(maxX, maxY);
		glVertex2i(minX, maxY);
		glEnd();
		return;
	}

	int split = min + static_cast<int>((max - min) * rand.next());
	if (splitHoriz) {
		subdivideRects(minX, maxX, minY, split,
			rand, !splitHoriz, drawInRed);
		subdivideRects(minX, maxX, split, maxY,
			rand, !splitHoriz, !drawInRed);
	} else {
		subdivideRects(minX, split, minY, maxY,
			rand, !splitHoriz, drawInRed);
		subdivideRects(split, maxX, minY, maxY,
			rand, !splitHoriz, !drawInRed);
	}
}

} // anonymous namespace

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////
void
OrthoPosPoints::runOne(OPResult& r, Window& w) {
	bool passed = true;

	GLUtils::useScreenCoords(windowSize, windowSize);

	glFrontFace(GL_CCW);

	glDisable(GL_LIGHTING);

	glDisable(GL_FOG);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_DITHER);
	glDisable(GL_COLOR_LOGIC_OP);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDisable(GL_POLYGON_STIPPLE);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glShadeModel(GL_FLAT);

	glClearColor(0, 0, 0, 0);

	////////////////////////////////////////////////////////////
	// Immediate-mode points
	////////////////////////////////////////////////////////////

	// Clear both front and back buffers and swap, to avoid confusing
	// this test with results of the previous test:
	glClear(GL_COLOR_BUFFER_BIT);
	w.swap();
	glClear(GL_COLOR_BUFFER_BIT);
	{
		glBegin(GL_POINTS);
		for (int x = 1; x <= drawingSize; ++x)
			for (int y = 1; y <= drawingSize; ++y) {
				if ((x ^ y) & 1)
					glColor4f(0.0, 1.0, 0.0, 0.5);
				else
					glColor4f(1.0, 0.0, 0.0, 0.5);
				glVertex2i(x, y);
			}
		glEnd();
	}
	verifyOrthPos(w, passed, name, r.config, r, env, "Immediate-mode points");
	r.pass = passed;
} // OrthoPosPoints::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
OrthoPosPoints::logOne(OPResult& r) {
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	} else {
		env->log << '\n';
	}
	logStats(r);
} // OrthoPosPoints::logOne

void
OrthoPosPoints::logStats(OPResult& r) {
	logStats1("Immediate-mode points", r, env);
} // OrthoPosPoints::logStats

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
OrthoPosPoints::compareOne(OPResult& oldR, OPResult& newR) {
	bool same = true;

	doComparison(oldR, newR, newR.config, same, name,
		     env, "immediate-mode points");

	if (same && env->options.verbosity) {
		env->log << name << ":  SAME "
			 << newR.config->conciseDescription()
			 << "\n";
	}

	if (env->options.verbosity) {
		env->log << env->options.db1Name << ':';
		logStats(oldR);
		env->log << env->options.db2Name << ':';
		logStats(newR);
	}
} // OrthoPosPoints::compareOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
OrthoPosPoints orthoPosPointsTest("orthoPosPoints",
	"window, rgb > 1, z, fast",

	"This test checks the positioning of unit-sized points under\n"
	"orthographic projection.  (This is important for apps that\n"
	"want to use OpenGL for precise 2D drawing.)  It fills in an\n"
	"entire rectangle one pixel at a time, drawing adjacent pixels\n"
	"with different colors and with blending enabled.  If there are\n"
	"gaps (pixels that are the background color, and thus haven't\n"
	"been filled), overlaps (pixels that show a blend of more than\n"
	"one color), or improper edges (pixels around the edge of the\n"
	"rectangle that haven't been filled, or pixels just outside the\n"
	"edge that have), then the test fails.\n"
	"\n"
	"This test generally fails for one of several reasons.  First,\n"
	"the coordinate transformation process may have an incorrect bias;\n"
	"this usually will cause a bad edge.  Second, the coordinate\n"
	"transformation process may round pixel coordinates incorrectly;\n"
	"this will usually cause gaps and/or overlaps.  Third, the point\n"
	"rasterization process may not be filling the correct pixels;\n"
	"this can cause gaps, overlaps, or bad edges.\n"

	);




///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////

void
OrthoPosVLines::runOne(OPResult& r, Window& w) {
	bool passed = true;

	GLUtils::useScreenCoords(windowSize, windowSize);

	glFrontFace(GL_CCW);

	glDisable(GL_LIGHTING);

	glDisable(GL_FOG);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_DITHER);
	glDisable(GL_COLOR_LOGIC_OP);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDisable(GL_POLYGON_STIPPLE);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glShadeModel(GL_FLAT);

	glClearColor(0, 0, 0, 0);

	////////////////////////////////////////////////////////////
	// Immediate-mode vertical lines
	// 	Note that these are a little tricky, because of
	// 	OpenGL's "diamond-exit rule" line semantics.  In
	// 	this case, we can safely treat them as half-open
	// 	lines, where the terminal point isn't drawn.  Thus
	// 	we need to specify a terminal coordinate one pixel
	// 	beyond the last pixel we wish to be drawn.
	////////////////////////////////////////////////////////////

	// Clear both front and back buffers and swap, to avoid confusing
	// this test with results of the previous test:
	glClear(GL_COLOR_BUFFER_BIT);
	w.swap();
	glClear(GL_COLOR_BUFFER_BIT);
	{
		glBegin(GL_LINES);
		for (int x = 1; x <= drawingSize; ++x) {
			if (x & 1)
				glColor4f(0.0, 1.0, 0.0, 0.5);
			else
				glColor4f(1.0, 0.0, 0.0, 0.5);
			glVertex2i(x, 1);
			glVertex2i(x, drawingSize + 1);
			}
		glEnd();
	}
	verifyOrthPos(w, passed, name, r.config, r, env,
		"Immediate-mode vertical lines");
	r.pass = passed;
} // OrthoPosVLines::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
OrthoPosVLines::logOne(OPResult& r) {
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	} else {
		env->log << '\n';
	}
	logStats(r);
} // OrthoPosVLines::logOne

void
OrthoPosVLines::logStats(OPResult& r) {
	logStats1("Immediate-mode vertical lines", r, env);
} // OrthoPosVLines::logStats

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
OrthoPosVLines::compareOne(OPResult& oldR, OPResult& newR) {
	bool same = true;

	doComparison(oldR, newR, newR.config, same, name,
		env, "immediate-mode vertical lines");

	if (same && env->options.verbosity) {
		env->log << name << ":  SAME "
			<< newR.config->conciseDescription()
			<< "\n";
	}

	if (env->options.verbosity) {
		env->log << env->options.db1Name << ':';
		logStats(oldR);
		env->log << env->options.db2Name << ':';
		logStats(newR);
	}
} // OrthoPosVLines::compareOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
OrthoPosVLines orthoPosVLinesTest("orthoPosVLines",
	"window, rgb > 1, z, fast",

	"This test checks the positioning of unit-width vertical lines\n"
	"under orthographic projection.	(This is important for apps\n"
	"that want to use OpenGL for precise 2D drawing.)  It fills in\n"
	"an entire rectangle with a collection of vertical lines, drawing\n"
	"adjacent lines with different colors and with blending enabled.\n"
	"If there are gaps (pixels that are the background color, and\n"
	"thus haven't been filled), overlaps (pixels that show a blend\n"
	"of more than one color), or improper edges (pixels around the\n"
	"edge of the rectangle that haven't been filled, or pixels just\n"
	"outside the edge that have), then the test fails.\n"
	"\n"
	"This test generally fails for one of several reasons.  First,\n"
	"the coordinate transformation process may have an incorrect bias;\n"
	"this usually will cause a bad edge.  Second, the coordinate\n"
	"transformation process may round pixel coordinates incorrectly;\n"
	"this will usually cause gaps and/or overlaps.  Third, the\n"
	"line rasterization process may not be filling the correct\n"
	"pixels; this can cause gaps, overlaps, or bad edges.  Fourth,\n"
	"the OpenGL implementation may not handle the diamond-exit rule\n"
	"(section 3.4.1 in version 1.2.1 of the OpenGL spec) correctly;\n"
	"this should cause a bad border or bad top edge.\n"
	"\n"
	"It can be argued that this test is more strict that the OpenGL\n"
	"specification requires.  However, it is necessary to be this\n"
	"strict in order for the results to be useful to app developers\n"
	"using OpenGL for 2D drawing.\n"

	);




///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////

void
OrthoPosHLines::runOne(OPResult& r, Window& w) {
	bool passed = true;

	GLUtils::useScreenCoords(windowSize, windowSize);

	glFrontFace(GL_CCW);

	glDisable(GL_LIGHTING);

	glDisable(GL_FOG);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_DITHER);
	glDisable(GL_COLOR_LOGIC_OP);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDisable(GL_POLYGON_STIPPLE);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glShadeModel(GL_FLAT);

	glClearColor(0, 0, 0, 0);

	////////////////////////////////////////////////////////////
	// Immediate-mode horizontal lines
	// 	See the comments in the vertical line case above.
	////////////////////////////////////////////////////////////

	// Clear both front and back buffers and swap, to avoid confusing
	// this test with results of the previous test:
	glClear(GL_COLOR_BUFFER_BIT);
	w.swap();
	glClear(GL_COLOR_BUFFER_BIT);
	{
		glBegin(GL_LINES);
		for (int y = 1; y <= drawingSize; ++y) {
			if (y & 1)
				glColor4f(0.0, 1.0, 0.0, 0.5);
			else
				glColor4f(1.0, 0.0, 0.0, 0.5);
			glVertex2i(1, y);
			glVertex2i(drawingSize + 1, y);
			}
		glEnd();
	}
	verifyOrthPos(w, passed, name, r.config, r, env,
	       "Immediate-mode horizontal lines");
	r.pass = passed;
} // OrthoPosHLines::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
OrthoPosHLines::logOne(OPResult& r) {
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	} else {
		env->log << '\n';
	}
	logStats(r);
} // OrthoPosHLines::logOne

void
OrthoPosHLines::logStats(OPResult& r) {
	logStats1("Immediate-mode horizontal lines", r, env);
} // OrthoPosHLines::logStats

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
OrthoPosHLines::compareOne(OPResult& oldR, OPResult& newR) {
	bool same = true;

	doComparison(oldR, newR, newR.config, same, name,
		env, "immediate-mode horizontal lines");

	if (same && env->options.verbosity) {
		env->log << name << ":  SAME "
			<< newR.config->conciseDescription()
			<< "\n";
	}

	if (env->options.verbosity) {
		env->log << env->options.db1Name << ':';
		logStats(oldR);
		env->log << env->options.db2Name << ':';
		logStats(newR);
	}
} // OrthoPosHLines::compareOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
OrthoPosHLines orthoPosHLinesTest("orthoPosHLines",
	"window, rgb > 1, z, fast",

	"This test checks the positioning of unit-width horizontal lines\n"
	"under orthographic projection.	(This is important for apps\n"
	"that want to use OpenGL for precise 2D drawing.)  It fills in\n"
	"an entire rectangle with a stack of horizontal lines, drawing\n"
	"adjacent lines with different colors and with blending enabled.\n"
	"If there are gaps (pixels that are the background color, and\n"
	"thus haven't been filled), overlaps (pixels that show a blend\n"
	"of more than one color), or improper edges (pixels around the\n"
	"edge of the rectangle that haven't been filled, or pixels just\n"
	"outside the edge that have), then the test fails.\n"
	"\n"
	"This test generally fails for one of several reasons.  First,\n"
	"the coordinate transformation process may have an incorrect bias;\n"
	"this usually will cause a bad edge.  Second, the coordinate\n"
	"transformation process may round pixel coordinates incorrectly;\n"
	"this will usually cause gaps and/or overlaps.  Third, the\n"
	"line rasterization process may not be filling the correct\n"
	"pixels; this can cause gaps, overlaps, or bad edges.  Fourth,\n"
	"the OpenGL implementation may not handle the diamond-exit rule\n"
	"(section 3.4.1 in version 1.2.1 of the OpenGL spec) correctly;\n"
	"this should cause a bad border or bad right edge.\n"
	"\n"
	"It can be argued that this test is more strict that the OpenGL\n"
	"specification requires.  However, it is necessary to be this\n"
	"strict in order for the results to be useful to app developers\n"
	"using OpenGL for 2D drawing.\n"

	);




///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////

void
OrthoPosTinyQuads::runOne(OPResult& r, Window& w) {
	bool passed = true;

	GLUtils::useScreenCoords(windowSize, windowSize);

	glFrontFace(GL_CCW);

	glDisable(GL_LIGHTING);

	glDisable(GL_FOG);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_DITHER);
	glDisable(GL_COLOR_LOGIC_OP);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDisable(GL_POLYGON_STIPPLE);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glShadeModel(GL_FLAT);

	glClearColor(0, 0, 0, 0);

	////////////////////////////////////////////////////////////
	// Immediate-mode 1x1-pixel quads
	////////////////////////////////////////////////////////////

	// Clear both front and back buffers and swap, to avoid confusing
	// this test with results of the previous test:
	glClear(GL_COLOR_BUFFER_BIT);
	w.swap();
	glClear(GL_COLOR_BUFFER_BIT);
	{
		glBegin(GL_QUADS);
		for (int x = 1; x <= drawingSize; ++x)
			for (int y = 1; y <= drawingSize; ++y) {
				if ((x ^ y) & 1)
					glColor4f(0.0, 1.0, 0.0, 0.5);
				else
					glColor4f(1.0, 0.0, 0.0, 0.5);
				glVertex2i(x, y);
				glVertex2i(x + 1, y);
				glVertex2i(x + 1, y + 1);
				glVertex2i(x, y + 1);
			}
		glEnd();
	}
	verifyOrthPos(w, passed, name, r.config, r, env,
	       "Immediate-mode 1x1 quads");
	r.pass = passed;
} // OrthoPosTinyQuads::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
OrthoPosTinyQuads::logOne(OPResult& r) {
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	} else {
		env->log << '\n';
	}
	logStats(r);
} // OrthoPosTinyQuads::logOne

void
OrthoPosTinyQuads::logStats(OPResult& r) {
	logStats1("Immediate-mode 1x1 quads", r, env);
} // OrthoPosTinyQuads::logStats

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
OrthoPosTinyQuads::compareOne(OPResult& oldR, OPResult& newR) {
	bool same = true;

	doComparison(oldR, newR, newR.config, same, name,
		env, "immediate-mode 1x1 quads");

	if (same && env->options.verbosity) {
		env->log << name << ":  SAME "
			<< newR.config->conciseDescription()
			<< "\n";
	}

	if (env->options.verbosity) {
		env->log << env->options.db1Name << ':';
		logStats(oldR);
		env->log << env->options.db2Name << ':';
		logStats(newR);
	}
} // OrthoPosTinyQuads::compareOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
OrthoPosTinyQuads orthoPosTinyQuadsTest("orthoPosTinyQuads",
	"window, rgb > 1, z, fast",

	"This test checks the positioning of 1x1-pixel quadrilaterals\n"
	"under orthographic projection.	(This is important for apps\n"
	"that want to use OpenGL for precise 2D drawing.)  It fills in\n"
	"an entire rectangle with an array of quadrilaterals, drawing\n"
	"adjacent quads with different colors and with blending enabled.\n"
	"If there are gaps (pixels that are the background color, and\n"
	"thus haven't been filled), overlaps (pixels that show a blend\n"
	"of more than one color), or improper edges (pixels around the\n"
	"edge of the rectangle that haven't been filled, or pixels just\n"
	"outside the edge that have), then the test fails.\n"
	"\n"
	"This test generally fails for one of several reasons.  First,\n"
	"the coordinate transformation process may have an incorrect bias;\n"
	"this usually will cause a bad edge.  Second, the coordinate\n"
	"transformation process may round pixel coordinates incorrectly;\n"
	"this will usually cause gaps and/or overlaps.  Third, the\n"
	"quad rasterization process may not be filling the correct\n"
	"pixels; this can cause gaps, overlaps, or bad edges.\n"

	);




///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////

void
OrthoPosRandRects::runOne(OPResult& r, Window& w) {
	bool passed = true;

	GLUtils::useScreenCoords(windowSize, windowSize);

	glFrontFace(GL_CCW);

	glDisable(GL_LIGHTING);

	glDisable(GL_FOG);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_DITHER);
	glDisable(GL_COLOR_LOGIC_OP);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDisable(GL_POLYGON_STIPPLE);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glShadeModel(GL_FLAT);

	glClearColor(0, 0, 0, 0);

	////////////////////////////////////////////////////////////
	// Immediate-mode random axis-aligned rectangles
	////////////////////////////////////////////////////////////

	// Clear both front and back buffers and swap, to avoid confusing
	// this test with results of the previous test:
	glClear(GL_COLOR_BUFFER_BIT);
	w.swap();
	glClear(GL_COLOR_BUFFER_BIT);
	RandomDouble rand(1618);
	subdivideRects(1, drawingSize + 1, 1, drawingSize + 1,
		rand, true, true);
	verifyOrthPos(w, passed, name, r.config, r, env,
	       "Immediate-mode axis-aligned rectangles");
	r.pass = passed;
} // OrthoPosRandRects::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
OrthoPosRandRects::logOne(OPResult& r) {
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	} else {
		env->log << '\n';
	}
	logStats(r);
} // OrthoPosRandRects::logOne

void
OrthoPosRandRects::logStats(OPResult& r) {
	logStats1("Immediate-mode axis-aligned rectangles", r, env);
} // OrthoPosRandRects::logStats

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
OrthoPosRandRects::compareOne(OPResult& oldR, OPResult& newR) {
	bool same = true;

	doComparison(oldR, newR, newR.config, same, name,
		env, "immediate-mode axis-aligned rectangles");

	if (same && env->options.verbosity) {
		env->log << name << ":  SAME "
			<< newR.config->conciseDescription()
			<< "\n";
	}

	if (env->options.verbosity) {
		env->log << env->options.db1Name << ':';
		logStats(oldR);
		env->log << env->options.db2Name << ':';
		logStats(newR);
	}
} // OrthoPosRandRects::compareOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
OrthoPosRandRects orthoPosRandRectsTest("orthoPosRandRects",
	"window, rgb > 1, z, fast",

	"This test checks the positioning of axis-aligned rectangles\n"
	"under orthographic projection.	(This is important for apps\n"
	"that want to use OpenGL for precise 2D drawing.)  It fills in\n"
	"an entire rectangle with an array of smaller rects, drawing\n"
	"adjacent rects with different colors and with blending enabled.\n"
	"If there are gaps (pixels that are the background color, and\n"
	"thus haven't been filled), overlaps (pixels that show a blend\n"
	"of more than one color), or improper edges (pixels around the\n"
	"edge of the rectangle that haven't been filled, or pixels just\n"
	"outside the edge that have), then the test fails.\n"
	"\n"
	"This test generally fails for one of several reasons.  First,\n"
	"the coordinate transformation process may have an incorrect bias;\n"
	"this usually will cause a bad edge.  Second, the coordinate\n"
	"transformation process may round pixel coordinates incorrectly;\n"
	"this will usually cause gaps and/or overlaps.  Third, the\n"
	"rectangle rasterization process may not be filling the correct\n"
	"pixels; this can cause gaps, overlaps, or bad edges.\n"

	);




///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////

void
OrthoPosRandTris::runOne(OPResult& r, Window& w) {
	bool passed = true;

	GLUtils::useScreenCoords(windowSize, windowSize);

	glFrontFace(GL_CCW);

	glDisable(GL_LIGHTING);

	glDisable(GL_FOG);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_DITHER);
	glDisable(GL_COLOR_LOGIC_OP);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDisable(GL_POLYGON_STIPPLE);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glShadeModel(GL_FLAT);

	glClearColor(0, 0, 0, 0);

	////////////////////////////////////////////////////////////
	// Immediate-mode random axis-aligned rectangles
	////////////////////////////////////////////////////////////

	// Clear both front and back buffers and swap, to avoid confusing
	// this test with results of the previous test:
	glClear(GL_COLOR_BUFFER_BIT);
	w.swap();
	glClear(GL_COLOR_BUFFER_BIT);
	const int nPoints = 10;
	RandomDouble vRand(141421356);
	RandomMesh2D v(1, drawingSize + 1, nPoints,
		1, drawingSize + 1, nPoints,
		vRand);
	for (int i = nPoints - 1; i > 0; --i) {
		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0; j < nPoints; ++j) {
			glColor4f(1.0, 0.0, 0.0, 0.5);
			glVertex2fv(v(i, j));
			glColor4f(0.0, 1.0, 0.0, 0.5);
			glVertex2fv(v(i - 1, j));
		}
		glEnd();
	}
	verifyOrthPos(w, passed, name, r.config, r, env,
	       "Immediate-mode triangles");
	r.pass = passed;
} // OrthoPosRandTris::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
OrthoPosRandTris::logOne(OPResult& r) {
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	} else {
		env->log << '\n';
	}
	logStats(r);
} // OrthoPosRandTris::logOne

void
OrthoPosRandTris::logStats(OPResult& r) {
	logStats1("Immediate-mode triangles", r, env);
} // OrthoPosRandTris::logStats

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
OrthoPosRandTris::compareOne(OPResult& oldR, OPResult& newR) {
	bool same = true;

	doComparison(oldR, newR, newR.config, same, name,
		env, "immediate-mode triangles");

	if (same && env->options.verbosity) {
		env->log << name << ":  SAME "
			<< newR.config->conciseDescription()
			<< "\n";
	}

	if (env->options.verbosity) {
		env->log << env->options.db1Name << ':';
		logStats(oldR);
		env->log << env->options.db2Name << ':';
		logStats(newR);
	}
} // OrthoPosRandTris::compareOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
OrthoPosRandTris orthoPosRandTrisTest("orthoPosRandTris",
	"window, rgb > 1, z, fast",

	"This test checks the positioning of random triangles under\n"
	"orthographic projection.  (This is important for apps that\n"
	"want to use OpenGL for precise 2D drawing.)  It fills in an\n"
	"entire rectangle with an array of randomly-generated triangles,\n"
	"drawing adjacent triangles with different colors and with blending\n"
	"enabled.  If there are gaps (pixels that are the background color,\n"
	"and thus haven't been filled), overlaps (pixels that show a blend\n"
	"of more than one color), or improper edges (pixels around the\n"
	"edge of the rectangle that haven't been filled, or pixels just\n"
	"outside the edge that have), then the test fails.\n"
	"\n"
	"This test generally fails for one of several reasons.  First,\n"
	"the coordinate transformation process may have an incorrect bias;\n"
	"this usually will cause a bad edge.  Second, the coordinate\n"
	"transformation process may round pixel coordinates incorrectly;\n"
	"this will usually cause gaps and/or overlaps.  Third, the\n"
	"triangle rasterization process may not be filling the correct\n"
	"pixels; this can cause gaps, overlaps, or bad edges.\n"

	);


} // namespace GLEAN
