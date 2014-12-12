/*
 * Copyright 2014 Intel Corporation
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

/** @file tex-errors.c
 *
 * Checks to see if *Tex* functions throw the correct errors. This is not
 * exhaustive since some *Tex* errors are covered elsewhere.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{

}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLuint name;

	/* n is negative */
	glGenTextures(-1, &name);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	glGenTextures(1, &name);
	/* Not a valid target */
	glBindTexture(GL_INVALID_ENUM, name);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	glBindTexture(GL_TEXTURE_2D, name);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 4, 5);
	/* Wrong dimensionality. */
	glBindTexture(GL_TEXTURE_1D, name);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* n is negative */
	glDeleteTextures(-1, &name);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

