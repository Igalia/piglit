/*
 * Copyright (c) 2010 VMware, Inc.
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
 */

/**
 * @file geterror-inside-begin.c
 *
 * Verifies glGetError errors.
 *
 * "GL_INVALID_OPERATION is generated if glGetError is executed between the
 * execution of glBegin and the corresponding execution of glEnd. In this
 * case, glGetError returns 0."
 *
 * @author Vinson Lee <vlee@vmware.com>
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
	enum piglit_result result = PIGLIT_PASS;
	GLenum err;

	while (glGetError() != 0) {
		/* empty */
	}

	glBegin(GL_POINTS);
	err = glGetError();
	glEnd();

	if (err != 0) {
		printf("Unexpected OpenGL error state 0x%04x for glGetError() "
		       "inside glBegin/glEnd pair (expected 0x0000).\n",
		       err);
		result = PIGLIT_FAIL;
	}

	err = glGetError();

	if (err != GL_INVALID_OPERATION) {
		printf("Unexpected OpenGL error state 0x%04x for glGetError() "
		       "inside glBegin/glEnd pair (expected 0x%04x).\n",
		       err, GL_INVALID_OPERATION);
		result = PIGLIT_FAIL;
	}

	piglit_report_result(result);
}
