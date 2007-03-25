// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 2001  Allen Akin   All Rights Reserved.
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

// tpgos.cpp:  implementation of polygon offset tests
// Derived in part from tests written by Angus Dorbie <dorbie@sgi.com>
// in September 2000 and Rickard E. (Rik) Faith <faith@valinux.com> in
// October 2000.

#include "tpgos.h"
#include "image.h"
#include "misc.h"
#include <cmath>


namespace {

typedef struct {
	GLfloat angle;
	GLfloat axis[3];
} AngleAxis;

void
red() {
	glColor3f(1.0, 0.0, 0.0);
}

void
black() {
	glColor3f(0.0, 0.0, 0.0);
}

void
drawQuadAtDistance(GLdouble dist) {
	glBegin(GL_QUADS);
		glVertex3d(-dist, -dist, -dist);
		glVertex3d( dist, -dist, -dist);
		glVertex3d( dist,  dist, -dist);
		glVertex3d(-dist,  dist, -dist);
	glEnd();
}

GLdouble
windowCoordDepth(GLdouble dist) {
	// Assumes we're using the "far at infinity" projection matrix
	// and simple viewport transformation.
	return 0.5 * (dist - 2.0) / dist + 0.5;
}

bool
redQuadWasDrawn() {
	GLEAN::Image
		img(PGOS_WIN_SIZE, PGOS_WIN_SIZE, GL_RGB, GL_UNSIGNED_BYTE);
	img.read(0, 0);

	GLubyte* row = reinterpret_cast<GLubyte*>(img.pixels());
	for (GLsizei r = 0; r < img.height(); ++r) {
		GLubyte* pix = row;
		for (GLsizei c = 0; c < img.width(); ++c) {
			if (pix[0] == 0		// bad red component?
			 || pix[1] != 0		// bad green component?
			 || pix[2] != 0)	// bad blue component?
				return false;	// ...then quad wasn't drawn
			pix += 3;
		}
		row += img.rowSizeInBytes();
	}

	return true;
}

void
findIdealMRD(GLEAN::POResult& r, GLEAN::Window& w) {

	// MRD stands for Minimum Resolvable Difference, the smallest
	// distance in depth that suffices to separate any two
	// polygons (or a polygon and the near or far clipping
	// planes).
	//
	// This function tries to determine the "ideal" MRD for the
	// current rendering context.  It's expressed in window
	// coordinates, because the value in model or clipping
	// coordinates depends on the scale factors in the modelview
	// and projection matrices and on the distances to the near
	// and far clipping planes.
	//
	// For simple unsigned-integer depth buffers that aren't too
	// deep (so that precision isn't an issue during coordinate
	// transformations), it should be about one least-significant
	// bit.  For deep or floating-point or compressed depth
	// buffers the situation may be more complicated, so we don't
	// pass or fail an implementation solely on the basis of its
	// ideal MRD.
	//
	// There are two subtle parts of this function.  The first is
	// the projection matrix we use for rendering.  This matrix
	// places the far clip plane at infinity (so that we don't run
	// into arbitrary limits during our search process).  The
	// second is the method used for drawing the polygon.  We
	// scale the x and y coords of the polygon vertices by the
	// polygon's depth, so that it always occupies the full view
	// frustum.  This makes it easier to verify that the polygon
	// was resolved completely -- we just read back the entire
	// window and see if any background pixels appear.
	//
	// To insure that we get reasonable results on machines with
	// unusual depth buffers (floating-point, or compressed), we
	// determine the MRD twice, once close to the near clipping
	// plane and once as far away from the eye as possible.  On a
	// simple integer depth buffer these two values should be
	// essentially the same.  For other depth-buffer formats, the
	// ideal MRD is simply the largest of the two.

	GLdouble nearDist, farDist;
	int i;

	// First, find a distance that is as far away as possible, yet
	// a quad at that distance can be distinguished from the
	// background.  Start by pushing quads away from the eye until
	// we find an interval where the closer quad can be resolved,
	// but the farther quad cannot.  Then binary-search to find
	// the threshold.

	glDepthFunc(GL_LESS);
	glClearDepth(1.0);
	red();
	nearDist = 1.0;
	farDist = 2.0;
	for (;;) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawQuadAtDistance(farDist);
		w.swap();
		if (!redQuadWasDrawn())
			break;
		nearDist = farDist;
		farDist *= 2.0;
	}
	for (i = 0; i < 64; ++i) {
		GLdouble halfDist = 0.5 * (nearDist + farDist);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawQuadAtDistance(halfDist);
		w.swap();
		if (redQuadWasDrawn())
			nearDist = halfDist;
		else
			farDist = halfDist;
	}
	r.nextToFar = nearDist;

