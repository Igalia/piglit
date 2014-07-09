/*
 * Copyright © 2012 Intel Corporation
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

/** @file readpixels-no-color-buffer.c
 *
 * Test that glReadPixels(GL_RGBA) when there is no corresponding
 * attachment bound correctly returns GL_INVALID_OPERATION.
 */

#include <stdio.h>
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display()
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static bool
test_bad_readpixels(GLenum format, GLenum type)
{
	uint32_t junk[4];
	glReadPixels(0, 0, 1, 1, format, type, junk);
	return piglit_check_gl_error(GL_INVALID_OPERATION);
}

void piglit_init(int argc, char **argv)
{
	GLuint fb, rb;
	bool pass = true;

	piglit_require_extension("GL_ARB_framebuffer_object");

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1, 1);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				  GL_RENDERBUFFER, rb);
	pass = test_bad_readpixels(GL_RGBA, GL_FLOAT) && pass;
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				  GL_RENDERBUFFER, 0);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, 1, 1);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb);
	pass = test_bad_readpixels(GL_DEPTH_COMPONENT, GL_FLOAT) && pass;
	pass = test_bad_readpixels(GL_STENCIL_INDEX, GL_FLOAT) && pass;
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, 0);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
