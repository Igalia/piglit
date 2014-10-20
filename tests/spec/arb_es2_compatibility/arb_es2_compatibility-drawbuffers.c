/*
 * Copyright © 2010 Intel Corporation
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

/** @file arb_es2_compatibility-drawbuffers.c
 *
 * Tests that pointing DrawBuffers or ReadBuffers at unattached
 * attachment points doesn't result in INCOMPLETE_DRAW_BUFFER or
 * INCOMPLETE_READ_BUFFER with ARB_ES2_compatibility.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 128;
	config.window_height = 128;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint
attach_texture(int i)
{
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		     piglit_width, piglit_height, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT + i,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return tex;
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint tex0, fb;
	GLenum status;

	piglit_require_extension("GL_ARB_ES2_compatibility");

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	tex0 = attach_texture(0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "fbo incomplete (status = 0x%04x)\n", status);
		piglit_report_result(PIGLIT_SKIP);
	}

	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER) {
		fprintf(stderr, "fbo incomplete draw buffer\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glReadBuffer(GL_COLOR_ATTACHMENT1);
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER) {
		fprintf(stderr, "fbo incomplete read buffer\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glDeleteTextures(1, &tex0);

	piglit_report_result(PIGLIT_PASS);
}
