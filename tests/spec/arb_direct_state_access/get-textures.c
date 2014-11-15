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

/** @file get-textures.c
 *
 * Tests glGetTextureImage to see if it behaves in the expected way,
 * throwing the correct errors, etc.
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
	piglit_require_extension("GL_ARB_direct_state_access");
	piglit_require_extension("GL_ARB_texture_storage");
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLuint name;
	GLubyte *data = malloc(50 * 50 * 6 * 4 * sizeof(GLubyte));
	GLubyte *image = malloc(50 * 50 * 4 * sizeof(GLubyte));

	/* Throw some invalid inputs at glGetTextureImage. */

	/* Non-gen-ed name */
	glGetTextureImage(3, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0, data);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* Unsupported target. */
	glGenTextures(1, &name);
	glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X, name);
	glGetTextureImage(name, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0, data);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);
	glDeleteTextures(1, &name);

	/* Unsupported dsa target for non-dsa version. */
	glGetTexImage(GL_TEXTURE_CUBE_MAP, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		     data);
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);

	/* No Storage
	 *
	 * The spec doesn't say what should happen in this case.  This is
	 * addressed by Khronos Bug 13223.
	 */
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &name);
	glGetTextureImage(name, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0, data);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);
	glDeleteTextures(1, &name);

	/* Insufficient storage
	 *
	 * The spec doesn't say what should happen in this case.  This is
	 * addressed by Khronos Bug 13223.
	 */
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &name);
	glBindTexture(GL_TEXTURE_CUBE_MAP, name);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0,
		     GL_RGBA8, 50, 50, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0,
		     GL_RGBA8, 50, 50, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0,
		     GL_RGBA8, 50, 50, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	/* Note: GL_TEXTURE_CUBE_MAP_NEGATIVE_Y not set */
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0,
		     GL_RGBA8, 50, 50, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0,
		     GL_RGBA8, 50, 50, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGetTextureImage(name, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0, data);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);
	glDeleteTextures(1, &name);

	/* Trivial, but should work. */
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &name);
	glTextureStorage2D(name, 1, GL_RGBA8, 50, 50);
	glGetTextureImage(name, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			  50 * 50 * 6 * 4, data);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

