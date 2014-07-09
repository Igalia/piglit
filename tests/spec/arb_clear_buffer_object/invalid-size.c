/*
 * Copyright © 2014 Intel Corporation
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
 * \file invalid-size.c
 *
 * From the GL_ARB_clear_buffer_object spec:
 * "Both <offset> and <range> must be multiples of the number of basic machine
 *  units per-element for that internal format specified by <internalformat>,
 *  otherwise the error INVALID_VALUE is generated."
 * and
 * "[ClearBufferData] is equivalent to calling ClearBufferSubData with <target>,
 *  <internalformat> and <data> as specified, with <offset> set to zero, and
 *  <size> set to the value of BUFFER_SIZE for the buffer bound to <target>."
 *
 * Test that the required GL_INVALID_VALUE error is generated if the buffer size
 * is not a multiple of the internal format size.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 15;
	config.supports_gl_core_version = 31;

PIGLIT_GL_TEST_CONFIG_END


void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	const int buffer_size = (1<<20) - 4;
	unsigned int buffer;
	static const char *const data_zero = "\x00\x00\x00\x00\x00\x00\x00\x00";

	piglit_require_extension("GL_ARB_clear_buffer_object");

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STREAM_READ);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	glClearBufferData(GL_ARRAY_BUFFER, GL_RGBA16, GL_RGBA,
			GL_UNSIGNED_SHORT, data_zero);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &buffer);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

