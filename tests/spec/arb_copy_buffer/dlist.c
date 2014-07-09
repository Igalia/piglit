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

/** @file dlist.c
 *
 * Tests the following piece of the GL_ARB_copy_buffer spec:
 *
 *     "Add to the list (page 310) of "Vertex Buffer Objects" commands "not
 *      compiled into the display list but are executed immediately":
 *
 *          CopyBufferSubData"
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
	GLuint bufs[2], list;
	uint8_t data[8];
	uint8_t bad_data[8];
	int i;
	void *ptr;

	piglit_require_extension("GL_ARB_copy_buffer");

	glGenBuffers(2, bufs);

	for (i = 0; i < ARRAY_SIZE(data); i++)
		data[i] = i;

	memset(bad_data, 0xd0, sizeof(bad_data));

	glBindBuffer(GL_COPY_READ_BUFFER, bufs[0]);
	glBufferData(GL_COPY_READ_BUFFER, sizeof(data), data,
		     GL_DYNAMIC_DRAW);
	glBindBuffer(GL_COPY_WRITE_BUFFER, bufs[1]);
	glBufferData(GL_COPY_WRITE_BUFFER, sizeof(bad_data), bad_data,
		     GL_DYNAMIC_DRAW);

	list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
			    0, 0, sizeof(data));
	glEndList();

	/* Make sure that it immediately executed. */
	ptr = glMapBuffer(GL_COPY_WRITE_BUFFER, GL_READ_ONLY);
	if (memcmp(ptr, data, sizeof(data))) {
		fprintf(stderr,
			"data not copied during display list compile\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	glUnmapBuffer(GL_COPY_WRITE_BUFFER);

	/* Now, make sure that it isn't in the list. */
	glBufferData(GL_COPY_WRITE_BUFFER, sizeof(bad_data), bad_data,
		     GL_DYNAMIC_DRAW);
	glCallList(list);

	ptr = glMapBuffer(GL_COPY_WRITE_BUFFER, GL_READ_ONLY);
	if (memcmp(ptr, bad_data, sizeof(bad_data))) {
		fprintf(stderr,
			"data copied during display list execute\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	glUnmapBuffer(GL_COPY_WRITE_BUFFER);

	glDeleteBuffers(2, bufs);

	piglit_report_result(PIGLIT_PASS);
}
