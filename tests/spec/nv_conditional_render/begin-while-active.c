/*
 * Copyright © 2011 Intel Corporation
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

#include "piglit-util-gl.h"

/**
 * @file begin-while-active.c
 *
 * Tests that starting conditional rendering on a query object that is
 * active results in INVALID_OPERATION.
 *
 * From the NV_conditional_render spec:
 *
 *     "BeginQuery sets the active query object name for the query
 *      type given by <target> to <id>.  If BeginQuery is called with
 *      an <id> of zero, if the active query object name for <target>
 *      is non-zero, if <id> is the active query object name for any
 *      query type, or if <id> is the active query object for
 *      condtional rendering (Section 2.X), the error INVALID
 *      OPERATION is generated."
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint q;

	piglit_require_gl_version(20);

	piglit_require_extension("GL_NV_conditional_render");

	glGenQueries(1, &q);
	glBeginQuery(GL_SAMPLES_PASSED, q);
	glBeginConditionalRenderNV(q, GL_QUERY_WAIT_NV);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);
	glEndQuery(GL_SAMPLES_PASSED);
	glDeleteQueries(1, &q);

	piglit_report_result(PIGLIT_PASS);
}
