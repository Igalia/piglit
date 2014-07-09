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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
#ifdef GL_ARB_ES2_compatibility
	GLboolean pass = GL_TRUE;
	GLint floats, vecs;

	piglit_require_gl_version(20);

	if (!piglit_is_extension_supported("GL_ARB_ES2_compatibility")) {
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
		piglit_report_result(PIGLIT_FAIL);
	else
		piglit_report_result(PIGLIT_PASS);
#else
	piglit_report_result(PIGLIT_SKIP);

#endif /* GL_ARB_ES2_compatibility */
}
