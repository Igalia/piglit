// BEGIN_COPYRIGHT -*- glean -*-

/*
 * Copyright © 2006 Intel Corporation
 * Copyright © 1999 Allen Akin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/* ttexrect.cpp:  Test the ARB_texture_rectangle extension
 * Author: Eric Anholt <eric@anholt.net>
 *
 * Test procedure:
 *   Create a 255x127 texture of varying colors and bind it as a
 *   GL_ARB_texture_recangle target.  Draw that rectangle to the window, and
 *   check that the texture was drawn correctly.  The common failure to be
 *   caught with this test is not adjusting the non-normalized coordinates on
 *   hardware that expects normalized coordinates.
 */

#include "ttexrect.h"
#include <cassert>
#include <stdio.h>
#include <cmath>

namespace GLEAN {

/**
 * Test if two colors are close enough to be considered the same.
 */
bool
TexRectTest::TestColor(const GLfloat c1[3], const GLfloat c2[3])
{
	if (fabs(c1[0] - c2[0]) <= mTolerance[0] &&
	    fabs(c1[1] - c2[1]) <= mTolerance[1] &&
	    fabs(c1[2] - c2[2]) <= mTolerance[2])
		return true;
	else
		return false;
}

void
TexRectTest::CalculateTolerance()
{
	GLint rBits, gBits, bBits;
	GLint rTexBits, gTexBits, bTexBits;

	// Get fb resolution
	glGetIntegerv(GL_RED_BITS, &rBits);
	glGetIntegerv(GL_GREEN_BITS, &gBits);
	glGetIntegerv(GL_BLUE_BITS, &bBits);

	// Get tex resolution
	glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE_ARB,
				 0, GL_TEXTURE_RED_SIZE, &rTexBits);
	glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE_ARB,
				 0, GL_TEXTURE_GREEN_SIZE, &gTexBits);
	glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE_ARB,
				 0, GL_TEXTURE_BLUE_SIZE, &bTexBits);

	// Find smaller of frame buffer and texture bits
	rBits = (rBits < rTexBits) ? rBits : rTexBits;
	gBits = (gBits < gTexBits) ? gBits : gTexBits;
	bBits = (bBits < bTexBits) ? bBits : bTexBits;

	// If these fail, something's seriously wrong.
	assert(rBits > 0);
	assert(gBits > 0);
	assert(bBits > 0);
	mTolerance[0] = 3.0 / (1 << rBits);
	mTolerance[1] = 3.0 / (1 << gBits);
	mTolerance[2] = 3.0 / (1 << bBits);
}

#define TEXTURE_WIDTH	255
#define TEXTURE_HEIGHT	127

/**
 * Creates a TEXTURE_WIDTH * TEXTURE_HEIGHT rectangular texture and draws it to
 * the window.  It then reads the output back to verify that the texture stayed
 * intact.
 */
void
TexRectTest::runOne(BasicResult& r, Window& w)
{
	float image[TEXTURE_WIDTH * TEXTURE_HEIGHT * 3];
	float actual[TEXTURE_WIDTH * TEXTURE_HEIGHT * 3];
	(void) w;

	/* Set up a texture that is color ramps with red to black top to
	 * bottom and green to black left to right.
	 */
	for (int y = 0; y < TEXTURE_HEIGHT; y++) {
		for (int x = 0; x < TEXTURE_WIDTH; x++) {
			int i = (y * TEXTURE_WIDTH + x) * 3;

			image[i + 0] = (float)x / (TEXTURE_WIDTH - 1);
			image[i + 1] = 1.0 - ((float)  y / (TEXTURE_HEIGHT - 1));
			image[i + 2] = 0.0;
		}
	}

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glShadeModel(GL_FLAT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 256, 0, 256, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, windowSize, windowSize);

	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, 255, 127, 0,
		     GL_RGB, GL_FLOAT, image);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);

	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_BACK);

	r.pass = true;

	/* Draw our texture to the window such that each texel should map
	 * to the corresponding pixel of the window.
	 */
	glBegin(GL_POLYGON);
	glTexCoord2f(0, 0);
	glVertex2f(0, 0);

	glTexCoord2f(TEXTURE_WIDTH, 0);
	glVertex2f(TEXTURE_WIDTH, 0);

	glTexCoord2f(TEXTURE_WIDTH, TEXTURE_HEIGHT);
	glVertex2f(TEXTURE_WIDTH, TEXTURE_HEIGHT);

	glTexCoord2f(0, TEXTURE_HEIGHT);
	glVertex2f(0, TEXTURE_HEIGHT);
	glEnd();

	/* Read back the output */
	glReadPixels(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT,
		     GL_RGB, GL_FLOAT, actual);

	w.swap(); // lets us watch the progress

	CalculateTolerance();

	/* Verify the output */
	for (int y = 0; y < TEXTURE_HEIGHT; y++) {
		for (int x = 0; x < TEXTURE_WIDTH; x++) {
			int i = (y * TEXTURE_WIDTH + x) * 3;

			if (!TestColor(&image[i], &actual[i])) {
				// Report the error
				env->log << name
					 << ":  FAIL at (" << x << "," << y
					 << "):\n"
					 << " Expected=("
					 << image[i + 0] << ", "
					 << image[i + 1] << ", "
					 << image[i + 2] << ")\n"
					 << " Measured=("
					 << actual[i + 0] << ", "
					 << actual[i + 1] << ", "
					 << actual[i + 2] << ")\n";
				r.pass = false;
			}
		}
	}
} // TexRectTest::runOne


void
TexRectTest::logOne(BasicResult& r) {
	logPassFail(r);
	logConcise(r);
} // TexRectTest::logOne


///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
TexRectTest texRectTest("texRect", "window, rgb",
			"GL_ARB_texture_rectangle",
			"Test basic texture rectangle functionality.\n");


} // namespace GLEAN
