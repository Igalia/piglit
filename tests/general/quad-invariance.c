/*
 * Copyright © 2010 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/**
 * @file quad-invariance.c
 *
 * Test whether quad rasterization changes when drawing one or more
 * than one quad.
 *
 * This is not strictly required by conformance, but seems to be in
 * the spirit of the invariance rules.  As a result, failure of this
 * test is only a warning.
 */

#include "piglit-util.h"

int piglit_width = 128, piglit_height = 128;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float verts[12][2] = {
		/* prim 1: left half of screen. */
		{-1.0, -1.0},
		{ 0.0, -1.0},
		{ 0.0,  1.0},
		{-1.0,  1.0},
		/* prim 2: right half of screen. */
		{ 0.0, -1.0},
		{ 1.0, -1.0},
		{ 1.0,  1.0},
		{ 0.0,  1.0},
		/* prim 3: somewhere off the screen. */
		{ 2.0, -1.0},
		{ 3.0, -1.0},
		{ 3.0,  1.0},
		{ 2.0,  1.0},
	};
	float colors[12][4] = {
		{1.0, 0.0, 0.0, 0.0},
		{0.0, 1.0, 0.0, 0.0},
		{0.0, 0.0, 1.0, 0.0},
		{1.0, 1.0, 1.0, 0.0},

		{1.0, 0.0, 0.0, 0.0},
		{0.0, 1.0, 0.0, 0.0},
		{0.0, 0.0, 1.0, 0.0},
		{1.0, 1.0, 1.0, 0.0},

		{1.0, 0.0, 0.0, 0.0},
		{0.0, 1.0, 0.0, 0.0},
		{0.0, 0.0, 1.0, 0.0},
		{1.0, 1.0, 1.0, 0.0},
	};
	int x, y;
	static GLboolean once = GL_TRUE;

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColorPointer(4, GL_FLOAT, 0, colors);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glEnableClientState(GL_VERTEX_ARRAY);

	/* left: 1 prim */
	glDrawArrays(GL_QUADS, 0, 4);

	/* right: 1 prim */
	glDrawArrays(GL_QUADS, 4, 8);

	if (once) {
		printf("Left and right half should match.\n");
		once = GL_FALSE;
	}

	glutSwapBuffers();

	for (x = 0; x < piglit_width / 2; x++) {
		for (y = 0; y < piglit_height; y++) {
			float probed[4];
			glReadPixels((piglit_width + 1) / 2 + x,
				     y,
				     1, 1,
				     GL_RGBA, GL_FLOAT, &probed);

			pass = pass && piglit_probe_pixel_rgb(x, y, probed);
		}
	}

	return pass ? PIGLIT_SUCCESS : PIGLIT_WARN;
}

void
piglit_init(int argc, char *argv[])
{

}
