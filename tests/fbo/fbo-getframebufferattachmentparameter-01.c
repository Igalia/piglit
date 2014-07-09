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
 * \file fbo-getframebufferattachmentparameter-01.c
 * Query attachment type and name parameters.
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

GLboolean
try_GetAttachmentParam(GLenum attachment, GLenum pname, GLint expected,
		       GLenum expected_err, const char *fmt)
{
	GLint value = ~expected;
	GLenum err;

	/* Clear any previous GL error state.
	 */
	while (glGetError() != 0)
		/* empty */ ;

	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER_EXT, attachment,
					      pname, &value);
	err = glGetError();
	if (err != expected_err) {
		printf("Unexpected GL error state 0x%04x querying "
		       "attachment=0x%04x, pname=0x%04x.  Expected 0x%04x.\n",
		       err, attachment, pname, expected_err);
		return GL_FALSE;
	}

	/* Only check the return value if the command was expected to succeed.
	 */
	if ((expected_err == 0) && (value != expected)) {
		printf(fmt, expected, value);
		return GL_FALSE;
	}

	return GL_TRUE;
}

void
piglit_init(int argc, char **argv)
{
	GLboolean pass;
	GLuint tex;
	GLuint fb;
	GLenum status;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_extension("GL_ARB_framebuffer_object");


	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 32, 32, 0,
		     GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_TEXTURE_2D, tex, 0);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FBO incomplete because 0x%04x\n", status);
		piglit_report_result(PIGLIT_FAIL);
	}

	pass = try_GetAttachmentParam(GL_COLOR_ATTACHMENT0,
				      GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
				      GL_TEXTURE,
				      0,
				      "Expected type of color attachment 0 to "
				      "be 0x%04x, got 0x%04x instead.\n");
	pass = try_GetAttachmentParam(GL_COLOR_ATTACHMENT0,
				      GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
				      tex,
				      0,
				      "Expected name of color attachment 0 to "
				      "be %d, got %d instead.\n")
		&& pass;
	pass = try_GetAttachmentParam(GL_DEPTH_ATTACHMENT,
				      GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE,
				      0,
				      GL_INVALID_OPERATION,
				      "")
		&& pass;
	pass = try_GetAttachmentParam(GL_DEPTH_ATTACHMENT,
				      GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
				      GL_NONE,
				      0,
				      "Expected type of depth attachment to be "
				      "0x%04x, got 0x%04x instead.\n")
		&& pass;
	pass = try_GetAttachmentParam(GL_DEPTH_ATTACHMENT,
				      GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
				      0,
				      0,
				      "Expected name of depth attachment to be "
				      "%d, got %d instead.\n")
		&& pass;
	pass = try_GetAttachmentParam(GL_DEPTH_ATTACHMENT,
				      GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE,
				      0,
				      GL_INVALID_OPERATION,
				      "")
		&& pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
