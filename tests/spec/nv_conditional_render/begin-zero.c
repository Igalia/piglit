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

#include "piglit-util-gl-common.h"

/**
 * @file begin-zero.c
 *
 * Tests that starting conditional rendering on a 0 query object
 * results in INVALID_OPERATION.
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

PIGLIT_GL_TEST_MAIN(
    32 /*window_width*/,
    32 /*window_height*/,
    GLUT_RGB | GLUT_ALPHA)

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(20);

	piglit_require_extension("GL_NV_conditional_render");

	glBeginConditionalRenderNV(0, GL_QUERY_WAIT_NV);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(PIGLIT_PASS);
}
