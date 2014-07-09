/*
 * Copyright © Marek Olšák <maraeo@gmail.com>
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
piglit_init(int argc, char *argv[])
{
	GLuint b1, b2;
	uintptr_t ptr;
	GLint alignment = 0;

	piglit_require_gl_version(15);

	piglit_require_extension("GL_ARB_map_buffer_range");
	piglit_require_extension("GL_ARB_map_buffer_alignment");

	glGetIntegerv(GL_MIN_MAP_BUFFER_ALIGNMENT, &alignment);

	/* Sanity check. */
	if (alignment < 64) {
		printf("GL_MIN_MAP_BUFFER_ALIGNMENT must be at least 64.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glGenBuffers(1, &b1);
	glBindBuffer(GL_ARRAY_BUFFER, b1);
	glBufferData(GL_ARRAY_BUFFER, alignment + 24, NULL, GL_STATIC_DRAW);

	glGenBuffers(1, &b2);
	glBindBuffer(GL_ARRAY_BUFFER, b2);
	glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);

	/* glMapBufferRange, offset > 0 */
	glBindBuffer(GL_ARRAY_BUFFER, b1);
	ptr = (intptr_t)glMapBufferRange(GL_ARRAY_BUFFER, 24, alignment,
					 GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

	if (ptr && (ptr - 24) % alignment != 0) {
		printf("glMapBufferRange returned an unaligned pointer.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* glMapBuffer. */
	glBindBuffer(GL_ARRAY_BUFFER, b2);
	ptr = (intptr_t)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

	if (ptr % alignment != 0) {
		printf("glMapBuffer returned an unaligned pointer.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}
