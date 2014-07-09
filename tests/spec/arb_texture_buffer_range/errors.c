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
 */

#include "piglit-util-gl.h"

/**
 * @file errors.c
 *
 * Tests error conditions and queries for glTexBufferRange.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

/* use min of MAX_TEXTURE_BUFFER_SIZE */
#define TBO_SIZE (1 << 16)

void
piglit_init(int argc, char **argv)
{
	GLint align, value[2];
	GLuint tex, bo;

	piglit_require_gl_version(20);
	piglit_require_extension("GL_ARB_texture_buffer_range");

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_BUFFER, tex);
	glGenBuffers(1, &bo);
	glBindBuffer(GL_TEXTURE_BUFFER, bo);

	glGetIntegerv(GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT, &align);
	if (align < 1) {
		fprintf(stderr, "GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT == %i, "
		        "should be >= 1\n", align);
		piglit_report_result(PIGLIT_FAIL);
	}

	/* If <offset> is negative or if <size> is
	 * less than or equal to zero or if <offset> + <size> is greater than
	 * the value of BUFFER_SIZE for the buffer bound to <target>, of if
	 * <offset> is not an integer multiple of
	 * TEXTURE_BUFFER_OFFSET_ALIGNMENT, then the error INVALID_VALUE
	 * is generated.
	 */

	glTexBufferRange(GL_TEXTURE_BUFFER, GL_RGBA8, bo, 0, 4);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glBufferData(GL_TEXTURE_BUFFER, TBO_SIZE, NULL, GL_STATIC_DRAW);

	glTexBufferRange(GL_TEXTURE_BUFFER, GL_RGBA8, bo, -align, 4);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glTexBufferRange(GL_TEXTURE_BUFFER, GL_RGBA8, bo, 0, 0);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glTexBufferRange(GL_TEXTURE_BUFFER, GL_RGBA8, bo, 0, -16);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	if (align > 1) {
		glTexBufferRange(GL_TEXTURE_BUFFER, GL_RGBA8, bo, align / 2, 16);
		if (!piglit_check_gl_error(GL_INVALID_VALUE))
			piglit_report_result(PIGLIT_FAIL);
	}

	glTexBufferRange(GL_TEXTURE_BUFFER, GL_RGBA8, bo,
	                 align, TBO_SIZE - align);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		fprintf(stderr, "glTexBufferRange failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	glGetTexLevelParameteriv(GL_TEXTURE_BUFFER, 0, GL_TEXTURE_BUFFER_OFFSET,
	                         &value[0]);
	glGetTexLevelParameteriv(GL_TEXTURE_BUFFER, 0, GL_TEXTURE_BUFFER_SIZE,
	                         &value[1]);
	if (value[0] != align || value[1] != TBO_SIZE - align) {
		fprintf(stderr, "GL_TEXTURE_BUFFER_OFFSET/SIZE returned %i/%i, "
		        "expected %i/%i\n",
		        value[0], value[1], align, TBO_SIZE - align);
		piglit_report_result(PIGLIT_FAIL);
	}

	/* If <buffer> is zero, then any buffer object attached to the
	 * buffer texture is detached, the values <offset> and <size> are
	 * ignored and the state for <offset> and <size> for the
	 * buffer texture are reset to zero.
	 */

	glTexBufferRange(GL_TEXTURE_BUFFER, GL_RGBA8, 0, -align, TBO_SIZE * 2);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGetTexLevelParameteriv(GL_TEXTURE_BUFFER, 0, GL_TEXTURE_BUFFER_OFFSET,
	                         &value[0]);
	glGetTexLevelParameteriv(GL_TEXTURE_BUFFER, 0, GL_TEXTURE_BUFFER_SIZE,
	                         &value[1]);
	if (value[0] || value[1]) {
		fprintf(stderr, "buffer detached but "
		        "GL_TEXTURE_BUFFER_OFFSET/SIZE "
		        "not reset to 0\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}

