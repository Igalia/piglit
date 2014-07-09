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
 * Test GL_ARB_copy_buffer
 *
 * Brian Paul
 * Jan 2012
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static bool
test_copy(void)
{
#define BUF_SIZE 600  /* multiple of 100 */

	GLuint bufs[2];
	char data[BUF_SIZE], *p;
	int i;
	int chunk = 100;
	bool pass = true;

	assert(BUF_SIZE % chunk == 0);

	glGenBuffers(2, bufs);

	for (i = 0; i < BUF_SIZE; i++)
		data[i] = rand() % 256;

	glBindBufferARB(GL_COPY_READ_BUFFER, bufs[0]);
	glBufferData(GL_COPY_READ_BUFFER, BUF_SIZE, data, GL_STATIC_DRAW);

	glBindBufferARB(GL_COPY_WRITE_BUFFER, bufs[1]);
	glBufferData(GL_COPY_WRITE_BUFFER, BUF_SIZE, NULL, GL_DYNAMIC_COPY);

	/* copy from bufs[0] to bufs[1] in chunks */
	for (i = 0; i < BUF_SIZE / chunk; i++) {
		int srcOffset = i * chunk;
		int dstOffset = i * chunk;
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				    srcOffset, dstOffset, chunk);
	}

	/* verify dest buffer */
	p = glMapBuffer(GL_COPY_WRITE_BUFFER, GL_READ_ONLY);
	for (i = 0; i < BUF_SIZE; i++) {
		if (p[i] != data[i]) {
			printf("expected %d, found %d at location %d\n",
			       data[i], p[i], i);
			pass = false;
			break;
		}
	}

	glUnmapBuffer(GL_COPY_WRITE_BUFFER);

	glDeleteBuffers(2, bufs);

	return pass;
}


enum piglit_result
piglit_display(void)
{
	/* should never get here */
	return PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_copy_buffer");

	if (test_copy())
		piglit_report_result(PIGLIT_PASS);
	else
		piglit_report_result(PIGLIT_FAIL);
}
