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
 * @file framebufferblit.c
 *
 * Tests interaction between GL_ARB_sampler_objects and
 * GL_EXT_framebuffer_blit.  There was a bug in mesa in which the
 * fbblit would accidentally apply an active sampler object from a
 * texture.
 *
 * To test this, ask for a nearest blit stretching from single pixel
 * to the window, and check if the LINEAR on the sampler object makes
 * the neighbors of that pixel get filtered in.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLuint tex, sampler, fb;
	const float tex_data[16] = {0, 1, 0, 0,
				    1, 0, 0, 0,
				    1, 0, 0, 0,
				    1, 0, 0, 0};
	const float *green = tex_data;

	glClearColor(0, 0, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0,
		     GL_RGBA, GL_FLOAT, tex_data);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindSampler(0, sampler);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, 1, 1,
			  0, 0, piglit_width, piglit_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green);
	piglit_present_results();

	glDeleteSamplers(1, &sampler);
	glDeleteTextures(1, &tex);
	glDeleteFramebuffers(1, &fb);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_sampler_objects");
	piglit_require_extension("GL_EXT_framebuffer_blit");
	piglit_require_extension("GL_EXT_texture_swizzle");

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
