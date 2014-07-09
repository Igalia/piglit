/*
 * Copyright © 2009,2012,2013 Intel Corporation
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
 * \file gen-delete-while-active.c
 *
 * Ensure that both glGenQueries and glDeleteQuery can be called on a
 * new object while another query object is active. Also, that
 * glDeleteQuery can be called on an active query object.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	GLuint active, inactive;
	GLint elapsed;
 
	/* Generate and start a query. */
	glGenQueries(1, &active);

	glBeginQuery(GL_SAMPLES_PASSED, active);

	printf ("Testing Gen/Delete of query while another query is active.\n");
	{
		/* While first query is active, gen a new one. */
		glGenQueries(1, &inactive);

		if (!piglit_check_gl_error(GL_NO_ERROR))
			return PIGLIT_FAIL;

		/* Delete the inactive query. */
		glDeleteQueries(1, &inactive);

		if (!piglit_check_gl_error(GL_NO_ERROR))
			return PIGLIT_FAIL;

		/* Finish and get result from active query. */
		glEndQuery(GL_SAMPLES_PASSED);

		glGetQueryObjectiv(active, GL_QUERY_RESULT, &elapsed);
	}

	printf ("Testing Delete of currently-active query.\n");
	{
		/* Finally, ensure that an active query can be deleted. */
		glGenQueries(1, &active);

		glBeginQuery(GL_SAMPLES_PASSED, active);

		glDeleteQueries(1, &active);

		if (!piglit_check_gl_error(GL_NO_ERROR))
			return PIGLIT_FAIL;
	}

	printf ("Testing that glEndQuery on deleted query (expecting error).\n");
	{
		/* And ensure that we get an error if we try to end a deleted
		 * query. */
		glEndQuery(GL_SAMPLES_PASSED);

		if (!piglit_check_gl_error(GL_INVALID_OPERATION))
			return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_occlusion_query");
}
