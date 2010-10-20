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

/**
 * \file fbo-incomplete-texture-01.c
 * Verify that an FBO with an incomplete texture attached in itself incomplete
 *
 * This test uses a 2D texture that specifies a mipmap filter, but the mipmap
 * stack is not complete.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util.h"

int piglit_width = 30;
int piglit_height = 30;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAILURE;
}

void
piglit_init(int argc, char **argv)
{
	GLuint tex;
	GLuint fb;
	GLenum status;
	GLenum err;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_extension("GL_ARB_framebuffer_object");

	/* This texture will be incomplete because a mipmap filter mode is used,
	 * but the mipmap stack is incomplete.
	 */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 32, 32, 0,
		     GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, tex, 0);

	err = glGetError();
	if (err != 0) {
		printf("Unexpected GL error state 0x%04x\n", err);
		piglit_report_result(PIGLIT_FAILURE);
	}

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_COMPLETE) {
		printf("FBO erroneously complete\n");
		piglit_report_result(PIGLIT_FAILURE);
	} else if (status != GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
		fprintf(stderr, "FBO incomplete for incorrect reason 0x%04x "
			"(should be 0x%04x)\n",
			GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT, status);
	}

	piglit_report_result(PIGLIT_SUCCESS);
}
