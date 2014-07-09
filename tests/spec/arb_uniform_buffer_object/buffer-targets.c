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

/** @file buffer-targets.c
 *
 * Tests that the GL_UNIFORM_BUFFER target is accepted by other gl
 * entrypoints.
 *
 * From the GL_ARB_uniform_buffer_object spec:
 *
 *     "Accepted by the <target> parameters of BindBuffer, BufferData,
 *      BufferSubData, MapBuffer, UnmapBuffer, GetBufferSubData, and
 *      GetBufferPointerv:
 *
 *          UNIFORM_BUFFER"
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
	GLuint bo;
	uint8_t in_data[1] = {0xaa};
	uint8_t out_data[1] = {0xd0};
	void *ptr1, *ptr2;

	piglit_require_extension("GL_ARB_uniform_buffer_object");

	glGenBuffers(1, &bo);

	glBindBuffer(GL_UNIFORM_BUFFER, bo);
	pass = pass && piglit_check_gl_error(0);

	glBufferData(GL_UNIFORM_BUFFER, 1, NULL, GL_STATIC_READ);
	pass = pass && piglit_check_gl_error(0);

	glBufferSubData(GL_UNIFORM_BUFFER, 0, 1, in_data);
	pass = pass && piglit_check_gl_error(0);

	ptr1 = glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_ONLY);
	pass = pass && piglit_check_gl_error(0);

	glGetBufferPointerv(GL_UNIFORM_BUFFER, GL_BUFFER_MAP_POINTER, &ptr2);
	pass = pass && piglit_check_gl_error(0);
	assert(ptr1 == ptr2);

	glUnmapBuffer(GL_UNIFORM_BUFFER);
	pass = pass && piglit_check_gl_error(0);

	glGetBufferSubData(GL_UNIFORM_BUFFER, 0, 1, out_data);
	pass = pass && piglit_check_gl_error(0);
	assert(memcmp(in_data, out_data, sizeof(in_data)) == 0);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

