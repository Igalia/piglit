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


// tpaths.h:  Basic test of GL rendering paths.
// This test verifies that basic, trival OpenGL paths work as expected.
// For example, glAlphaFunc(GL_GEQUAL, 0.0) should always pass and
// glAlphaFunc(GL_LESS, 0.0) should always fail.  We setup trivial
// pass and fail conditions for each of alpha test, blending, color mask,
// depth test, logic ops, scissor, stencil, stipple, and texture and
// make sure they work as expected.  We also setup trival-pass for all
// these paths simultaneously and test that as well.
//
// To test for pass/fail we examine the color buffer for white or black,
// respectively.
//
// Author: Brian Paul (brianp@valinux.com)  November 2000

#include <stdlib.h>
#include "tpaths.h"

namespace GLEAN {

void
PathsTest::SetPathState(Path path, State state) const {

	switch (path) {
	case ALPHA:
		if (state == ALWAYS_PASS) {
			glAlphaFunc(GL_GEQUAL, 0.0);
			glEnable(GL_ALPHA_TEST);
		}
		else if (state == ALWAYS_FAIL) {
			glAlphaFunc(GL_GREATER, 1.0);
			glEnable(GL_ALPHA_TEST);
		}
		else {
			glDisable(GL_ALPHA_TEST);
		}
		break;
	case BLEND:
		if (state == ALWAYS_PASS) {
			glBlendFunc(GL_ONE, GL_ZERO);
			glEnable(GL_BLEND);
		}
		else if (state == ALWAYS_FAIL) {
			glBlendFunc(GL_ZERO, GL_ONE);
			glEnable(GL_BLEND);
		}
		else {
			glDisable(GL_BLEND);
		}
		break;
	case COLOR_MASK:
		if (state == ALWAYS_PASS) {
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}
		else if (state == ALWAYS_FAIL) {
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		}
		else {
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}
		break;
	case DEPTH:
		if (state == ALWAYS_PASS) {
			glDepthFunc(GL_ALWAYS);
			glEnable(GL_DEPTH_TEST);
		}
		else if (state == ALWAYS_FAIL) {
			glDepthFunc(GL_NEVER);
			glEnable(GL_DEPTH_TEST);
		}
		else {
			glDisable(GL_DEPTH_TEST);
		}
		break;
	case LOGIC:
		if (state == ALWAYS_PASS) {
			glLogicOp(GL_OR);
			glEnable(GL_COLOR_LOGIC_OP);
		}
		else if (state == ALWAYS_FAIL) {
			glLogicOp(GL_AND);
			glEnable(GL_COLOR_LOGIC_OP);
		}
		else {
			glDisable(GL_COLOR_LOGIC_OP);
		}
		break;
	case SCISSOR:
		if (state == ALWAYS_PASS) {
			glScissor(0, 0, 10, 10);
			glEnable(GL_SCISSOR_TEST);
		}
		else if (state == ALWAYS_FAIL) {
			glScissor(0, 0, 0, 0);
			glEnable(GL_SCISSOR_TEST);
		}
		else {
			glDisable(GL_SCISSOR_TEST);
		}
		break;
	case STENCIL:
		if (state == ALWAYS_PASS) {
			// pass if reference <= stencil value (ref = 0)
			glStencilFunc(GL_LEQUAL, 0, ~0);
			glEnable(GL_STENCIL_TEST);
		}
		else if (state == ALWAYS_FAIL) {
			// pass if reference > stencil value (ref = 0)
			glStencilFunc(GL_GREATER, 0, ~0);
			glEnable(GL_STENCIL_TEST);
		}
		else {
			glDisable(GL_STENCIL_TEST);
		}
		break;
	case STIPPLE:
		if (state == ALWAYS_PASS) {
			GLubyte stipple[4*32];
			for (int i = 0; i < 4*32; i++)
				stipple[i] = 0xff;
			glPolygonStipple(stipple);
			glEnable(GL_POLYGON_STIPPLE);
		}
		else if (state == ALWAYS_FAIL) {
			GLubyte stipple[4*32];
			for (int i = 0; i < 4*32; i++)
				stipple[i] = 0x0;
			glPolygonStipple(stipple);
			glEnable(GL_POLYGON_STIPPLE);
		}
		else {
			glDisable(GL_POLYGON_STIPPLE);
		}
		break;
	case TEXTURE:
		if (state == DISABLE) {
			glDisable(GL_TEXTURE_2D);
		}
		else {
			GLubyte val = (state == ALWAYS_PASS) ? 0xff : 0x00;
			int i;
			GLubyte texImage[4*4*4];
			for (i = 0; i < 4*4*4; i++)
				texImage[i] = val;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0,
				     GL_RGBA, GL_UNSIGNED_BYTE, texImage);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
					GL_NEAREST);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
				  GL_MODULATE);
			glEnable(GL_TEXTURE_2D);
		}
		break;
	default:
		abort(); // error
	}
}


