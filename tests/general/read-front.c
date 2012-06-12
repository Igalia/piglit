/*
 * Copyright © 2009 Intel Corporation
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

/** @file read-front.c
 *
 * Tests that reading the front buffer after a draw to back and swap
 * works correctly.
 *
 * This catches a regression in the Intel driver with DRI2, where the
 * read buffer didn't have an actual buffer present if it hadn't been
 * used as a draw buffer.
 */

#include "piglit-util.h"

PIGLIT_GL_TEST_MAIN(
    100 /*window_width*/,
    100 /*window_height*/,
    GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL)

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	static float blue[]   = {0.0, 0.0, 1.0, 0.0};
	static float green[] = {0.0, 1.0, 0.0, 0.0};

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.0, 0.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor4fv(green);
	piglit_draw_rect(0, piglit_height / 2, piglit_width, piglit_height);

	glReadBuffer(GL_FRONT);

	glutSwapBuffers();

	pass &= piglit_probe_rect_rgb(0, 0,
				      piglit_width, piglit_height / 2, blue);
	pass &= piglit_probe_rect_rgb(0, piglit_height / 2,
				      piglit_width, piglit_height / 2, green);

	glReadBuffer(GL_BACK);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
}
