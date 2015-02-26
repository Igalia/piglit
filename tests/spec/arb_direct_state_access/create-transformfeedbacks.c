/*
 * Copyright 2015 Intel Corporation
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

/** @file create-transformfeedbacks.c
 *
 * Tests glCreateTransformFeedbacks to see if it behaves in the expected way,
 * throwing the correct errors, etc.
 *
 * From OpenGL 4.5, section 13.2.1 "Transform Feedback Objects", page 420:
 *
 * "CreateTransformFeedbacks returns n previously unused transform feedback
 * object names in ids, each representing a new state vector, comprising the
 * state and with all the same initial values listed in table 23.48.
 * Errors
 *   An INVALID_VALUE error is generated if n is negative."
 */

#include "piglit-util-gl.h"
#include "dsa-utils.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_direct_state_access");
	piglit_require_extension("GL_ARB_transform_feedback3");
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLuint ids[10];

	/* Throw some invalid inputs at glCreateTransformFeedbacks. */

	/* n is negative */
	glCreateTransformFeedbacks(-1, ids);
	SUBTEST(GL_INVALID_VALUE, pass, "n < 0");

	/* Throw some valid inputs at glCreateTransformFeedbacks. */

	/* n is zero */
	glCreateTransformFeedbacks(0, NULL);
	SUBTEST(GL_NO_ERROR, pass, "n == 0");

	/* n is more than 1 */
	glCreateTransformFeedbacks(10, ids);
	SUBTEST(GL_NO_ERROR, pass, "n > 1");

	SUBTESTCONDITION(glIsTransformFeedback(ids[2]), pass,
			"IsTransformFeedback()");

	/* the default state is tested in the following piglit test:
	 *	arb_direct_state_access-gettransformfeedback
	 */

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
