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
 */

/** @file arb_es2_compatibility-maxvectors.c
 *
 * Tests that MAX_*_VECTORS = MAX_*_COMPONENTS / 4.
 */

#include "piglit-util.h"

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAILURE;
}

void
piglit_init(int argc, char **argv)
{
	GLboolean pass = GL_TRUE;
	GLint floats, vecs;

	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	if (!GLEW_ARB_ES2_compatibility) {
		printf("Requires ARB_ES2_compatibility\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	glGetIntegerv(GL_MAX_VARYING_FLOATS, &floats);
	glGetIntegerv(GL_MAX_VARYING_VECTORS, &vecs);
	if (floats / 4 != vecs) {
		printf("GL_MAX_VARYING_FLOATS / 4 != GL_MAX_VARYING_VECTORS "
		       "(%d, %d)\n", floats, vecs);
		pass = GL_FALSE;
	}

	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &floats);
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &vecs);
	if (floats / 4 != vecs) {
		printf("GL_MAX_VERTEX_UNIFORM_COMPONENTS / 4 != "
		       "GL_MAX_VERTEX_UNIFORM_VECTORS "
		       "(%d, %d)\n", floats, vecs);
		pass = GL_FALSE;
	}

	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &floats);
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &vecs);
	if (floats / 4 != vecs) {
		printf("GL_MAX_FRAGMENT_UNIFORM_COMPONENTS / 4 != "
		       "GL_MAX_FRAGMENT_UNIFORM_VECTORS "
		       "(%d, %d)\n", floats, vecs);
		pass = GL_FALSE;
	}

	assert(!glGetError());

	if (!pass)
		piglit_report_result(PIGLIT_FAILURE);
	else
		piglit_report_result(PIGLIT_SUCCESS);
}
