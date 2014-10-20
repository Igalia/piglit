/*
 * Copyright © 2009 Intel Corporation
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
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file fbo-1d.c
 *
 * Tests that rendering to a 1D texture and then drawing it to the framebuffer
 * succeeds.
 */

#include "piglit-util-gl.h"

#define BUF_WIDTH 32

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

static int
create_1d_fbo(void)
{
	GLuint tex, fb;
	GLenum status;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_1D, tex);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA,
		     BUF_WIDTH,
		     0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_1D,
				  tex,
				  0);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "FBO incomplete\n");
		goto done;
	}

	glViewport(0, 0, BUF_WIDTH, 1);
	piglit_ortho_projection(BUF_WIDTH, 1, GL_FALSE);

	/* left side: red */
	glColor4f(1.0, 0.0, 0.0, 0.0);
	piglit_draw_rect(0, 0, BUF_WIDTH / 2, 1);

	/* right side: green */
	glColor4f(0.0, 1.0, 0.0, 0.0);
	piglit_draw_rect(BUF_WIDTH / 2, 0, BUF_WIDTH, 1);

done:
	glBindFramebufferEXT(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glDeleteFramebuffersEXT(1, &fb);

	return tex;
}

static void
draw_fbo_1d(int x, int y)
{
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	glEnable(GL_TEXTURE_1D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	piglit_draw_rect_tex(x, y, BUF_WIDTH, 1,
			     0, 0, 1, 1);
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float red[] = {1,0,0,0};
	float green[] = {0,1,0,0};
	float *expected;
	int x;
	GLuint tex;

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_1d_fbo();

	draw_fbo_1d(10, 10);

	for (x = 0; x < BUF_WIDTH; x++) {
		if (x < BUF_WIDTH / 2)
			expected = red;
		else
			expected = green;

		pass &= piglit_probe_pixel_rgb(10 + x, 10, expected);
	}

	glDeleteTextures(1, &tex);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
}