	// We can derive a resolvable difference from the value
	// nextToFar, but it's not necessarily the one we want. 
	// Consider mapping the object coordinate range [0,1] onto the
	// integer window coordinate range [0,2].  A natural way to do
	// this is with a linear function, windowCoord =
	// 2*objectCoord.  With rounding, this maps [0,0.25) to 0,
	// [0.25,0.75) to 1, and [0.75,1] to 2.  Note that the
	// intervals at either end are 0.25 wide, but the one in the
	// middle is 0.5 wide.  The difference we can derive from
	// nextToFar is related to the width of the final interval. 
	// We want to back up just a bit so that we can get a
	// (possibly much larger) difference that will work for the
	// larger interval.  To do this we need to find a difference
	// that allows us to distinguish two quads when the more
	// distant one is at distance nextToFar.

	nearDist = 1.0;
	farDist = r.nextToFar;
	for (i = 0; i < 64; ++i) {
		GLdouble halfDist = 0.5 * (nearDist + farDist);

		black();
		glDepthFunc(GL_ALWAYS);
		drawQuadAtDistance(r.nextToFar);

		red();
		glDepthFunc(GL_LESS);
		drawQuadAtDistance(halfDist);

		w.swap();
		if (redQuadWasDrawn())
			nearDist = halfDist;
		else
			farDist = halfDist;
	}

	r.idealMRDFar = windowCoordDepth(r.nextToFar)
		- windowCoordDepth(nearDist);


	// Now we apply a similar strategy at the near end of the
	// depth range, but swapping the senses of various comparisons
	// so that we approach the near clipping plane rather than the
	// far.

	glClearDepth(0.0);
	glDepthFunc(GL_GREATER);
	red();
	nearDist = 1.0;
	farDist = r.nextToFar;
	for (i = 0; i < 64; ++i) {
		GLdouble halfDist = 0.5 * (nearDist + farDist);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawQuadAtDistance(halfDist);
		w.swap();
		if (redQuadWasDrawn())
			farDist = halfDist;
		else
			nearDist = halfDist;
	}
	r.nextToNear = farDist;

	nearDist = r.nextToNear;
	farDist = r.nextToFar;
	for (i = 0; i < 64; ++i) {
		GLdouble halfDist = 0.5 * (nearDist + farDist);

		black();
		glDepthFunc(GL_ALWAYS);
		drawQuadAtDistance(r.nextToNear);

		red();
		glDepthFunc(GL_GREATER);
		drawQuadAtDistance(halfDist);

		w.swap();
		if (redQuadWasDrawn())
			farDist = halfDist;
		else
			nearDist = halfDist;
	}

	r.idealMRDNear = windowCoordDepth(farDist)
		- windowCoordDepth(r.nextToNear);
} // findIdealMRD

double
readDepth(int x, int y) {
	GLuint depth;
	glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, &depth);

	// This normalization of "depth" is correct even on 64-bit
	// machines because GL types have machine-independent ranges.
	return static_cast<double>(depth) / 4294967295.0;
}

double
readDepth(double x, double y) {
	return readDepth(static_cast<int>(x), static_cast<int>(y));
}