const char *
PathsTest::PathName(Path path) const {

	switch (path) {
	case ALPHA:
		return "Alpha Test";
	case BLEND:
		return "Blending";
	case COLOR_MASK:
		return "Color Mask";
	case DEPTH:
		return "Depth Test";
	case LOGIC:
		return "LogicOp";
	case SCISSOR:
		return "Scissor Test";
	case STENCIL:
		return "Stencil Test";
	case STIPPLE:
		return "Polygon Stipple";
	case TEXTURE:
		return "Modulated Texture";
	case ZZZ:
		return "paths";
	default:
		return "???";
	}
}


void
PathsTest::FailMessage(BasicResult &r, Path path, State state, GLfloat pixel[3])
		       const {
	env->log << name << ":  FAIL "
		 << r.config->conciseDescription() << '\n';
	if (state == ALWAYS_PASS) {
		env->log << "\t" << PathName(path)
			 << " should have had no effect (1, 1, 1)"
			 << " but actually modified the fragment: "
			 << "(" << pixel[0] << "," << pixel[1] << ","
			 << pixel[2] << ")\n";
	}
	else {
		env->log << "\t" << PathName(path)
			 << " should have culled the fragment (0, 0, 0)"
			 << " but actually didn't: (" << pixel[0] << ","
			 << pixel[1] << "," << pixel[2] << ")\n";
	}
}


///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////
void
PathsTest::runOne(BasicResult& r, Window&) {

	Path p, paths[1000];
	int i, numPaths = 0;

	// draw 10x10 pixel quads
	glViewport(0, 0, 10, 10);

	glDisable(GL_DITHER);

	// Build the list of paths to exercise.  Skip depth test if we have no
	// depth buffer.  Skip stencil test if we have no stencil buffer.
	for (p = ALPHA; p != ZZZ; p = (Path) (p + 1)) {
		if (!((p == DEPTH && r.config->z == 0) ||
		      (p == STENCIL && r.config->s == 0))) {
			paths[numPaths++] = p;
		}
	}

	// test always-pass paths
	for (i = 0; i < numPaths; i++) {
		glClear(GL_COLOR_BUFFER_BIT);

		SetPathState(paths[i], ALWAYS_PASS);

		// draw polygon
		glColor4f(1, 1, 1, 1);
		glBegin(GL_POLYGON);
		glVertex2f(-1, -1);
		glVertex2f( 1, -1);
		glVertex2f( 1,  1);
		glVertex2f(-1,  1);
		glEnd();

		SetPathState(paths[i], DISABLE);

		// test buffer
		GLfloat pixel[3];
		glReadPixels(4, 4, 1, 1, GL_RGB, GL_FLOAT, pixel);
		if (pixel[0] != 1.0 || pixel[1] != 1.0 || pixel[2] != 1.0) {
			FailMessage(r, paths[i], ALWAYS_PASS, pixel);
			r.pass = false;
			return;
		}
	}

	// enable all always-pass paths
	{
		glClear(GL_COLOR_BUFFER_BIT);

		for (i = 0; i < numPaths; i++) {
			SetPathState(paths[i], ALWAYS_PASS);
		}

		// draw polygon
		glColor4f(1, 1, 1, 1);
		glBegin(GL_POLYGON);
		glVertex2f(-1, -1);
		glVertex2f( 1, -1);
		glVertex2f( 1,  1);
		glVertex2f(-1,  1);
		glEnd();

		for (i = 0; i < numPaths; i++) {
			SetPathState(paths[i], DISABLE);
		}

		// test buffer
		GLfloat pixel[3];
		glReadPixels(4, 4, 1, 1, GL_RGB, GL_FLOAT, pixel);
		if (pixel[0] != 1.0 || pixel[1] != 1.0 || pixel[2] != 1.0) {
			FailMessage(r, paths[i], ALWAYS_PASS, pixel);
			r.pass = false;
			return;
		}
	}

	// test never-pass paths
	for (i = 0; i < numPaths; i++) {
		glClear(GL_COLOR_BUFFER_BIT);

		SetPathState(paths[i], ALWAYS_FAIL);

		// draw polygon
		glColor4f(1, 1, 1, 1);
		glBegin(GL_POLYGON);
		glVertex2f(-1, -1);
		glVertex2f( 1, -1);
		glVertex2f( 1,  1);
		glVertex2f(-1,  1);
		glEnd();

		SetPathState(paths[i], DISABLE);

		// test buffer
		GLfloat pixel[3];
		glReadPixels(4, 4, 1, 1, GL_RGB, GL_FLOAT, pixel);
		if (pixel[0] != 0.0 || pixel[1] != 0.0 || pixel[2] != 0.0) {
			FailMessage(r, paths[i], ALWAYS_FAIL, pixel);
			r.pass = false;
			return;
		}
	}

	// success
	r.pass = true;
} // PathsTest::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
PathsTest::logOne(BasicResult& r) {
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	}
} // PathsTest::logOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
PathsTest pathsTest("paths", "window, rgb",

	"This test verifies that basic OpenGL operations such as the alpha\n"
	"test, depth test, blending, stippling, and texturing work for\n"
	"trivial cases.\n");


} // namespace GLEAN
