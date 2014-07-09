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
 * \file occlusion_query_order.c
 *
 * Verify that once one occlusion query has results, all previous
 * occlusion queries also have results available, as per the spec.:
 *
 *	It must always be true that if any query object
 *	returns a result available of TRUE, all queries of
 *	the same type issued prior to that query must also
 *	return TRUE. [OpenGL 3.1 § 6.1.6]
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

static void
draw_some_things(double frac)
{
	int i;
	float x, y;

	x = 0.0;
	y = frac * piglit_height;

	glBegin(GL_QUADS);

	for (i=0; i < 1024; i++) {
		glVertex3f(x, y, 0.0);
		glVertex3f(x + 1.0, y, 0.0);
		glVertex3f(x + 1.0, y + 1.0, 0.0);
		glVertex3f(x, y + 1.0, 0.0);

		x++;
		if (x >= piglit_width) {
			x = 0.0;
			y++;
			if (y >= piglit_height) {
				y = 0.0;
			}
		}
	}

	glEnd();
}

enum piglit_result
piglit_display(void)
{
#define NUM_QUERIES 5
	GLuint queries[NUM_QUERIES], available;
	bool test_pass = true;
	GLint result;
	int i;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glGenQueries(NUM_QUERIES, queries);

	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

	/* Queue up a bunch of drawing with several queries. */
	for (i = 0; i < NUM_QUERIES - 1;  i++)
	{
		glBeginQuery(GL_SAMPLES_PASSED, queries[i]);

		draw_some_things((double)i / (NUM_QUERIES - 1));

		glEndQuery(GL_SAMPLES_PASSED);
	}

	/* Now fire off a query with no drawing. */
	glBeginQuery(GL_SAMPLES_PASSED, queries[NUM_QUERIES - 1]);
	glEndQuery(GL_SAMPLES_PASSED);

	/* Get the result for the final query. */
	glGetQueryObjectiv(queries[NUM_QUERIES - 1], GL_QUERY_RESULT, &result);

	/* At this point, the results of all the previous queries
	 * should be available.
	 */
	for (i = 0; i < NUM_QUERIES - 1; i++)
	{
		glGetQueryObjectuiv(queries[i], GL_QUERY_RESULT_AVAILABLE,
				    &available);
		if (available != 1) {
			printf("Query #%d result not available (expected in-order processing)\n", i);
			test_pass = false;
		}
	}

	glDeleteQueries(NUM_QUERIES, queries);

	piglit_present_results();

	return test_pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint query_bits;

	piglit_require_extension("GL_ARB_occlusion_query");

	/* It is legal for a driver to support the query API but not have
	 * any query bits.  I wonder how many applications actually check for
	 * this case...
	 */
	glGetQueryiv(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS,
		       & query_bits);
	if (query_bits == 0) {
		piglit_report_result(PIGLIT_SKIP);
	}
}
