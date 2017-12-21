/*
 * Copyright (C) 2007  Intel Corporation
 * Copyright (C) 1999  Allen Akin   All Rights Reserved.
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

/** @file error-handling.c
 *
 * Authors:  Shuang He <shuang.he@intel.com>
 *
 * Test that glCheckFramebufferStatus reports various incomplete statuses
 * required by the spec.
 */

#include "piglit-util-gl.h"

#define TEXSIZE 64

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 11;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");

	GLuint fbo;
	GLuint textures[2];
	GLuint renderbuffer;
	GLenum status;
	GLuint max_color_attachments;
	const bool have_ARB_ES2 =
		piglit_is_extension_supported("GL_ARB_ES2_compatibility");
	const bool have_ARB_fbo =
		piglit_is_extension_supported("GL_ARB_framebuffer_object");

	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT,
		      (GLint *)&max_color_attachments);
	if (max_color_attachments < 1) {
		printf("Failed to get max color attachment points");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* At least one image attached to the framebuffer */
	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDeleteFramebuffersEXT(1, &fbo);
	if (status !=
	    GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT) {
		printf("If no image is attached to framebuffer, status "
		       "should be "
		       "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* All attached images have the same width and height,
	 * unless GL_ARB_framebuffer object is supported.
	 */
	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glGenTextures(2, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXSIZE,
		     TEXSIZE, 0, GL_RGB, GL_INT, NULL);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D, textures[0], 0);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXSIZE / 2,
		     TEXSIZE / 2, 0, GL_RGB, GL_INT, NULL);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT
				  + max_color_attachments - 1,
				  GL_TEXTURE_2D, textures[1], 0);
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDeleteFramebuffersEXT(1, &fbo);
	glDeleteTextures(2, textures);
	if (!have_ARB_fbo &&
	    status != GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT) {
		printf("If renderbuffer sizes don't all match, status should "
		       "be GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* All images attached to the attachment points
	 * COLOR_ATTACHMENT0_EXT through COLOR_ATTACHMENTn_EXT must
	 * have the same internal format, unless ARB_fbo is supported.
	 */
	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glGenTextures(2, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXSIZE,
		     TEXSIZE, 0, GL_RGB, GL_INT, NULL);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D, textures[0], 0);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXSIZE,
		     TEXSIZE, 0, GL_RGBA, GL_INT, NULL);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT
				  + max_color_attachments - 1,
				  GL_TEXTURE_2D, textures[1], 0);
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDeleteFramebuffersEXT(1, &fbo);
	glDeleteTextures(2, textures);
	if (!have_ARB_fbo &&
	    status != GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT) {
		printf("All color renderbuffers must be of same format, "
		       "status should be "
		       "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT");
		piglit_report_result(PIGLIT_FAIL);
	}


	/* The value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT must not
	 * be NONE for any color attachment point(s) named by
	 * DRAW_BUFFERi.
	 * [Note: to avoid being caught by the no-attachments
	 * case above, we attach a depth renderbuffer.]
	 */
	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT +
		     max_color_attachments - 1);
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER_EXT, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24,
			      TEXSIZE, TEXSIZE);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT,
				  GL_DEPTH_ATTACHMENT_EXT,
				  GL_RENDERBUFFER_EXT,
				  renderbuffer);
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDeleteFramebuffersEXT(1, &fbo);
	glDeleteTextures(1, textures);
	glDeleteRenderbuffers(1, &renderbuffer);
	if (status != GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT &&
	    !have_ARB_ES2) {
		printf("All any buffer named by glDrawBuffers is missing, "
		       "status should be "
		       "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* If READ_BUFFER is not NONE, then the value of
	 * FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT must not be NONE for
	 * the color attachment point named by READ_BUFFER.
	 * [Note: to avoid being caught by the no-attachments
	 * case above, we attach a depth renderbuffer.]
	 */
	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT +
		     max_color_attachments - 1);
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER_EXT, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24,
			      TEXSIZE, TEXSIZE);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT,
				  GL_DEPTH_ATTACHMENT_EXT,
				  GL_RENDERBUFFER_EXT,
				  renderbuffer);
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDeleteFramebuffersEXT(1, &fbo);
	glDeleteRenderbuffers(1, &renderbuffer);
	if (status != GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT &&
	    !have_ARB_ES2) {
		printf("If buffer named by glReadBuffers is missing, status "
		       "should be GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}
