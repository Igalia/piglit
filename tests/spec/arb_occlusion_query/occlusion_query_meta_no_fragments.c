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
 * \file occlusion_query_meta_no_fragments.c
 *
 * Verify that various operations, (potentially implemented as
 * meta-operations within the OpenGL implementation), do not generate
 * fragments as specified.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

static bool
verify_no_fragments(GLuint query, const char *operation)
{
	GLint result;

	glGetQueryObjectiv(query, GL_QUERY_RESULT, &result);

	if (result != 0) {
		printf("Occlusion query for %s resulted in %d samples, (expected 0)\n", operation, result);
		return false;
	}

	return true;
}

/* Draw several things that should not generate fragments, each within
 * an occlusion query. Then verify that each query returns 0.
 */

enum piglit_result
piglit_display(void)
{
	/* 2x2 texture data: Red, Green, Blue, and White. */
	float data[16] = { 1.0, 0.0, 0.0,
			   0.0, 1.0, 0.0,
			   0.0, 0.0, 1.0,
			   1.0, 1.0, 1.0 };
	GLuint texture, texture_copy, fb;
	GLuint query;
	int test_pass = 1;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glGenQueries(1, &query);
	glGenTextures(1, &texture);
	glGenTextures(1, &texture_copy);
	glBindTexture(GL_TEXTURE_2D, texture);
	glGenFramebuffers(1, &fb);

	/* No fragments for glClear
	 *
	 * Clear is specified to bypass most of the fragment pipeline:
	 *
	 *	 When Clear is called, the only per-fragment
	 *	operations that are applied (if enabled) are
	 *	the pixel ownership test, the scissor test,
	 *	and dithering. [OpenGL 3.1 § 4.2.3]
	 */
	glBeginQuery(GL_SAMPLES_PASSED, query);
	{
		glClearColor(0.0, 1.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	glEndQuery(GL_SAMPLES_PASSED);
	test_pass &= verify_no_fragments(query, "glClear");

	/* No fragments for glGenerateMipmap
	 *
	 * This call does not affect the framebuffer, so
	 * should not generate any fragments. */
	glBeginQuery(GL_SAMPLES_PASSED, query);
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 2, 2, 0,
			     GL_RGB, GL_FLOAT, data);

		glGenerateMipmap(GL_TEXTURE_2D);
	}
	glEndQuery(GL_SAMPLES_PASSED);
	test_pass &= verify_no_fragments(query, "glGenerateMipmap");

	/* No fragments for glBlitFramebuffer
	 *
	 * The specification could not be more clear:
	 *
	 *	Blit operations bypass the fragment
	 *	pipeline. [OpenGL 3.1 § 4.3]
	 */
	glBeginQuery(GL_SAMPLES_PASSED, query);
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fb);

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					  GL_COLOR_ATTACHMENT0_EXT,
					  GL_TEXTURE_2D,
					  texture,
					  0);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glBlitFramebuffer(0, 0, 2, 2,
				  2, 2, 20, 20,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	}
	glEndQuery(GL_SAMPLES_PASSED);
	test_pass &= verify_no_fragments(query, "glBlitFramebuffer");

	/* No fragments for glCopyTexImage
	 *
	 * This call does not affect the framebuffer, so
	 * should not generate any fragments. */
	glBeginQuery(GL_SAMPLES_PASSED, query);
	{
		glBindTexture(GL_TEXTURE_2D, texture_copy);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,
				 0, 0, 2, 2, 0);
	}
	glEndQuery(GL_SAMPLES_PASSED);
	test_pass &= verify_no_fragments(query, "glCopyTexImage2D");

	/* Paint the copied texture just ensure it worked. */
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	piglit_draw_rect_tex(22, 2, 18, 18, 0, 0, 1, 1);

	/* No fragments for glCopyTexSubImage */
	glBeginQuery(GL_SAMPLES_PASSED, query);
	{
		glBindTexture(GL_TEXTURE_2D, texture_copy);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0,
				    1, 1, 0, 0, 1, 1);
		glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	}
	glEndQuery(GL_SAMPLES_PASSED);
	test_pass &= verify_no_fragments(query, "glCopyTexImage2D");

	/* Paint the copied texture so a user can see that it worked. */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	piglit_draw_rect_tex(42, 2, 18, 18, 0, 0, 1, 1);

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
