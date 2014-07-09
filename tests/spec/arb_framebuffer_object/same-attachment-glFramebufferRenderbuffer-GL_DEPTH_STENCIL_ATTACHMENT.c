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

/**
 * Attach a renderbuffer to the GL_DEPTH_STENCIL_ATTACHMENT point, then verify
 * with glGetFramebufferAttachmentParameteriv() that all three of
 * GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT, and GL_DEPTH_STENCIL_ATTACHMENT
 * point to the renderbuffer.
 */

#include <stdio.h>
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 200;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

const char*
get_attachment_string(GLint attach)
{
	switch (attach) {
	case GL_DEPTH_ATTACHMENT: return "GL_DEPTH_ATTACHMENT";
	case GL_STENCIL_ATTACHMENT: return "GL_STENCIL_ATTACHMENT";
	case GL_DEPTH_STENCIL_ATTACHMENT: return "GL_DEPTH_STENICL";
	default: return NULL;
	}
}

bool
check_attachment(GLenum attach, GLint expect_name)
{
	GLint actual_type;
	GLint actual_name;

	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
					      attach,
				              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
				              &actual_type);

	if (actual_type != GL_RENDERBUFFER) {
		char actual_type_str[16];

		if (actual_type == GL_NONE) {
			sprintf(actual_type_str, "GL_NONE");
		} else {
			snprintf(actual_type_str, 16, "0x%x", actual_type);
		}

		fprintf(stderr,
			"error: expected GL_RENDERBUFFER for %s attachment type, but found %s\n",
			get_attachment_string(attach), actual_type_str);

		/* Return now and don't query the attachment name, because
		 * that would generate a GL error.
		 */
		return false;
	}

	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
					      attach,
				              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
				              &actual_name);

	if (actual_name != expect_name) {
		fprintf(stderr,
			"error: expected %d for %s attachment name, but found %d\n",
			expect_name, get_attachment_string(attach), actual_name);
		return false;
	}

	return true;
}

enum piglit_result
piglit_display()
{
	/* Never reach here. */
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	bool pass = true;

	GLuint fb;
	GLuint rb;

	piglit_require_extension("GL_ARB_framebuffer_object");

	glGenRenderbuffers(1, &rb);
	glGenFramebuffers(1, &fb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, 200, 200);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,
			         GL_DEPTH_STENCIL_ATTACHMENT,
			         GL_RENDERBUFFER,
			         rb);

	pass = piglit_check_gl_error(0) && pass;

	pass = check_attachment(GL_DEPTH_ATTACHMENT, rb) && pass;
	pass = check_attachment(GL_STENCIL_ATTACHMENT, rb) && pass;
	pass = check_attachment(GL_DEPTH_STENCIL_ATTACHMENT, rb) && pass;

	pass = piglit_check_gl_error(0) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
