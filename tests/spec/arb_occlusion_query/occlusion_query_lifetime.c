/*
 * Copyright © 2009,2012 Intel Corporation
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
 *    Ian Romanick <ian.d.romanick@intel.com>
 *    Carl Worth <cworth@cworth.org>
 */

/**
 * \file occlusion_query_lifetime.c
 *
 * Ensure that glIsQuery reports correct values throughout each stage
 * of a query's lifetime.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

/* Check is glIsQuery() for 'query' returns 'expected'
 */
static bool
is_query_matches(GLuint query, GLboolean expected, const char *lifetime)
{
	int is_query = glIsQuery(query);

	if (is_query != expected) {
		fprintf(stderr, "glIsQuery returned %d (expected %d) %s\n",
		       is_query, expected, lifetime);
		return false;
	}

	return true;
}

enum piglit_result
piglit_display(void)
{
	int test_pass = 1;
	GLuint query;
	GLint result;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Guaranteed to be random, see: http://xkcd.com/221
	 */
	query = 3243;
	test_pass &= is_query_matches(query, 0, "with un-generated name");

	glGenQueries(1, &query);
	test_pass &= is_query_matches(query, 0, "after glGenQueries");

	glBeginQuery(GL_SAMPLES_PASSED, query);
	test_pass &= is_query_matches(query, 1, "after glBeginQuery");

	/* Do a little drawing at least */
	glColor3ub(0x00, 0xff, 0x00);
	piglit_draw_rect(0, 0, piglit_width, piglit_height);

	glEndQuery(GL_SAMPLES_PASSED);
	test_pass &= is_query_matches(query, 1, "after glEndQuery");

	glGetQueryObjectiv(query, GL_QUERY_RESULT, &result);
	test_pass &= is_query_matches(query, 1, "after glGetQueryObjectiv");

	glDeleteQueries(1, &query);
	test_pass &= is_query_matches(query, 0, "after glDeleteQueries");

	piglit_present_results();

	return test_pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_occlusion_query");
}
