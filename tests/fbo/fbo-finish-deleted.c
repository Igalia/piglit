/*
 * Copyright © 2011 Intel Corporation
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

/** @file fbo-finish-deleted.c
 *
 * Tests that glFinish() on an FBO with recently deleted renderbuffers
 * doesn't segfault.
 *
 * https://bugs.freedesktop.org/show_bug.cgi?id=34656
 */

#include "piglit-util-gl.h"

#define BUF_WIDTH 32
#define BUF_HEIGHT 32

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

void piglit_init(int argc, char **argv)
{
	GLuint tex, fb;
	GLenum status;
	float green[4] = {0.0, 1.0, 0.0, 0.0};

	piglit_require_extension("GL_EXT_framebuffer_object");

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		     BUF_WIDTH, BUF_HEIGHT, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	assert(glGetError() == 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "framebuffer incomplete (status = 0x%04x)\n",
			status);
		piglit_report_result(PIGLIT_SKIP);
	}

	/* Draw something to get the driver's state all set up
	 * pointing at our buffer.
	 */
	glColor4f(0.0, 1.0, 0.0, 0.0);
	piglit_draw_rect(-1, -1, 2, 2);

	piglit_probe_rect_rgba(0, 0, BUF_WIDTH, BUF_HEIGHT, green);

	/* This glFinish() should work. */
	glFinish();

	glDeleteTextures(1, &tex);

	/* This is the one that crashed. */
	glFinish();

	glDeleteFramebuffersEXT(1, &fb);

	piglit_report_result(PIGLIT_PASS);

}
