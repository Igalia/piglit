/*
 * Copyright © 2015 Intel Corporation
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

/** @file getintegeri_v.c
 *
 * From the GL_ARB_shader_storage_buffer_object spec:
 *
 *   "To query the starting offset or size of the range of each buffer object
 *    binding used for shader storage buffers, call GetInteger64i_v with <param>
 *    set to SHADER_STORAGE_BUFFER_START or SHADER_STORAGE_BUFFER_SIZE
 *    respectively.  <index> must be in the range zero to the value of
 *    MAX_SHADER_STORAGE_BUFFER_BINDINGS-1.  If the parameter (starting offset
 *    or size) was not specified when the buffer object was bound (e.g.  if
 *    bound with BindBufferBase), or if no buffer object is bound to index, zero
 *    is returned."
 *
 * Based on ARB_uniform_buffer_object's getintegeri_v.c
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static bool pass = true;

static void
test_index(int line, GLenum e, int index, int expected)
{
	GLint val;

	glGetIntegeri_v(e, index, &val);
	if (val != expected) {
		printf("%s:%d: %s[%d] was %d, expected %d\n",
		       __FILE__, line, piglit_get_gl_enum_name(e), index,
		       val, expected);
		pass = false;
	}
}

void
test_range(int line, int index, int bo, int offset, int size)
{
	test_index(line, GL_SHADER_STORAGE_BUFFER_BINDING, index, bo);
	test_index(line, GL_SHADER_STORAGE_BUFFER_START, index, offset);
	test_index(line, GL_SHADER_STORAGE_BUFFER_SIZE, index, size);
}

void
piglit_init(int argc, char **argv)
{
	GLuint bo[2];
	int size = 1024;
	GLint max_bindings;
	GLint junk;
	GLint alignment;

	piglit_require_extension("GL_ARB_shader_storage_buffer_object");

        /* If no buffer object is bound to index, zero is returned. */
	test_range(__LINE__, 1, 0, 0, 0);

	glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &alignment);

	glGenBuffers(2, bo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bo[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_STATIC_READ);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bo[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_STATIC_READ);

	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, bo[0], 0, 1);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, bo[1], 2 * alignment, 3);
	test_range(__LINE__, 0, bo[0], 0, 1);
	test_range(__LINE__, 1, bo[1], 2 * alignment, 3);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bo[1]);
	test_range(__LINE__, 1, bo[1], 0, 0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	test_range(__LINE__, 0, 0, 0, 0);

	/* Test the error condition. */
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &max_bindings);
	glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, max_bindings, &junk);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

