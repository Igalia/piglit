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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file negative-bindbuffer-index.c
 *
 * Tests for errors when binding higher than the maximum uniform
 * buffer binding point.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLint max_bindings;
	GLuint bo;

	piglit_require_extension("GL_ARB_uniform_buffer_object");

	glGenBuffers(1, &bo);
	glBindBuffer(GL_UNIFORM_BUFFER, bo);
	glBufferData(GL_UNIFORM_BUFFER, 1, NULL, GL_STATIC_READ);

	glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &max_bindings);

	/* From the GL_ARB_uniform_buffer_object spec:
	 *
	 *     "The error INVALID_VALUE is generated if <index> is
	 *      greater than or equal to the value of
	 *      MAX_UNIFORM_BUFFER_BINDINGS.
	 */

	glBindBufferBase(GL_UNIFORM_BUFFER, max_bindings, bo);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	glBindBufferRange(GL_UNIFORM_BUFFER, max_bindings, bo, 0, 1);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
