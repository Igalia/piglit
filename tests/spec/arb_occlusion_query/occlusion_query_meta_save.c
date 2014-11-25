/*
 * Copyright © 2014 Intel Corporation
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
 *    Neil Roberts <neil@linux.intel.com>
 */

/**
 * \file occlusion_query_meta_save.c
 *
 * Verify that doing a clear (which is potentially implemented as a
 * meta operation) doesn't reset the samples-passed count back to
 * zero.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	GLuint query;
	GLint result = -1;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glGenQueries(1, &query);

	glBeginQuery(GL_SAMPLES_PASSED, query);

	/* Render 64 pixels. This should affect the query */
	piglit_draw_rect(0, 0, 8, 8);

	/* Clear the framebuffer. This shouldn't affect the query */
	glClearColor(0.0, 0.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Render another 64 pixels. This should continue adding to
	 * the query */
	piglit_draw_rect(4, 0, 8, 8);

	glEndQuery(GL_SAMPLES_PASSED);

	glGetQueryObjectiv(query, GL_QUERY_RESULT, &result);

	glDeleteQueries(1, &query);

	piglit_present_results();

	if (result != 128) {
		printf("Occlusion query resulted in %d samples "
		       "(expected 128)\n",
		       result);
		return PIGLIT_FAIL;
	} else {
		return PIGLIT_PASS;
	}
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
	glGetQueryiv(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &query_bits);
	if (query_bits == 0)
		piglit_report_result(PIGLIT_SKIP);
}
