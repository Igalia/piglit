/*
 * Copyright © 2012 Intel Corporation
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
 * \file api.c
 *
 * Test miscellaneous other entrypoints for GL_ARB_occlusion_query2.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = (PIGLIT_GL_VISUAL_RGB |
				PIGLIT_GL_VISUAL_DOUBLE |
				PIGLIT_GL_VISUAL_DEPTH);

PIGLIT_GL_TEST_CONFIG_END

static bool
test_error_begin_while_other_active(void)
{
	GLuint oq[2];
	bool pass = true;

	/* GL_ARB_occlusion_query2 specifies INVALID_OPERATION for
	 * starting either query type with the other one active.
	 */
	glGenQueries(2, oq);

	glBeginQuery(GL_SAMPLES_PASSED, oq[0]);
	if (!piglit_check_gl_error(0))
		pass = false;
	glBeginQuery(GL_ANY_SAMPLES_PASSED, oq[1]);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;
	glEndQuery(GL_ANY_SAMPLES_PASSED);
	glEndQuery(GL_SAMPLES_PASSED);
	piglit_reset_gl_error();

	glDeleteQueries(2, oq);

	glGenQueries(2, oq);

	glBeginQuery(GL_ANY_SAMPLES_PASSED, oq[0]);
	if (!piglit_check_gl_error(0))
		pass = false;
	glBeginQuery(GL_SAMPLES_PASSED, oq[1]);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;
	glEndQuery(GL_SAMPLES_PASSED);
	glEndQuery(GL_ANY_SAMPLES_PASSED);
	piglit_reset_gl_error();

	glDeleteQueries(2, oq);

	return pass;
}

static bool
test_counter_bits(void)
{
	GLint result = -1;

	/* From the GL_ARB_occlusion_query2 spec:
	 *
	 *   "Modify the paragraph beginning with "For occlusion
	 *   queries (SAMPLES_PASSED)..."
	 *
	 *       For occlusion queries
	 *    |  (SAMPLES_PASSED and ANY_SAMPLES_PASSED), the number of bits
	 *    |  depends on the target.  For a target of ANY_SAMPLES_PASSED, if
	 *    |  the number of bits is non-zero,  the minimum number of bits
	 *    |  is 1.  For a target of SAMPLES_PASSED,
	 *       if the number of bits is non-zero, ..."
	 *
	 * So, the number of bits has to be either a zero or >= 1.
	 */
	glGetQueryiv(GL_ANY_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &result);
	if (result < 0) {
		fprintf(stderr, "GL_QUERY_COUNTER_BITS returned %d\n", result);
		return false;
	}
	return true;
}

static bool
test_error_begin_wrong_target(void)
{
	bool pass = true;
	GLuint oq;

	glGenQueries(1, &oq);

	glBeginQuery(GL_SAMPLES_PASSED, oq);
	if (!piglit_check_gl_error(0))
		pass = false;
	glEndQuery(GL_SAMPLES_PASSED);

	/* From the OpenGL 3.3 spec, section "2.14. ASYNCHRONOUS QUERIES", page 94:
	 *
	 *    "[...] if id is the name of an existing query object whose type does not
	 *     match target, [...] the error INVALID_OPERATION is generated."
	 *
	 * Similar wording exists in the OpenGL ES 3.0.0 spec, section "2.13.
	 * ASYNCHRONOUS QUERIES", page 82.
	 */
	glBeginQuery(GL_ANY_SAMPLES_PASSED, oq);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;
	glEndQuery(GL_ANY_SAMPLES_PASSED);
	piglit_reset_gl_error();

	glDeleteQueries(1, &oq);

	return pass;
}

