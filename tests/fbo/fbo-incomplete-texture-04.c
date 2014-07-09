/*
 * Copyright © 2010, 2011 Intel Corporation
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
 * \file fbo-incomplete-texture-04.c
 * Verify that using a nonexistent texture level is not an error
 *
 * This is based on a discussion at the January 2011 Khronos face-to-face
 * meeting.  The intention is that calling \c glFramebufferTexture with a
 * level that does not exist in the texture should not generate an error.
 * However, the FBO will be incomplete until the texture level is created.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint tex;
	GLuint fb;
	GLenum status;
	const float color[] = {1.0,0.0,0.0,1.0};

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_extension("GL_ARB_framebuffer_object");

	/* This texture will be incomplete because a mipmap filter mode is used,
	 * but the mipmap stack is incomplete.
	 */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, tex, 2);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Since texture level 2 does not exist, the FBO should be incomplete.
	 */
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_COMPLETE) {
		printf("FBO erroneously complete\n");
		piglit_report_result(PIGLIT_FAIL);
	} else if (status != GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
		fprintf(stderr, "FBO incomplete for incorrect reason 0x%04x "
			"(should be 0x%04x)\n",
			GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT, status);
	}

	/* Create level 1 and level 2 of the texture.  Verify that the FBO
	 * becomes complete as a result.
	 */
	glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 16, 16, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 8, 8, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FBO erroneously incomplete\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glClearColor(color[0], color[1], color[2], color[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	if (!piglit_probe_texel_rect_rgba(GL_TEXTURE_2D, 2, 0, 0, 8, 8,
					  color)) {
		fprintf(stderr, "FBO clear didn't work\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}
