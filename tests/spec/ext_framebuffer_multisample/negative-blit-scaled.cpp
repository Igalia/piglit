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

/** \file negative-blit-scaled.cpp
 *
 * This test verifies that expected GL errors are produced for cases mentioned
 * in EXT_framebuffer_multisample_blit_scaled extension:
 *
 * "If the draw framebuffer is framebuffer complete and has a value of
 * SAMPLE_BUFFERS that is greater than zero, or if the read framebuffer
 * is framebuffer complete and has a value of SAMPLE_BUFFERS that is
 * zero, then the error INVALID_OPERATION is generated if BlitFramebuffer
 * is called and the filter is SCALED_RESOLVE_FASTEST_EXT or
 * SCALED_RESOLVE_NICEST_EXT."
 *
 */

#include "common.h"

const int pattern_width = 256; const int pattern_height = 256;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = pattern_width;
	config.window_height = pattern_height;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static Fbo multisampled_fbo_1, multisampled_fbo_2, singlesampled_fbo;

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLint max_samples;

	piglit_require_extension("GL_EXT_framebuffer_multisample_blit_scaled");

	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	FboConfig Config(max_samples, pattern_width, pattern_height);
	multisampled_fbo_1.setup(Config);
	multisampled_fbo_2.setup(Config);
	Config.num_samples = 0;
	singlesampled_fbo.setup(Config);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}

        /* Do multi-sample to multi-sample scaled blit */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampled_fbo_1.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampled_fbo_2.handle);

	glBlitFramebuffer(0, 0, pattern_width / 2, pattern_height / 2,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_SCALED_RESOLVE_FASTEST_EXT);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	glBlitFramebuffer(0, 0, pattern_width / 2, pattern_height / 2,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_SCALED_RESOLVE_NICEST_EXT);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	/* Do single-sample to single-sample scaled blit */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, singlesampled_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);

	glBlitFramebuffer(0, 0, pattern_width / 2, pattern_height / 2,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_SCALED_RESOLVE_FASTEST_EXT);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	glBlitFramebuffer(0, 0, pattern_width / 2, pattern_height / 2,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_SCALED_RESOLVE_NICEST_EXT);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	/* Do multi-sample to single-sample scaled blit */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampled_fbo_1.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, singlesampled_fbo.handle);

	glBlitFramebuffer(0, 0, pattern_width / 2, pattern_height / 2,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_SCALED_RESOLVE_FASTEST_EXT);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBlitFramebuffer(0, 0, pattern_width / 2, pattern_height / 2,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_SCALED_RESOLVE_NICEST_EXT);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	Config.color_internalformat = GL_RGBA8I;
	multisampled_fbo_1.setup(Config);
	singlesampled_fbo.setup(Config);

	/* Do multi-sample integer buffer to single-sample scaled blit */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampled_fbo_1.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, singlesampled_fbo.handle);

	glBlitFramebuffer(0, 0, pattern_width / 2, pattern_height / 2,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_SCALED_RESOLVE_FASTEST_EXT);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	glBlitFramebuffer(0, 0, pattern_width / 2, pattern_height / 2,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_SCALED_RESOLVE_NICEST_EXT);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display()
{
        /* UNREACHED */
	return PIGLIT_FAIL;
}
