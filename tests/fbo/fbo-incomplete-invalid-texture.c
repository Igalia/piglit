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

/**
 * \file fbo-incomplete-invalid-texture.c
 * Try reproducing a segfault in Mesa by attaching a "broken" texture to an
 * FBO, the unbind and rebind the FBO.
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
	bool pass = true;
	GLuint tex;
	GLuint fbo;
	GLenum status;

	piglit_require_extension("GL_ARB_framebuffer_object");


	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	/* The format of the pixel data is invalide for the specified
	 * internalFormat.  This should fail and generate
	 * GL_INVALID_OPERATION.  This leaves the texture in a weird, broken
	 * state.
	 */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 4, 4, 0,
		     GL_RGBA, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
		pass = false;
		goto cleanup;
	}

	/* Attach the broken texture to the FBO.
	 */
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, tex, 0);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		pass = false;
		goto cleanup;
	}

	/* Unbind and rebind the FBO.  At one point on Mesa this triggered a
	 * segfault down inside the glBindFramebuffer code.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
		fprintf(stderr,
			"status was %s (0x%04x), expected %s (0x%04x).\n",
			piglit_get_gl_enum_name(status),
			status,
			"GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT",
			GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
		pass = false;
		goto cleanup;
	}

cleanup:
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);

	glDeleteTextures(1, &tex);
	glDeleteFramebuffers(1, &fbo);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
