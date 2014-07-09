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
 */

/**
 * \file dlist-fdo3129-01.c
 * Test odd combinations of command in a display list.
 *
 * This test is based on a test case posted to fdo bug #3129 by David Reveman.
 * Once upon a time, this triggered an assertion failure in Mesa.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 256;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint list;

enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_PASS;
	static const GLfloat color[4] = { 1.0, 1.0, 1.0, 1.0 };

	glClear (GL_COLOR_BUFFER_BIT);

	/* Set some values outside the list.
	 */
	glColor3fv(color);
	glNormal3f(1.0, 0.0, 0.0);


	/* Compile a list.  Reset one of the parameters after the first vertex
	 * in the list.
	 */
	glNewList(list, GL_COMPILE);

	glBegin(GL_QUADS);
	glVertex2f(piglit_width / 4, piglit_height / 4);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex2f((piglit_width * 3) / 4, piglit_height / 4);
	glVertex2f((piglit_width * 3) / 4, (piglit_height * 3) / 4);
	glVertex2f(piglit_width / 4, (piglit_height * 3) / 4);
	glEnd();

	glEndList();

	glCallList(list);

	if (!piglit_probe_pixel_rgb(piglit_width / 2,
				    piglit_height / 2,
				    color)) {
		result = PIGLIT_FAIL;
	}

	piglit_present_results();

	return result;
}


void
piglit_init(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	list = glGenLists(1);
}
