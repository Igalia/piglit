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

/** @file dlist-clear.c
 *
 * Tests that clears and primitives get stored properly in a
 * COMPILE_AND_EXECUTE display list.  Caught a regression in the intel driver
 * with the new metaops clear code.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	static float red[]   = {1.0, 0.0, 0.0, 0.0};
	static float green[] = {0.0, 1.0, 0.0, 0.0};
	static float blue[]  = {0.0, 0.0, 1.0, 0.0};

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.5, 0.0, 0.0, 0.0);
	glColor4fv(red);

	/* Make a list containing a clear and a rectangle.  It'll draw
	 * colors we don't expect to see, due to COMPILE_AND_EXECUTE.
	 */
	glNewList(1, GL_COMPILE_AND_EXECUTE);
	/* Even though we don't use depth, GL_DEPTH_BUFFER_BIT is what
	 * triggered the metaops clear path which messed up the display list.
	 */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_QUADS);
		glVertex2f(10, 10);
		glVertex2f(20, 10);
		glVertex2f(20, 20);
		glVertex2f(10, 20);
	glEnd();
	glEndList();

	/* Now, set up our expected colors, translate the dlist's rectangle
	 * over a little, and do the draw we actually expect to see.
	 */
	glClearColor(0.0, 1.0, 0.0, 0.0);
	glColor4fv(blue);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(20, 0, 0);

	glCallList(1);

	pass &= piglit_probe_rect_rgb(0, 0, piglit_width, 10, green);

	pass &= piglit_probe_rect_rgb(0, 10, 30, 10, green);
	pass &= piglit_probe_rect_rgb(30, 10, 10, 10, blue);
	pass &= piglit_probe_rect_rgb(40, 10, piglit_width-40, 10, green);

	pass &= piglit_probe_rect_rgb(0, 20, piglit_width, piglit_height - 20,
				      green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
}
