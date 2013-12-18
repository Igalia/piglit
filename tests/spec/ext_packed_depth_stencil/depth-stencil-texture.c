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

/**
 * \file negative-texture
 * Create a depth-stencil texture when ARB_depth_texture is not supported.
 *
 * The EXT_packed_depth_stencil spec neglects to mention an interaction
 * (though the header of the spec says "ARB_depth_texture affects the
 * definition of this extension."), but ARB_framebuffer_object, which includes
 * EXT_packed_depth_stencil functionality says:
 *
 *     "If ARB_depth_texture or SGIX_depth_texture is supported,
 *     GL_DEPTH_STENCIL/GL_UNSIGNED_INT_24_8 data can also be used for
 *     textures;"
 *
 * In cases where neither ARB_depth_texture nor SGIX_depth_texture is
 * supported, trying to create a texture with a depth-stencil format should
 * generate an error.
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 12;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static bool has_texture_cube_map = false;
static bool has_depth_texture_cube_map = false;
static bool has_depth_texture = false;

static bool
try_TexImage(GLenum internalFormat)
{
	bool pass = true;
	GLuint tex[4];

	const GLenum expected_error = has_depth_texture
		? GL_NO_ERROR : GL_INVALID_VALUE;

	const GLenum expected_3D_error = has_depth_texture
		? GL_INVALID_OPERATION : GL_INVALID_VALUE;

	const GLenum expected_cube_error = has_depth_texture_cube_map
		? GL_NO_ERROR : expected_3D_error;

	printf("Testing glTexImage with %s...\n",
	       piglit_get_gl_enum_name(internalFormat));

	glGenTextures(ARRAY_SIZE(tex), tex);

	glBindTexture(GL_TEXTURE_1D, tex[0]);
	glTexImage1D(GL_TEXTURE_1D, 0, internalFormat,
		     16, 0,
		     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	pass = piglit_check_gl_error(expected_error) && pass;


	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
		     16, 16, 0,
		     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	pass = piglit_check_gl_error(expected_error) && pass;


	/* Section 3.8.1 (Texture Image Specification) of the OpenGL 2.1 spec
	 * says:
	 *
	 *     "Textures with a base internal format of DEPTH_COMPONENT are
	 *     supported by texture image specification commands only if
	 *     target is TEXTURE_1D, TEXTURE_2D, PROXY_TEXTURE_1D or
	 *     PROXY_TEXTURE_2D. Using this format in conjunction with any
	 *     other target will result in an INVALID_OPERATION error."
	 *
	 * The OpenGL 4.4 spec lists the same error, but it greatly expands
	 * the list of valid texture targets.
	 */
	glBindTexture(GL_TEXTURE_3D, tex[2]);
	glTexImage3D(GL_TEXTURE_3D, 0, internalFormat,
		     8, 8, 8, 0,
		     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	pass = piglit_check_gl_error(expected_3D_error) && pass;

	if (has_texture_cube_map) {
		unsigned i;

		glBindTexture(GL_TEXTURE_CUBE_MAP, tex[3]);

		for (i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				     0, internalFormat,
				     16, 16, 0,
				     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
				     NULL);
			pass = piglit_check_gl_error(expected_cube_error)
				&& pass;
		}
	}

	glBindTexture(GL_TEXTURE_1D, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_3D, 0);
	if (has_texture_cube_map)
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glDeleteTextures(ARRAY_SIZE(tex), tex);

	printf("Done.\n\n");

	return pass;
}

static bool
try_TexStorage(GLenum internalFormat)
{
	bool pass = true;
	GLuint tex[4];

	GLenum expected_error;
	GLenum expected_3D_error;
	GLenum expected_cube_error;

	/* The GL_ARB_texture_storage spec says:
	 *
	 *     "- If <internalformat> is one of the internal formats listed in
	 *      table 3.11, an INVALID_ENUM error is generated."
	 *
	 * Table 3.11 lists the unsized formats, including GL_DEPTH_STENCIL.
	 */
	if (internalFormat == GL_DEPTH_STENCIL) {
		expected_error = has_depth_texture
			? GL_INVALID_ENUM : GL_INVALID_VALUE;
		expected_3D_error = has_depth_texture
			? GL_INVALID_ENUM : GL_INVALID_VALUE;
		expected_cube_error = has_depth_texture_cube_map
			? GL_INVALID_ENUM : GL_INVALID_VALUE;
	} else {
		expected_error = has_depth_texture
			? GL_NO_ERROR : GL_INVALID_VALUE;
		expected_3D_error = has_depth_texture
			? GL_INVALID_OPERATION : GL_INVALID_VALUE;
		expected_cube_error = has_depth_texture_cube_map
			? GL_NO_ERROR : expected_3D_error;
	}

	printf("Testing glTexStorage with %s...\n",
	       piglit_get_gl_enum_name(internalFormat));

	glGenTextures(ARRAY_SIZE(tex), tex);

	glBindTexture(GL_TEXTURE_1D, tex[0]);
	glTexStorage1D(GL_TEXTURE_1D, 1, internalFormat, 16);
	pass = piglit_check_gl_error(expected_error) && pass;

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, 16, 16);
	pass = piglit_check_gl_error(expected_error) && pass;

	glBindTexture(GL_TEXTURE_3D, tex[2]);
	glTexStorage3D(GL_TEXTURE_3D, 1, internalFormat, 8, 8, 8);
	pass = piglit_check_gl_error(expected_3D_error) && pass;

	if (has_texture_cube_map) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex[3]);
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, internalFormat, 16, 16);
		pass = piglit_check_gl_error(expected_cube_error) && pass;
	}

	glBindTexture(GL_TEXTURE_1D, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_3D, 0);
	if (has_texture_cube_map)
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glDeleteTextures(ARRAY_SIZE(tex), tex);

	printf("Done.\n\n");

	return pass;
}

void piglit_init(int argc, char **argv)
{
	bool pass = true;

	if (piglit_get_gl_version() < 30
	    && !piglit_is_extension_supported("GL_EXT_packed_depth_stencil")
	    && !piglit_is_extension_supported("GL_ARB_framebuffer_object")) {
		printf("OpenGL 3.0, GL_EXT_packed_depth_stencil, or "
		       "GL_ARB_framebuffer_object is required.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	has_depth_texture = piglit_get_gl_version() >= 14
		|| piglit_is_extension_supported("GL_ARB_depth_texture")
		|| piglit_is_extension_supported("GL_SGIX_depth_texture");

	has_texture_cube_map = piglit_get_gl_version() >= 13
		|| piglit_is_extension_supported("GL_ARB_texture_cube_map");

	has_depth_texture_cube_map = piglit_get_gl_version() >= 30
		|| piglit_is_extension_supported("GL_EXT_gpu_shader4");

	pass = try_TexImage(GL_DEPTH_STENCIL) && pass;
	pass = try_TexImage(GL_DEPTH24_STENCIL8) && pass;

	if (piglit_is_extension_supported("GL_ARB_texture_storage")) {
		pass = try_TexStorage(GL_DEPTH_STENCIL) && pass;
		pass = try_TexStorage(GL_DEPTH24_STENCIL8) && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
