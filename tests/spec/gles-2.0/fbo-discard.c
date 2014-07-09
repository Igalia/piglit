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
 *
 * Authors:
 *    Tapani Pälli <tapani.palli@intel.com>
 */

/** @file fbo-discard.c
 *
 * Tests GL_EXT_discard_framebuffer implementation
 *
 * Test iterates over valid and invalid arguments and checks
 * that the implementation returns correct error codes.
 *
 * GL_EXT_discard_framebuffer specification "Errors" section states:
 *
 *  "The error INVALID_ENUM is generated if DiscardFramebufferEXT is called
 *   with a <target> that is not FRAMEBUFFER.
 *
 *   The error INVALID_ENUM is generated if DiscardFramebufferEXT is called with
 *   a token other than COLOR_ATTACHMENT0, DEPTH_ATTACHMENT, or
 *   STENCIL_ATTACHMENT in its <attachments> list when a framebuffer object is
 *   bound to <target>.
 *
 *   The error INVALID_ENUM is generated if DiscardFramebufferEXT is called with
 *   a token other than COLOR_EXT, DEPTH_EXT, or STENCIL_EXT in its
 *   <attachments> list when the default framebuffer is bound to <target>.
 *
 *   The error INVALID_VALUE is generated if DiscardFramebufferEXT is called
 *   with <numAttachments> less than zero."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;

PIGLIT_GL_TEST_CONFIG_END

static GLboolean
run_test(void)
{
	bool pass = true;
	GLenum *p = NULL;
	GLuint fbo;

	/* valid enums for user created fb */
	GLenum usr_attach[] = {
		GL_COLOR_ATTACHMENT0,
		GL_DEPTH_ATTACHMENT,
		GL_STENCIL_ATTACHMENT,
		0
	};

	/* valid enums for default fb */
	GLenum def_attach[] = {
		GL_COLOR_EXT,
		GL_DEPTH_EXT,
		GL_STENCIL_EXT,
		0
	};

	/* bonus invalid enum */
	const GLenum invalid[] = { GL_COMPILE_STATUS };

	glGenFramebuffers(1, &fbo);

	/* test with invalid target */
	glDiscardFramebufferEXT(GL_RENDERBUFFER, 1, usr_attach);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	/* test with attachments < 0 */
	glDiscardFramebufferEXT(GL_FRAMEBUFFER, -1, usr_attach);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	/* test with default fb */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDiscardFramebufferEXT(GL_FRAMEBUFFER, 3, def_attach);
	for (p = def_attach; *p != 0; p++) {
		glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, p);
		pass &= piglit_check_gl_error(GL_NO_ERROR);
	}

	glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, invalid);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	for (p = usr_attach; *p != 0; p++) {
		glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, p);
		pass &= piglit_check_gl_error(GL_INVALID_ENUM);
	}

	/* test user created fb */
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glDiscardFramebufferEXT(GL_FRAMEBUFFER, 3, usr_attach);
	for (p = usr_attach; *p != 0; p++) {
		glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, p);
		pass &= piglit_check_gl_error(GL_NO_ERROR);
	}

	glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, invalid);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	for (p = def_attach; *p != 0; p++) {
		glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, p);
		pass &= piglit_check_gl_error(GL_INVALID_ENUM);
	}

	return pass;
}


enum piglit_result
piglit_display(void)
{
	GLboolean pass = run_test();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_discard_framebuffer");
}