void
findActualMRD(GLEAN::POResult& r, GLEAN::Window& w) {

	// Here we use polygon offset to determine the
	// implementation's actual MRD.

	double baseDepth;

	glDepthFunc(GL_ALWAYS);

	// Draw a quad far away from the eye and read the depth at its center:
	glDisable(GL_POLYGON_OFFSET_FILL);
	drawQuadAtDistance(r.nextToFar);
	baseDepth = readDepth(PGOS_WIN_SIZE/2, PGOS_WIN_SIZE/2);

	// Now draw a quad that's one MRD closer to the eye:
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.0, -1.0);
	drawQuadAtDistance(r.nextToFar);

	// The difference between the depths of the two quads is the
	// value the implementation is actually using for one MRD:
	r.actualMRDFar = baseDepth
		- readDepth(PGOS_WIN_SIZE/2, PGOS_WIN_SIZE/2);

	// Repeat the process for a quad close to the eye:
	glDisable(GL_POLYGON_OFFSET_FILL);
	drawQuadAtDistance(r.nextToNear);
	baseDepth = readDepth(PGOS_WIN_SIZE/2, PGOS_WIN_SIZE/2);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.0, 1.0);	// 1 MRD further away
	drawQuadAtDistance(r.nextToNear);
	r.actualMRDNear = readDepth(PGOS_WIN_SIZE/2, PGOS_WIN_SIZE/2)
		- baseDepth;
	w.swap();
} // findActualMRD

void
logMRD(ostream& log, double mrd, GLEAN::DrawingSurfaceConfig* config) {
	int bits = static_cast<int>(0.5 + (pow(2.0, config->z) - 1.0) * mrd);
	log << mrd << " (nominally " << bits
		<< ((bits == 1)? " bit)": " bits)");
} // logMRD

void
draw2x2Quad() {
	glBegin(GL_QUADS);
		glVertex2f(-1.0, -1.0);
		glVertex2f( 1.0, -1.0);
		glVertex2f( 1.0,  1.0);
		glVertex2f(-1.0,  1.0);
	glEnd();
}

void
checkSlopeOffset(GLEAN::POResult& r, GLEAN::Window& w, AngleAxis* aa) {
	// This function checks for correct slope-based offsets for
	// a quad rotated to a given angle around a given axis.
	//
	// The basic strategy is to:
	//	Draw the quad.  (Note: the quad's size and position
	//		are chosen so that it won't ever be clipped.)
	//	Sample three points in the quad's interior.
	//	Compute dz/dx and dz/dy based on those samples.
	//	Compute the range of allowable offsets; must be between
	//		max(abs(dz/dx), abs(dz/dy)) and
	//		sqrt((dz/dx)**2, (dz/dy)**2)
	//	Sample the depth of the quad at its center.
	//	Use PolygonOffset to produce an offset equal to one
	//		times the depth slope of the base quad.
	//	Draw another quad with the same orientation as the first.
	//	Sample the second quad at its center.
	//	Compute the difference in depths between the first quad
	//		and the second.
	//	Verify that the difference is within the allowable range.
	//	Repeat for a third quad at twice the offset from the first.
	//		(This verifies that the implementation is scaling
	//		the depth offset correctly.)

	if (!r.slopeOffsetsPassed)
		return;

	const GLfloat quadDist = 2.5;	// must be > 1+sqrt(2) to avoid
					// clipping by the near plane

	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	red();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -quadDist);
	glRotatef(aa->angle, aa->axis[0], aa->axis[1], aa->axis[2]);

	GLdouble modelViewMat[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelViewMat);
	GLdouble projectionMat[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projectionMat);
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glDisable(GL_POLYGON_OFFSET_FILL);

	draw2x2Quad();
	w.swap();

	GLdouble centerW[3];
	gluProject(0.0, 0.0, 0.0, modelViewMat, projectionMat, viewport,
		centerW + 0, centerW + 1, centerW + 2);
	GLdouble baseDepth = readDepth(centerW[0], centerW[1]);

	GLdouble p0[3];
	gluProject(-0.9, -0.9, 0.0, modelViewMat, projectionMat, viewport,
		p0 + 0, p0 + 1, p0 + 2);
	p0[2] = readDepth(p0[0], p0[1]);
	GLdouble p1[3];
	gluProject( 0.9, -0.9, 0.0, modelViewMat, projectionMat, viewport,
		p1 + 0, p1 + 1, p1 + 2);
	p1[2] = readDepth(p1[0], p1[1]);
	GLdouble p2[3];
	gluProject( 0.9,  0.9, 0.0, modelViewMat, projectionMat, viewport,
		p2 + 0, p2 + 1, p2 + 2);
	p2[2] = readDepth(p2[0], p2[1]);

	double det = (p0[0] - p1[0]) * (p0[1] - p2[1])
		- (p0[0] - p2[0]) * (p0[1] - p1[1]);
	if (fabs(det) < 0.001)
		return;		// too close to colinear to evaluate

	double dzdx = ((p0[2] - p1[2]) * (p0[1] - p2[1])
		- (p0[2] - p2[2]) * (p0[1] - p1[1])) / det;
	double dzdy = ((p0[0] - p1[0]) * (p0[2] - p2[2])
		- (p0[0] - p2[0]) * (p0[2] - p1[2])) / det;

	double mMax = 1.1 * sqrt(dzdx * dzdx + dzdy * dzdy) + r.idealMRDNear;
		// (adding idealMRDNear is a fudge for roundoff error when
		// the slope is extremely close to zero)
	double mMin = 0.9 * max(fabs(dzdx), fabs(dzdy));

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-1.0, 0.0);
	draw2x2Quad();
	GLdouble offsetDepth = readDepth(centerW[0], centerW[1]);
	GLdouble offset = max(baseDepth - offsetDepth, 0.0);
	if (offset < mMin || offset > mMax) {
		r.slopeOffsetsPassed = false;
		r.failingAngle = aa->angle;
		r.failingAxis[0] = aa->axis[0];
		r.failingAxis[1] = aa->axis[1];
		r.failingAxis[2] = aa->axis[2];
		r.failingOffset = offset;
		r.minGoodOffset = mMin;
		r.maxGoodOffset = mMax;
		return;
	}

	glPolygonOffset(-2.0, 0.0);
	draw2x2Quad();
	offsetDepth = readDepth(centerW[0], centerW[1]);
	offset = max(baseDepth - offsetDepth, 0.0);
	if (offset < 2.0 * mMin || offset > 2.0 * mMax) {
		r.slopeOffsetsPassed = false;
		r.failingAngle = aa->angle;
		r.failingAxis[0] = aa->axis[0];
		r.failingAxis[1] = aa->axis[1];
		r.failingAxis[2] = aa->axis[2];
		r.failingOffset = offset;
		r.minGoodOffset = 2.0 * mMin;
		r.maxGoodOffset = 2.0 * mMax;
		return;
	}
}

