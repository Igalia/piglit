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
 * Test a long-ish display list to make sure Mesa's display list
 * implementation (linked list of blocks) works properly.
 * Ideally, this test should be run with valgrind.
 */

#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


static GLuint
build_long_list(void)
{
	static const GLfloat color[4] = { 1.0f, 0.5f, 0.25f, 1.0f };
	GLuint list, i;

	list = glGenLists(1);
	glNewList(list, GL_COMPILE);

	/* build a list of non-vertex commands (since vertex data is
	 * typically put into a VBO).
	 */
	for (i = 0; i < 10 * 1000; i++) {
		glEnable(GL_CULL_FACE);
		glLightfv(GL_LIGHT0, GL_AMBIENT, color);
		glStencilOp(GL_KEEP, GL_INCR, GL_DECR);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
		glBindTexture(GL_TEXTURE_2D, 1);
		glPointSize(1.0);
		glFogf(GL_FOG_DENSITY, 5.0);
		glDisable(GL_FOG);
	}

	glEndList();

	return list;
}


enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
	GLuint l1, l2;

	l1 = build_long_list();
	l2 = build_long_list();

	glCallList(l1);
	glCallList(l2);
	glDeleteLists(l1, 1);
	glCallList(l2);
	glDeleteLists(l2, 1);

	/* if we get here, it means we didn't crash at least. */

	piglit_report_result(PIGLIT_PASS);
}
