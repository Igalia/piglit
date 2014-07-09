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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file getenv4d-with-error.c
 *
 * Tests for a bug in Mesa where glGetProgramEnvParameter4dARB would
 * fail to update the result if there was an existing GL error in the
 * context.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	double test_data[4] = { 0.1, 0.2, 0.3, 0.4 };
	double result_data[4];
	float epsilon = .00001;

	piglit_require_extension("GL_ARB_vertex_program");

	glProgramEnvParameter4dARB(GL_VERTEX_PROGRAM_ARB, 0,
				   test_data[0],
				   test_data[1],
				   test_data[2],
				   test_data[3]);

	/* Produce a GL error to trick Mesa's
	 * glGetProgramEnvParameterdvARB code.
	 */
	glDepthFunc(0xd0d0d0d0);

	glGetProgramEnvParameterdvARB(GL_VERTEX_PROGRAM_ARB, 0, result_data);

	if (fabs(test_data[0] - result_data[0]) > epsilon ||
	    fabs(test_data[1] - result_data[1]) > epsilon ||
	    fabs(test_data[2] - result_data[2]) > epsilon ||
	    fabs(test_data[3] - result_data[3]) > epsilon) {
		fprintf(stderr, "glProgramEnvParamter4dvARB failed:\n");
		fprintf(stderr, "Expected: (%f %f %f %f)\n",
			test_data[0],
			test_data[1],
			test_data[2],
			test_data[3]);
		fprintf(stderr, "Expected: (%f %f %f %f)\n",
			result_data[0],
			result_data[1],
			result_data[2],
			result_data[3]);

		pass = false;
	}

	/* Clear our error. */
	(void)glGetError();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
