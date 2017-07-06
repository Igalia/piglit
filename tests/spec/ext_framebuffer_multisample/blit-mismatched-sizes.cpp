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
 * @file blit-mismatched-sizes.cpp
 *
 * This test verifies if glBlitFramebuffer() throws GL_INVALID_OPERATION with
 * non-matching rectangle sizes for multisample framebuffer objects.
 *
 * We initialize two FBOs with minimum supported sample count, do blitting
 * operation between non-matching rectangle sizes and then query the gl error.
 *
 * Author: Anuj Phogat <anuj.phogat@gmail.com>
 */

#include "piglit-test-pattern.h"
#include "piglit-fbo.h"
using namespace piglit_util_fbo;
using namespace piglit_util_test_pattern;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 256;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

const int pattern_width = 256; const int pattern_height = 256;
Fbo src_fbo, dst_fbo;

enum piglit_result
piglit_display()
{
	bool pass = true;

	/* Blit multisample-to-multisample with non-matching rectangle sizes */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo.handle);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width / 2, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Scaling blits are not allowed for multisample frambuffers. Here
	 * GL_INVALID_OPERATION is an expected gl error
	 */
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	return (pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_vertex_array_object");

	/* Passing sample count = 1 will create the FBOs with minimum supported
	 * sample count.
	 */
	src_fbo.setup(FboConfig(1 /* sample count */,
				pattern_width,
				pattern_height));

	dst_fbo.setup(FboConfig(1 /* sample count */,
				pattern_width,
				pattern_height));

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error setting up frame buffer objects\n");
		piglit_report_result(PIGLIT_FAIL);
	}

}
