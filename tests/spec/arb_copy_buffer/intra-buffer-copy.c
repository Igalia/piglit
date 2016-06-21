/*
 * Copyright 2016 VMware, Inc.
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
 * Simple test of copying within a single buffer.
 *
 * Brian Paul
 * 21 June 2016
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 15;
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


void
piglit_init(int argc, char *argv[])
{
	bool pass = true;
	GLuint buffer;
	GLubyte data[250], *map;
	int i;

	piglit_require_extension("GL_ARB_copy_buffer");

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_COPY_READ_BUFFER, buffer);
	glBindBuffer(GL_COPY_WRITE_BUFFER, buffer);

	/* create empty buffer */
	glBufferData(GL_COPY_READ_BUFFER, 1000, NULL, GL_STREAM_COPY);

	/* fill last 250 of 1000 bytes */
	for (i = 0; i < 250; i++)
		data[i] = i;
	glBufferSubData(GL_COPY_READ_BUFFER, 750, 250, data);

	/* copy last 250 bytes to offset 500 */
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
			    750, 500, 250);

	/* copy last 500 bytes to start of buffer */
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
			    500, 0, 500);

	/* check results */
	map = glMapBuffer(GL_COPY_READ_BUFFER, GL_READ_ONLY);
	assert(map);

	for (i = 0; i < 1000; i++) {
		if (map[i] != i % 250) {
			printf("Wrong buffer value at position %u.\n", i);
			printf("Expected %u, found %u\n", i % 250, map[i]);
			pass = false;
			break;
		}
	}

	glUnmapBuffer(GL_COPY_READ_BUFFER);

	glDeleteBuffers(1, &buffer);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* unreachable */
	return PIGLIT_FAIL;
}
