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

/** @file negative-bindbufferrange-range.c
 *
 * From the GL_ARB_uniform_buffer_object spec:
 *
 *     "For BindBufferRange, <offset> specifies a starting offset into
 *      the buffer object <buffer>, and <size> specifies the amount of
 *      data that can be read from the buffer object while used as the
 *      storage for a uniform block. Both <offset> and <size> are in
 *      basic machine units. The error INVALID_VALUE is generated if
 *      the value of <size> is less than or equal to zero, if <offset>
 *      + <size> is greater than the value of BUFFER_SIZE, or if
 *      <offset> is not a multiple of the implementation-dependent
 *      required alignment
 *      (UNIFORM_BUFFER_OFFSET_ALIGNMENT). BindBufferBase is
 *      equivalent to calling BindBufferRange with <offset> zero and
 *      <size> equal to the size of <buffer>."
 */

#include "piglit-util.h"

PIGLIT_GL_TEST_MAIN(
    10 /*window_width*/,
    10 /*window_height*/,
    GLUT_RGB | GLUT_DOUBLE | GLUT_ALPHA)

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLint alignment;
	GLuint bo;
	int size = 1024;
	int index = 0;
	int i;

	piglit_require_extension("GL_ARB_uniform_buffer_object");

	glGenBuffers(1, &bo);
	glBindBuffer(GL_UNIFORM_BUFFER, bo);
	glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_READ);

	glBindBufferRange(GL_UNIFORM_BUFFER, index, bo, 0, 0);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	glBindBufferRange(GL_UNIFORM_BUFFER, index, bo, 0, -1);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	glBindBufferRange(GL_UNIFORM_BUFFER, index, bo, 0, size + 1);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	glBindBufferRange(GL_UNIFORM_BUFFER, index, bo, 1, size);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
	for (i = 1; i < alignment; i++) {
		glBindBufferRange(GL_UNIFORM_BUFFER, index, bo, i, 4);
		if (!piglit_check_gl_error(GL_INVALID_VALUE))
			pass = false;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