void
checkSlopeOffsets(GLEAN::POResult& r, GLEAN::Window& w) {
	// This function checks that the implementation is offsetting
	// primitives correctly according to their depth slopes.
	// (Note that it uses some values computed by findIdealMRD, so
	// that function must be run first.)

	// Rotation angles (degrees) and axes for which offset will be checked:
	AngleAxis aa[] = {
		{ 0,	{1, 0, 0}},
		{30,	{1, 0, 0}},
		{45,	{1, 0, 0}},
		{60,	{1, 0, 0}},
		{80,	{1, 0, 0}},
		{ 0,	{0, 1, 0}},
		{30,	{0, 1, 0}},
		{45,	{0, 1, 0}},
		{60,	{0, 1, 0}},
		{80,	{0, 1, 0}},
		{ 0,	{1, 1, 0}},
		{30,	{1, 1, 0}},
		{45,	{1, 1, 0}},
		{60,	{1, 1, 0}},
		{80,	{1, 1, 0}},
		{ 0,	{2, 1, 0}},
		{30,	{2, 1, 0}},
		{45,	{2, 1, 0}},
		{60,	{2, 1, 0}},
		{80,	{2, 1, 0}}
	};

	r.slopeOffsetsPassed = true;
	for (unsigned int i = 0;
	    r.slopeOffsetsPassed && i < sizeof(aa)/sizeof(aa[0]);
	    ++i)
		checkSlopeOffset(r, w, aa + i);
} // checkSlopeOffsets


} // anonymous namespace

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////
void
PgosTest::runOne(POResult& r, GLEAN::Window& w) {

	glViewport(0, 0, PGOS_WIN_SIZE, PGOS_WIN_SIZE);
	glDepthRange(0.0, 1.0);
	{
	glMatrixMode(GL_PROJECTION);

	// The following projection matrix places the near clipping
	// plane at distance 1.0, and the far clipping plane at
	// infinity.  This allows us to stress depth-buffer resolution
	// as far away from the eye as possible, without introducing
	// code that depends on the size or format of the depth
	// buffer.
	//
	// (To derive this matrix, start with the matrix generated by
	// glFrustum with near-plane distance equal to 1.0, and take
	// the limit of the matrix elements as the far-plane distance
	// goes to infinity.)

	static GLfloat near1_farInfinity[] = {
		1.0,  0.0,  0.0,  0.0,
		0.0,  1.0,  0.0,  0.0,
		0.0,  0.0, -1.0, -1.0,
		0.0,  0.0, -2.0,  0.0
	};
	glLoadMatrixf(near1_farInfinity);
	}

	glDisable(GL_LIGHTING);

	glFrontFace(GL_CCW);
	glDisable(GL_NORMALIZE);
	glDisable(GL_COLOR_MATERIAL);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);

	glDisable(GL_FOG);

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DITHER);
	glDisable(GL_COLOR_LOGIC_OP);
	glReadBuffer(GL_FRONT);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDisable(GL_POLYGON_STIPPLE);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glShadeModel(GL_FLAT);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	
        // Clear both front and back buffers and swap, to avoid confusing
        // this test with results of the previous test:
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        w.swap();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	findIdealMRD(r, w);
	findActualMRD(r, w);
	double idealMRD = max(r.idealMRDNear, r.idealMRDFar);
	double actualMRD = max(r.actualMRDNear, r.actualMRDFar);
	r.bigEnoughMRD = (actualMRD >= 0.99 * idealMRD);
	r.smallEnoughMRD = (actualMRD <= 2.0 * idealMRD);

	checkSlopeOffsets(r, w);

	r.pass = r.bigEnoughMRD && r.smallEnoughMRD && r.slopeOffsetsPassed;
} // PgosTest::runOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
PgosTest::compareOne(POResult& oldR, POResult& newR) {
	comparePassFail(oldR, newR);
	if (oldR.bigEnoughMRD != newR.bigEnoughMRD) {
		env->log << "\tMin size MRD criterion: "
			<< env->options.db1Name
			<< (oldR.bigEnoughMRD? " PASS, ": " FAIL, ")
			<< env->options.db2Name
			<< (newR.bigEnoughMRD? " PASS\n": " FAIL\n");
	}
	if (oldR.smallEnoughMRD != newR.smallEnoughMRD) {
		env->log << "\tMax size MRD criterion: "
			<< env->options.db1Name
			<< (oldR.smallEnoughMRD? " PASS, ": " FAIL, ")
			<< env->options.db2Name
			<< (newR.smallEnoughMRD? " PASS\n": " FAIL\n");
	}
	if (oldR.slopeOffsetsPassed != newR.slopeOffsetsPassed) {
		env->log << "\tSlope-relative offsets criterion: "
			<< env->options.db1Name
			<< (oldR.slopeOffsetsPassed? " PASS, ": " FAIL, ")
			<< env->options.db2Name
			<< (newR.slopeOffsetsPassed? " PASS\n": " FAIL\n");
	}
	if (!oldR.slopeOffsetsPassed && !newR.slopeOffsetsPassed) {
		if (oldR.failingAngle != newR.failingAngle) {
			env->log << '\t'
				<< env->options.db1Name
				<< " failed at angle "
				<< oldR.failingAngle
				<< ", "
				<< env->options.db2Name
				<< " failed at angle "
				<< newR.failingAngle
				<< '\n';
		}
		if (oldR.failingAxis[0] != newR.failingAxis[0]
		 || oldR.failingAxis[1] != newR.failingAxis[1]
		 || oldR.failingAxis[2] != newR.failingAxis[2]) {
			env->log << '\t'
				<< env->options.db1Name
				<< " failed at axis ("
				<< oldR.failingAxis[0]
				<< ", "
				<< oldR.failingAxis[1]
				<< ", "
				<< oldR.failingAxis[2]
				<< "), "
				<< env->options.db2Name
				<< " failed at axis ("
				<< newR.failingAxis[0]
				<< ", "
				<< newR.failingAxis[1]
				<< ", "
				<< newR.failingAxis[2]
				<< ")\n";
		}
	}
} // PgosTest::compareOne

