/*
 * Copyright © 2011 Intel Corporation
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
 *
 * Authors:
 *    Yuanhan Liu <yuanhan.liu@linux.intel.com>
 *
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
	GLenum target = GL_ARRAY_BUFFER;
	GLbitfield access = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
	GLuint buffer;

	piglit_require_gl_version(15);

	piglit_require_extension("GL_ARB_map_buffer_range");

	glGenBuffers(1, &buffer);
	glBindBuffer(target, buffer);
	glBufferData(target, 100, NULL, GL_STATIC_DRAW);
	glGetError();

	/*
	 * Test cases for checking GL_INVALID_VALUE error
	 *
	 * GL_INVALID_VALUE is generated if either of offset or length is negative,
	 * or if offset + length is greater than the value of GL_BUFFER_SIZE, or if
	 * access has any bits set other than those defined bits.
	 *
	 */

	/* offset < 0 */
	glMapBufferRange(target, -1, 1, access);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	/* length < 0 */
	glMapBufferRange(target, 0, -1, access);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	/* offset + lenght > GL_BUFFER_SIZE */
	glMapBufferRange(target, 1, GL_BUFFER_SIZE, access);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	/* undefined access bits */
	glMapBufferRange(target, 0, 10, 0xffffffff);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);


	/*
	 * Tests cases for checking GL_INVALID_OPERATION error
	 *
	 * GL_INVALID_OPERATION is generated for any of the following conditions:
	 *
	 *   (a) The buffer is already in a mapped state.
	 *
	 *   (b) Neither GL_MAP_READ_BIT or GL_MAP_WRITE_BIT is set.
	 *
	 *   (c) GL_MAP_READ_BIT is set and any of GL_MAP_INVALIDATE_RANGE_BIT,
	 *   GL_MAP_INVALIDATE_BUFFER_BIT, or GL_MAP_UNSYNCHRONIZED_BIT is set.
	 *
	 *   (d) GL_MAP_FLUSH_EXPLICIT_BIT is set and GL_MAP_WRITE_BIT is not set.
	 */
	glMapBufferRange(target, 0, 10, access);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* (a) map again */
	glMapBufferRange(target, 0, 10, access);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);
	glUnmapBuffer(target);

	/* for (b) case */
	glMapBufferRange(target, 0, 10, 0);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

	/* for (c) case */
	glMapBufferRange(target, 0, 10, GL_MAP_READ_BIT |
					      GL_MAP_INVALIDATE_RANGE_BIT);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

	glMapBufferRange(target, 0, 10, GL_MAP_READ_BIT |
					      GL_MAP_INVALIDATE_BUFFER_BIT);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

	glMapBufferRange(target, 0, 10, GL_MAP_READ_BIT |
					      GL_MAP_UNSYNCHRONIZED_BIT);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

	/* for (d) case */
	glMapBufferRange(target, 0, 10, GL_MAP_FLUSH_EXPLICIT_BIT |
					      GL_MAP_READ_BIT);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);


	piglit_report_result(PIGLIT_PASS);
}
