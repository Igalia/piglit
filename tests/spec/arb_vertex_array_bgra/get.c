/*
 * Copyright 2014 VMware, Inc
 * Copyright 2013 Intel Corporation
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

/*
 * This tests that glGet*(GL_*_ARRAY_SIZE) returns GL_BGRA.
 *
 * Tools like ApiTrace rely on this to work correctly.  See for example
 * https://github.com/apitrace/apitrace/issues/261 .
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	static const GLubyte ubytes[4] = { 255, 0, 0, 127 };
	bool pass = true;
	GLint size;

	piglit_require_gl_version(20);
	piglit_require_extension("GL_ARB_vertex_array_bgra");

	glColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof ubytes, ubytes);
	size = 0;
	glGetIntegerv(GL_COLOR_ARRAY_SIZE, &size);
	if (size != GL_BGRA) {
		fprintf(stderr, "glGetIntegerv(GL_COLOR_ARRAY_SIZE) returned %i, GL_BGRA expected\n", size);
		pass = false;
	}

	glSecondaryColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof ubytes, ubytes);
	size = 0;
	glGetIntegerv(GL_SECONDARY_COLOR_ARRAY_SIZE, &size);
	if (size != GL_BGRA) {
		fprintf(stderr, "glGetIntegerv(GL_SECONDARY_COLOR_ARRAY_SIZE) returned %i, GL_BGRA expected\n", size);
		pass = false;
	}

	glVertexAttribPointer(1, GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE, sizeof ubytes, ubytes);
	size = 0;
	glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
	if (size != GL_BGRA) {
		fprintf(stderr, "glGetVertexAttribiv(GL_VERTEX_ATTRIB_ARRAY_SIZE) returned %i, GL_BGRA expected\n", size);
		pass = false;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* Should never get here. */
	return PIGLIT_FAIL;
}
