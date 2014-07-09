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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Kristian Høgsberg <krh@bitplanet.net>
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	GLint red_bits, att_type;
	GLenum err;

	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
					      GL_BACK_LEFT,
					      GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
					      &att_type);

	if (att_type != GL_FRAMEBUFFER_DEFAULT) {
		printf("Default framebuffer's attachment type is not GL_FRAMEBUFFER_DEFAULT\n");
		while (glGetError());
		return PIGLIT_FAIL;
	}

	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
					      GL_BACK_LEFT,
					      GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE,
					      &red_bits);

	printf("Red bits: %d\n", red_bits);

	err = glGetError();
	if (err) {
		printf("glGetFramebufferAttachmentParameteriv GL error: 0x%x\n", err);
		return PIGLIT_FAIL;
	}

       	return PIGLIT_PASS;
}

void piglit_init(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	piglit_require_extension("GL_ARB_framebuffer_object");

	piglit_ortho_projection(1.0, 1.0, GL_FALSE);

	piglit_automatic = GL_TRUE;
}