void
PgosTest::logOne(POResult& r) {
	logPassFail(r);
	logConcise(r);

	if (!r.bigEnoughMRD)
		env->log << "\tActual MRD is too small "
			"(may cause incorrect results)\n";
	if (!r.smallEnoughMRD)
		env->log << "\tActual MRD is too large "
			"(may waste depth-buffer range)\n";
	if (!r.slopeOffsetsPassed) {
		env->log << "\tDepth-slope related offset was too "
			<< ((r.failingOffset < r.minGoodOffset)?
				"small": "large")
			<< "; first failure at:\n";
		env->log << "\t\tAngle = " << r.failingAngle << " degrees, "
			<< "axis = (" << r.failingAxis[0] << ", "
				<< r.failingAxis[1] << ", "
				<< r.failingAxis[2] << ")\n";
		env->log << "\t\tFailing offset was "
			<< setprecision(16) << r.failingOffset << "\n";
		env->log << "\t\tAllowable range is ("
			<< r.minGoodOffset << ", " << r.maxGoodOffset << ")\n";
	}

	if (!r.pass)
		env->log << '\n';

	env->log << "\tIdeal  MRD at near plane is ";
	logMRD(env->log, r.idealMRDNear, r.config);
	env->log << '\n';

	env->log << "\tActual MRD at near plane is ";
	logMRD(env->log, r.actualMRDNear, r.config);
	env->log << '\n';

	env->log << "\tIdeal  MRD at infinity is ";
	logMRD(env->log, r.idealMRDFar, r.config);
	env->log << '\n';

	env->log << "\tActual MRD at infinity is ";
	logMRD(env->log, r.actualMRDFar, r.config);
	env->log << '\n';

} // PgosTest::logOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
PgosTest
pgosTest("polygonOffset", "window, rgb, z",

	"This test verifies glPolygonOffset.  It is run on every\n"
	"OpenGL-capable drawing surface configuration that supports\n"
	"creation of a window, has a depth buffer, and is RGB.\n"
	"\n"
	"The first subtest verifies that the OpenGL implementation is\n"
	"using a plausible value for the \"minimum resolvable\n"
	"difference\" (MRD).  This is the offset in window coordinates\n"
	"that is sufficient to provide separation in depth (Z) for any\n"
	"two parallel surfaces.  The subtest searches for the MRD by\n"
	"drawing two surfaces at a distance from each other and\n"
	"checking the resulting image to see if they were cleanly\n"
	"separated.  The distance is then modified (using a binary\n"
	"search) until a minimum value is found.  This is the so-called\n"
	"\"ideal\" MRD.  Then two surfaces are drawn using\n"
	"glPolygonOffset to produce a separation that should equal one\n"
	"MRD.  The depth values at corresponding points on each surface\n"
	"are subtracted to form the \"actual\" MRD.  The subtest performs\n"
	"these checks twice, once close to the viewpoint and once far\n"
	"away from it, and passes if the largest of the ideal MRDs and\n"
	"the largest of the actual MRDs are nearly the same.\n"
	"\n"
	"The second subtest verifies that the OpenGL implementation is\n"
	"producing plausible values for slope-dependent offsets.  The\n"
	"OpenGL spec requires that the depth slope of a surface be\n"
	"computed by an approximation that is at least as large as\n"
	"max(abs(dz/dx),abs(dz/dy)) and no larger than\n"
	"sqrt((dz/dx)**2+(dz/dy)**2).  The subtest draws a quad rotated\n"
	"by various angles along various axes, samples three points on\n"
	"the quad's surface, and computes dz/dx and dz/dy.  Then it\n"
	"draws two additional quads offset by one and two times the\n"
	"depth slope, respectively.  The base quad and the two new\n"
	"quads are sampled and their actual depths read from the depth\n"
	"buffer.  The subtest passes if the quads are offset by amounts\n"
	"that are within one and two times the allowable range,\n"
	"respectively.\n"

	);

} // namespace GLEAN
