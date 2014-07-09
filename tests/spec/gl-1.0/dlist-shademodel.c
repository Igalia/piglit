/*
 * Copyright (C) 2013 VMware, Inc.
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
 * Test glShadeModel in a display list.
 * This is pretty trivial and shouldn't fail with any decent OpenGL,
 * but it's useful for checking an optimization in Mesa's display list
 * compiler.
 */

#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


static const GLfloat red[] = {1.0, 0.0, 0.0};
static const GLfloat green[] = {0.0, 1.0, 0.0};


enum piglit_result
piglit_display(void)
{
	GLuint list;
	bool pass;

	list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	glShadeModel(GL_FLAT);

	glBegin(GL_QUADS);
	glColor3fv(red);
	glVertex2f(-1, -1);
	glColor3fv(green);
	glVertex2f( 0, -1);
	glColor3fv(red);
	glVertex2f( 0, 1);
	glColor3fv(green);
	glVertex2f(-1, 1);
	glEnd();

	/* Mesa should be able to optimize this away so that
	 * the two GL_QUADS primitives get combined into one batch.
	 */
	glShadeModel(GL_FLAT);

	glBegin(GL_QUADS);
	glColor3fv(red);
	glVertex2f(0, -1);
	glColor3fv(green);
	glVertex2f(1, -1);
	glColor3fv(red);
	glVertex2f(1, 1);
	glColor3fv(green);
	glVertex2f(0, 1);
	glEnd();

	glEndList();

	glShadeModel(GL_SMOOTH);
	glClear(GL_COLOR_BUFFER_BIT);
	glCallList(list);

	glDeleteLists(list, 1);

	pass = piglit_probe_pixel_rgb(20, 20, green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	/* nothing */
}
