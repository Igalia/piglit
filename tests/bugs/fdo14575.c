/*
 * Copyright © 2007 Intel Corporation
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
 * @file fdo14575.c
 *
 * Tests that the driver doesn't fail when deleting a mapped buffer object.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char**argv)
{
	GLfloat data = 1.0;
	GLfloat *v;
	GLuint buf;

	piglit_require_extension("GL_ARB_vertex_buffer_object");

	glGenBuffersARB(1, &buf);

	/* First, do a normal buffer create/data/delete */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, buf);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, 4, &data, GL_STATIC_DRAW_ARB);
	glDeleteBuffersARB(1, &buf);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Then, another normal path: create, map, write, unmap, delete */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, buf);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, 4, NULL, GL_STATIC_DRAW_ARB);
	v = (GLfloat *) glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	*v = data;
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	glDeleteBuffersARB(1, &buf);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Then, do the failing path: create, map, delete */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, buf);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, 4, NULL, GL_STATIC_DRAW_ARB);
	v = (GLfloat *) glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	*v = data;
	glDeleteBuffersARB(1, &buf);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(PIGLIT_PASS);
}

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