static bool
test_error_end_wrong_target(void)
{
	bool pass = true;
	GLuint oq;

	glGenQueries(1, &oq);

	/* From the GL_ARB_occlusion_query2 spec:
	 *
	 *     "If EndQueryARB is called while no query with the same
	 *      target is in progress, an INVALID_OPERATION error is
	 *      generated."
	 */
	glBeginQuery(GL_SAMPLES_PASSED, oq);
	if (!piglit_check_gl_error(0))
		pass = false;
	glEndQuery(GL_ANY_SAMPLES_PASSED);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;
	glEndQuery(GL_SAMPLES_PASSED);
	piglit_reset_gl_error();

	glDeleteQueries(1, &oq);

	glGenQueries(1, &oq);

	glBeginQuery(GL_ANY_SAMPLES_PASSED, oq);
	if (!piglit_check_gl_error(0))
		pass = false;
	glEndQuery(GL_SAMPLES_PASSED);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;
	glEndQuery(GL_ANY_SAMPLES_PASSED);
	piglit_reset_gl_error();

	glDeleteQueries(1, &oq);

	return pass;
}

static bool
test_current_query(void)
{
	bool pass = true;
	GLint result = -1;
	GLuint oq;

	glGenQueries(1, &oq);

	/* Test that GL_CURRENT_QUERY returns our target and not the
	 * other one. First, check that we're inactive after the
	 * previous sequence of query code.
	 */
	result = -1;
	glGetQueryiv(GL_ANY_SAMPLES_PASSED, GL_CURRENT_QUERY, &result);
	if (result != 0) {
		fprintf(stderr,
			"GL_CURRENT_QUERY(GL_ANY_SAMPLES_PASSED) returned %d "
			"while inactive\n",
			result);
		pass = false;
	}
	result = -1;
	glGetQueryiv(GL_SAMPLES_PASSED, GL_CURRENT_QUERY, &result);
	if (result != 0) {
		fprintf(stderr,
			"GL_CURRENT_QUERY(GL_SAMPLES_PASSED) returned %d "
			"while inactive\n",
			result);
		pass = false;
	}

	/* Test the result for GL_ANY_SAMPLES_PASSED active */
	glBeginQuery(GL_ANY_SAMPLES_PASSED, oq);
	result = -1;
	glGetQueryiv(GL_ANY_SAMPLES_PASSED, GL_CURRENT_QUERY, &result);
	if (result != oq) {
		fprintf(stderr,
			"GL_CURRENT_QUERY(GL_ANY_SAMPLES_PASSED) returned %d "
			"while GL_ANY_SAMPLES_PASSED active\n",
			result);
		pass = false;
	}
	result = -1;
	glGetQueryiv(GL_SAMPLES_PASSED, GL_CURRENT_QUERY, &result);
	if (result != 0) {
		fprintf(stderr,
			"GL_CURRENT_QUERY(GL_SAMPLES_PASSED) returned %d while "
			"GL_ANY_SAMPLES_PASSED active\n",
			result);
		pass = false;
	}
	glEndQuery(GL_ANY_SAMPLES_PASSED);
	glDeleteQueries(1, &oq);

	glGenQueries(1, &oq);

	/* Test the result for GL_SAMPLES_PASSED active */
	glBeginQuery(GL_SAMPLES_PASSED, oq);
	result = -1;
	glGetQueryiv(GL_ANY_SAMPLES_PASSED, GL_CURRENT_QUERY, &result);
	if (result != 0) {
		fprintf(stderr,
			"GL_CURRENT_QUERY(GL_ANY_SAMPLES_PASSED) returned %d "
			"while GL_SAMPLES_PASSED active\n",
			result);
		pass = false;
	}
	result = -1;
	glGetQueryiv(GL_SAMPLES_PASSED, GL_CURRENT_QUERY, &result);
	if (result != oq) {
		fprintf(stderr,
			"GL_CURRENT_QUERY(GL_SAMPLES_PASSED) returned %d "
			"while GL_SAMPLES_PASSED active\n",
			result);
		pass = false;
	}
	glEndQuery(GL_SAMPLES_PASSED);

	glDeleteQueries(1, &oq);

	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	pass = test_counter_bits() && pass;
	pass = test_current_query() && pass;
	pass = test_error_begin_wrong_target() && pass;
	pass = test_error_end_wrong_target() && pass;
	pass = test_error_begin_while_other_active() && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

	/* UNREACHED */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_occlusion_query2");
}
