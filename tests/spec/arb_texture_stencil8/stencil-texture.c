/*
 * Copyright © 2011 Intel Corporation
 * Copyright © 2015 Red Hat
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
 * \file stencil-texture.c
 * Create a stencil texture.
 * based on ext_packed_depth_stencil/depth-stencil-texture.c test.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static bool has_cube_array;

static bool
try_TexImage(GLenum internalFormat)
{
	bool pass = true;
	GLuint tex[7];
	unsigned i;

	printf("Testing glTexImage with %s...\n",
	       piglit_get_gl_enum_name(internalFormat));

	glGenTextures(ARRAY_SIZE(tex), tex);

	glBindTexture(GL_TEXTURE_1D, tex[0]);
	glTexImage1D(GL_TEXTURE_1D, 0, internalFormat,
		     16, 0,
		     GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, NULL);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
		     16, 16, 0,
		     GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, NULL);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* 3D texture is not in the list of supported STENCIL_INDEX */
	glBindTexture(GL_TEXTURE_3D, tex[2]);
	glTexImage3D(GL_TEXTURE_3D, 0, internalFormat,
		     8, 8, 8, 0,
		     GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, NULL);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	glBindTexture(GL_TEXTURE_CUBE_MAP, tex[3]);

	for (i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			     0, internalFormat,
			     16, 16, 0,
			     GL_STENCIL_INDEX, GL_UNSIGNED_BYTE,
			     NULL);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	}

	glBindTexture(GL_TEXTURE_1D_ARRAY, tex[4]);
	glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, internalFormat,
		     16, 16, 0,
		     GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, NULL);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBindTexture(GL_TEXTURE_2D_ARRAY, tex[5]);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalFormat,
		     8, 8, 8, 0,
		     GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, NULL);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	if (has_cube_array) {
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, tex[6]);
		glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, internalFormat,
			     8, 8, 6, 0,
			     GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, NULL);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	}

	glBindTexture(GL_TEXTURE_1D, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_3D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindTexture(GL_TEXTURE_1D_ARRAY, 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	if (has_cube_array)
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

	glDeleteTextures(ARRAY_SIZE(tex), tex);

	printf("Done.\n\n");

	return pass;
}

static bool
try_TexStorage(GLenum internalFormat)
{
	bool pass = true;
	GLuint tex[7];

	printf("Testing glTexStorage with %s...\n",
	       piglit_get_gl_enum_name(internalFormat));

	glGenTextures(ARRAY_SIZE(tex), tex);

	glBindTexture(GL_TEXTURE_1D, tex[0]);
	glTexStorage1D(GL_TEXTURE_1D, 1, internalFormat, 16);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, 16, 16);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBindTexture(GL_TEXTURE_3D, tex[2]);
	glTexStorage3D(GL_TEXTURE_3D, 1, internalFormat, 8, 8, 8);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	glBindTexture(GL_TEXTURE_CUBE_MAP, tex[3]);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, internalFormat, 16, 16);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBindTexture(GL_TEXTURE_1D_ARRAY, tex[4]);
	glTexStorage2D(GL_TEXTURE_1D_ARRAY, 1, internalFormat, 16, 16);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBindTexture(GL_TEXTURE_2D_ARRAY, tex[5]);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, internalFormat, 16, 16, 8);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	if (has_cube_array) {
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, tex[6]);
		glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 1, internalFormat, 16, 16, 6);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	}

	glBindTexture(GL_TEXTURE_1D, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_3D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindTexture(GL_TEXTURE_1D_ARRAY, 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	if (has_cube_array)
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

	glDeleteTextures(ARRAY_SIZE(tex), tex);

	printf("Done.\n\n");

	return pass;
}

void piglit_init(int argc, char **argv)
{
	bool pass = true;
	bool has_texture_storage;

	piglit_require_extension("GL_ARB_texture_stencil8");
	has_cube_array = piglit_get_gl_version() >= 40
		|| piglit_is_extension_supported("GL_ARB_texture_cube_map_array");

	has_texture_storage = piglit_get_gl_version() >= 42
		|| piglit_is_extension_supported("GL_ARB_texture_storage");

	pass = try_TexImage(GL_STENCIL_INDEX8) && pass;

	if (has_texture_storage) {
		pass = try_TexStorage(GL_STENCIL_INDEX8) && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
