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

/** @file create-queries.c
 *
 * Tests glCreateQueries to see if it behaves in the expected way, throwing
 * the correct errors, etc.
 *
 * From OpenGL 4.5, section 4.2 "Query Objects and Asynchronous Queries",
 * page 42:
 *
 * "void CreateQueries( enum target, sizei n, uint *ids );
 *
 * CreateQueries returns n previously unused query object names in ids, each
 * representing a new query object with the specified target. target may be
 * one of SAMPLES_PASSED, ANY_SAMPLES_PASSED, ANY_SAMPLES_PASSED_CONSERVATIVE,
 * TIME_ELAPSED, TIMESTAMP, PRIMITIVES_GENERATED, and
 * TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN. The initial state of the resulting
 * query object is that the result is marked available (the value of
 * QUERY_RESULT_AVAILABLE for the query object is TRUE) and the result
 * value (the value of QUERY_RESULT) is zero.
 *
 * Errors
 * An INVALID_ENUM error is generated if target is not one of the targets
 *  listed above.
 * An INVALID_VALUE error is generated if n is negative.
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
	piglit_require_extension("GL_ARB_timer_query");
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLuint ids[10];
	GLint param;

	/* Throw some invalid inputs at glCreateQueries */

	/* n is negative */
	glCreateQueries(GL_SAMPLES_PASSED, -1, ids);
	SUBTEST(GL_INVALID_VALUE, pass, "n < 0");

	/* invalid target */
	glCreateQueries(GL_RGBA, 0, ids);
	SUBTEST(GL_INVALID_ENUM, pass, "invalid target");

	/* Throw some valid inputs at glCreateQueries. */

	/* n is zero */
	glCreateQueries(GL_SAMPLES_PASSED, 0, NULL);
	SUBTEST(GL_NO_ERROR, pass, "n == 0");

	/* n is more than 1 */
	glCreateQueries(GL_SAMPLES_PASSED, 10, ids);
	SUBTEST(GL_NO_ERROR, pass, "n > 1");

	/* test the default state of dsa-created query objects */
	SUBTESTCONDITION(glIsQuery(ids[2]), pass, "IsQuery()");
	glGetQueryObjectiv(ids[2], GL_QUERY_RESULT_AVAILABLE, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == GL_TRUE, pass,
			 "default AVAILABLE state(%d) == TRUE", param);
	glGetQueryObjectiv(ids[2], GL_QUERY_RESULT, &param);
	piglit_check_gl_error(GL_NO_ERROR);
	SUBTESTCONDITION(param == 0, pass, "default RESULT(%d) == 0", param);

	/* clean up */
	glDeleteQueries(10, ids);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
