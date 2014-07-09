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
 * \file occlusion_query_meta_fragments.c
 *
 * Verify that various operations, (potentially implemented as
 * meta-operations within the OpenGL implementation), generate
 * fragments as specified.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

static bool
verify_fragments(GLuint query, const char *operation, int expected_fragments)
{
	GLint result;

	glGetQueryObjectiv(query, GL_QUERY_RESULT, &result);

	if (result == expected_fragments)
		return true;

	printf("Occlusion query for %s resulted in %d samples, (expected %d)\n",
	       operation, result, expected_fragments);
	return false;
}

/* Draw several things that should generate fragments, each within an
 * occlusion query. Then verify that each query returns a non-zero
 * value.
 */
enum piglit_result
piglit_display(void)
{
	/* 2x2 data: Red, Green, Blue, and White. */
	float data[16] = { 1.0, 0.0, 0.0,
			   0.0, 1.0, 0.0,
			   0.0, 0.0, 1.0,
			   1.0, 1.0, 1.0 };
	GLubyte bitmap[16] = { 0x5f, 0xff, 0xff, 0xff,
			       0xAf, 0xff, 0xff, 0xff,
			       0x5f, 0xff, 0xff, 0xff,
			       0xAf, 0xff, 0xff, 0xff };
	GLuint query;
	int test_pass = 1;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glGenQueries(1, &query);

	/* Fragments for glDrawPixels
	 *
	 * Assuming one fragment per pixel based on:
	 *
	 * 	Rectangles of color, depth, and certain
	 *	other values may be converted to fragments
	 *	using the DrawPixels command. [OpenGL 3.0 § 3.7]
	 */
	glBeginQuery(GL_SAMPLES_PASSED, query);
	{
		glRasterPos2i(2, 2);
		glDrawPixels(2, 2, GL_RGB, GL_FLOAT, data);
	}
	glEndQuery(GL_SAMPLES_PASSED);
	test_pass &= verify_fragments(query, "glDrawPixels", 4);

	/* Fragments for glCopyPixels
	 *
	 * And here, CopyPixels is specified to behave
	 * identically to DrawPixels:
	 *
	 *	The groups of elements so obtained are then
	 *	written to the framebuffer just as if
	 *	DrawPixels had been given width and height,
	 *	beginning with final conversion of elements.
	 *	[OpenGL 3.0 § 4.3.3]
	 */
	glBeginQuery(GL_SAMPLES_PASSED, query);
	{
		glRasterPos2i(6, 2);
		glCopyPixels(2, 2, 2, 2, GL_COLOR);
	}
	glEndQuery(GL_SAMPLES_PASSED);
	test_pass &= verify_fragments(query, "glCopyPixels", 4);

	/* Fragments for glBitmap.
	 *
	 * The specification implies very strongly that a bitmap
	 * should generate one fragment per set bit:
	 *
	 * 	Bitmaps are rectangles of zeros and ones
	 *	specifying a particular pattern of frag-
	 *	ments to be produced. [OpenGL 3.0 § 3.8]
	 */
	glBeginQuery(GL_SAMPLES_PASSED, query);
	{
		glRasterPos2i(10, 2);
		glColor4f(0.0, 0.0, 1.0, 0.0);
		glBitmap(4, 4, 0, 0, 0, 0, bitmap);
	}
	glEndQuery(GL_SAMPLES_PASSED);
	test_pass &= verify_fragments(query, "glBitmap", 8);

	glDeleteQueries(1, &query);

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
