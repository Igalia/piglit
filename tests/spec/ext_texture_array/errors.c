/*
 * Copyright 2014 VMware, Inc.
 *
 * Permission is hereby , free of charge, to any person obtaining a
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
 * Basic API error tests for GL_EXT_texture_array.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


static GLint MaxLayers;


static bool
test_1d_dimensions(void)
{
	bool pass = true;
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_1D_ARRAY, tex);

	/*
	 * zero dimensions should be OK
	 */
	glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, GL_RGBA,
		     0, 0, /* w, h */
		     0, /* border */
		     GL_RGBA, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, GL_RGBA,
		     1, 0, /* w, h */
		     0, /* border */
		     GL_RGBA, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, GL_RGBA,
		     0, 1, /* w, h */
		     0, /* border */
		     GL_RGBA, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	/* Test too many layers */
	glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, GL_RGBA,
		     32, MaxLayers + 1, /* w, h */
		     0, /* border */
		     GL_RGBA, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	/* Test invalid target */
	glTexImage2D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
		     32, 2, /* w, h */
		     0, /* border */
		     GL_RGBA, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_INVALID_ENUM))
		pass = false;

        glDeleteTextures(1, &tex);

	return pass;
}


static bool
test_2d_dimensions(void)
{
	bool pass = true;
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);

	/*
	 * zero dimensions should be OK
	 */
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
		     0, 0, 0, /* w, h, d */
		     0, /* border */
		     GL_RGBA, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
		     1, 0, 0, /* w, h, d */
		     0, /* border */
		     GL_RGBA, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
		     1, 1, 0, /* w, h, d */
		     0, /* border */
		     GL_RGBA, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
		     1, 0, 1, /* w, h, d */
		     0, /* border */
		     GL_RGBA, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	/* Test too many layers */
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
		     32, 32, MaxLayers + 1, /* w, h, d */
		     0, /* border */
		     GL_RGBA, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	/* Test invalid target */
	glTexImage3D(GL_TEXTURE_1D_ARRAY, 0, GL_RGBA,
		     32, 32, 2, /* w, h, d */
		     0, /* border */
		     GL_RGBA, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_INVALID_ENUM))
		pass = false;

        glDeleteTextures(1, &tex);

	return pass;
}


enum piglit_result
piglit_display(void)
{
	bool pass;

	pass = test_1d_dimensions();
	pass = test_2d_dimensions() && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_texture_array");

	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS_EXT, &MaxLayers);
}
