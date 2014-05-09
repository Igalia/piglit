/*
 * Copyright © 2013 Intel Corporation
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

/** \file attachments-blit-scaled-linear.cpp
 *
 * This test verifies that same output is produced by scaled blitting
 * of texture or renderbuffer color attachments with GL_LINEAR filter.
 * It compares the output from following rendering scenarios:
 * 1. Scaled blit using a single-sampled framebuffer with texture attachment.
 * 2. Scaled blit using a single-sampled framebuffer with renderbuffer
 *    attachment.
 */

#include "piglit-test-pattern.h"
#include "piglit-fbo.h"
using namespace piglit_util_fbo;
using namespace piglit_util_test_pattern;

const int pattern_width = 258; const int pattern_height = 258;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = pattern_width * 2;
	config.window_height = pattern_height;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static TestPattern *test_pattern;
const int srcX0 = 6, srcY0 = 7, dstX0 = 0, dstY0 = 0;
const int srcX1 = pattern_width / 2, srcY1 = pattern_height / 2;
static Fbo fbo_tex, fbo_rb;

void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(21);
	/* Create two singlesample FBOs with same format and dimensions but
	 * different color attachment types.
	 */
	FboConfig Config(0, pattern_width, pattern_height);
	fbo_rb.setup(Config);
	Config.num_rb_attachments = 0;
	Config.num_tex_attachments = 1;
	fbo_tex.setup(Config);

	test_pattern = new Triangles();
	test_pattern->compile();

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}
}

bool test_blit_scaled_linear()
{
	GLfloat scale;
	GLint samples;
	bool pass = true, result = true;

	/* Draw the test pattern into the framebuffer with texture
	 * attachment.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_tex.handle);
	glViewport(0, 0, srcX1, srcY1);
	glGetIntegerv(GL_SAMPLES, &samples);
	glClear(GL_COLOR_BUFFER_BIT);
	test_pattern->draw(TestPattern::no_projection);

	/* Blit the framebuffer with texture attachment into the
	 * framebuffer with renderbuffer attachment without scaling.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_tex.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_rb.handle);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlitFramebuffer(0, 0,
			  fbo_tex.config.width,
			  fbo_tex.config.height,
			  0, 0,
			  fbo_tex.config.width,
			  fbo_tex.config.height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	for(scale = 0.1; scale < 2.5f; scale += 0.1) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glClear(GL_COLOR_BUFFER_BIT);

		/* Do scaled blit of fbo_tex to left half of piglit_winsys_fbo.
                 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_tex.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
                glClearColor(0.0, 1.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
                glClearColor(0.0, 0.0, 0.0, 0.0);
		glEnable(GL_SCISSOR_TEST);
		glScissor(0, 0, pattern_width, pattern_height);
		glBlitFramebuffer(srcX0, srcY0,
				  srcX1, srcY1,
				  dstX0, dstY0,
				  dstX0 + srcX1 * scale, dstY0 + srcY1 * scale,
				  GL_COLOR_BUFFER_BIT,
				  GL_LINEAR);
		glDisable(GL_SCISSOR_TEST);

		/* Do scaled blit of fbo_rb to right half piglit_winsys_fbo. */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_rb.handle);
		glBlitFramebuffer(srcX0, srcY0,
				  srcX1, srcY1,
				  pattern_width + dstX0, dstY0,
				  pattern_width + dstX0 + srcX1 * scale,
				  dstY0 + srcY1 * scale,
				  GL_COLOR_BUFFER_BIT,
				  GL_LINEAR);

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
		glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
		result = piglit_probe_rect_halves_equal_rgba(0, 0,
                                                           piglit_width,
                                                           piglit_height);
		pass = result && pass;
		piglit_present_results();
		printf("scale = %f, result = %s\n",
		       scale, result ? "pass" : "fail");
	}
	return pass;
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	printf("Left Image: Linear scaled blit using texture attachment.\n"
	       "Right Image: Linear scaled blit using renderbuffer attachment.\n");
	pass = test_blit_scaled_linear()
	       && pass;
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
