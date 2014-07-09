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

/** @file get.c
 *
 * Tests that the getters for the new binding points in GL_ARB_copy_buffer work.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* uncreached */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint bufs[2];
	GLint get_bufs[2];
	uint8_t data[8] = { 0 };

	memset(get_bufs, 0xd0, sizeof(get_bufs));

	piglit_require_extension("GL_ARB_copy_buffer");

	glGetIntegerv(GL_COPY_READ_BUFFER, &get_bufs[0]);
	glGetIntegerv(GL_COPY_WRITE_BUFFER, &get_bufs[1]);

	if (get_bufs[0] != 0 || get_bufs[1] != 0) {
		fprintf(stderr, "Initial copy read/write bindings %d/%d, "
			"should be 0/0.\n",
			get_bufs[0], get_bufs[1]);
		piglit_report_result(PIGLIT_FAIL);
	}

	glGenBuffers(2, bufs);
	glBindBuffer(GL_COPY_READ_BUFFER, bufs[0]);
	glBufferData(GL_COPY_READ_BUFFER, sizeof(data), data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_COPY_WRITE_BUFFER, bufs[1]);
	glBufferData(GL_COPY_WRITE_BUFFER, sizeof(data), data, GL_DYNAMIC_DRAW);

	glGetIntegerv(GL_COPY_READ_BUFFER, &get_bufs[0]);
	glGetIntegerv(GL_COPY_WRITE_BUFFER, &get_bufs[1]);

	if (bufs[0] != get_bufs[0]) {
		fprintf(stderr, "GL_COPY_READ_BUFFER should return %d, returned %d\n",
			bufs[0], get_bufs[0]);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (bufs[1] != get_bufs[1]) {
		fprintf(stderr, "GL_COPY_WRITE_BUFFER should return %d, returned %d\n",
			bufs[1], get_bufs[1]);
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}
