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

/** @file bind-texture-unit.c
 *
 * Tests glBindTextureUnit to see if it behaves in the expected way,
 * throwing the correct errors, etc.
 */

#include "piglit-util-gl.h"
#include "dsa-utils.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;
	config.supports_gl_compat_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | 
		PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_direct_state_access");
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLuint name = 3;
	GLint nunits;

	/* Throw some invalid inputs at BindTextureUnit. */


	/* Section 8.1. of the OpenGL 4.5 Core Profile spec says:
	 *
	 *     "An INVALID_OPERATION error is generated by BindTextureUnit if
	 *     texture is not zero or the name of an existing texture object."
	 */

	/* Texture name doesn't exist */
	glBindTextureUnit(0, name);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	/* Texture name exists, but texture object does not */
	glGenTextures(1, &name);
	glBindTextureUnit(0, name);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	/* Section 8.1. of the OpenGL 4.5 Core Profile spec says for
	 * BindTextures:
	 *
	 *     "An INVALID_OPERATION error is generated if first + count is
	 *     greater than the number of texture image units supported by the
	 *     implementation."
	 *
	 * However, it doesn't say the same about BindTextureUnit. Table 2.3
	 * implies that a numeric argument out of range yields INVALID_VALUE,
	 * not INVALID_OPERATION.
	 */

	/* Texture unit doesn't exist */
	glDeleteTextures(1, &name);
	glCreateTextures(GL_TEXTURE_2D, 1, &name);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &nunits);
	glBindTextureUnit(nunits, name); /* Too High */
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	/* Trivial, but should work. */
	glBindTextureUnit(1, name);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

