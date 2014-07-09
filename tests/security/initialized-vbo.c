/*
 * Copyright © 2012 VMware, Inc.
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
 * Test that VBO memory is initialized to a constant value and not stale
 * data that may show previous contents of VRAM.
 *
 * To pass this test an OpenGL implementation should initialize the
 * contents of the new buffer to some fixed value (like all zeros).
 * But since that's not spec'd by OpenGL, we only return WARN instead of
 * FAIL if that's not the case.
 *
 * Brian Paul
 * June 2012
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END


enum piglit_result
piglit_display(void)
{
	GLsizei size = 10 * 1000 * 1000;
	GLuint buf;
	GLubyte *tmp = malloc(size);
	int i;
	bool pass = true;

	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);

	glGetBufferSubData(GL_ARRAY_BUFFER, 0, size, tmp);

	/* check if all same values */
	for (i = 1; i < size; i++) {
		if (tmp[i] != tmp[0]) {
			pass = false;
			break;
		}
	}

	glDeleteBuffers(1, &buf);

	return pass ? PIGLIT_PASS : PIGLIT_WARN;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_vertex_buffer_object");
}
