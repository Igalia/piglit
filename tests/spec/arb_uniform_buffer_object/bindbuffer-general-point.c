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

/** @file bindbuffer-general-point.c
 *
 * Tests that the glBindBuffer* entrypoints also bind to the general
 * binding point.
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
	GLuint bo[2];
	GLint binding;

	piglit_require_extension("GL_ARB_uniform_buffer_object");

	glGenBuffers(2, bo);
	glBindBuffer(GL_UNIFORM_BUFFER, bo[0]);
	glBufferData(GL_UNIFORM_BUFFER, 1, NULL, GL_STATIC_READ);
	glBindBuffer(GL_UNIFORM_BUFFER, bo[1]);
	glBufferData(GL_UNIFORM_BUFFER, 1, NULL, GL_STATIC_READ);

	/* From the GL_ARB_uniform_buffer_object spec:
	 *
	 *     "Buffer objects are bound to uniform block binding
	 *     points by calling one of the commands
	 *
	 *     void BindBufferRange(...)
	 *     void BindBufferBase(...)
	 *
	 *      There is an array of buffer object binding points with
	 *      which uniform blocks can be associated via
	 *      UniformBlockBinding, plus a single general binding
	 *      point that can be used by other buffer object
	 *      manipulation functions (e.g. BindBuffer,
	 *      MapBuffer). Both commands bind the buffer object named
	 *      by <buffer> to the general binding point, and
	 *      additionally bind the buffer object to the binding
	 *      point in the array given by <index>."
	 */
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, bo[0]);
	glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &binding);
	if (binding != bo[0]) {
		fprintf(stderr, "GL_UNIFORM_BUFFER_BINDING[0] was %d, expected %d\n",
			binding, bo[0]);
		pass = false;
	}

	glBindBufferRange(GL_UNIFORM_BUFFER, 1, bo[1], 0, 1);
	glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &binding);
	if (binding != bo[1]) {
		fprintf(stderr, "GL_UNIFORM_BUFFER_BINDING[1] was %d, expected %d\n",
			binding, bo[1]);
		pass = false;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

