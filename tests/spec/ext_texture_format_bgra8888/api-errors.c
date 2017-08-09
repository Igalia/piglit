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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * @file
 * Tests glTex(Sub)Image functions for valid and invalid combinations of
 * GL_BGRA_EXT format and internal format, as defined by the extension
 * EXT_texture_format_BGRA888.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static bool
run_test(void)
{
	GLuint tex;
	bool pass = true;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* glTexImage2D */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 2, 2, 0,
		     GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	/* glTexImage2D, invalid internal format */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0,
		     GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;

	/* glTexImage2D, invalid format */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 2, 2, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;

	/* glTexImage2D, invalid type */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 2, 2, 0,
		     GL_BGRA_EXT, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;

	/* glTexSubImage2D */
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1,
		        GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	/* glTexSubImage2D, invalid format */
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1,
		        GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;

	/* glTexSubImage2D, invalid type */
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1,
		        GL_BGRA_EXT, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;

	glDeleteTextures(1, &tex);

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = run_test();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_texture_format_BGRA8888");
}
