/* Copyright 2012 Intel Corporation
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
 * \file genmipmap-errors.c
 * Do error checking for glGenerateMipmap() with various texture internal formats.
 * GL_INVALID_OPERATION is expected in case of integer or depth-stencil textures.
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Test textures having integer or depth-stencil internalFormat with
 * glGenerateMipmap. GL_INVALID_OPERATION should be thrown by the
 * implementation.
 */
static bool
test_genmipmap_errors(void)
{
	GLuint tex;
	static const struct {
		GLenum intFormat, srcFormat, srcType;
		const char *extension[2];
	} formats[] = {
		/* Integer internal formats */
		{ GL_RGBA8I_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },
		{ GL_RGBA16I_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },
		{ GL_RGBA32I_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },
		{ GL_RGB8I_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },
		{ GL_RGB16I_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL } },
		{ GL_RGB32I, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },
		{ GL_RG32I, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", "GL_ARB_texture_rg"} },
		{ GL_R32I, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", "GL_ARB_texture_rg"} },
		{ GL_ALPHA8I_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },
		{ GL_LUMINANCE8I_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },
		{ GL_INTENSITY8I_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },
		{ GL_LUMINANCE_ALPHA8I_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },

		{ GL_RGBA8UI_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },
		{ GL_RGBA16UI_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },
		{ GL_RGBA32UI_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },
		{ GL_RGB8UI_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },
		{ GL_RGB16UI_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer" } },
		{ GL_RGB32UI_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },
		{ GL_RG32UI, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", "GL_ARB_texture_rg"} },
		{ GL_R32UI, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", "GL_ARB_texture_rg"} },
		{ GL_ALPHA8UI_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },
		{ GL_LUMINANCE8UI_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },
		{ GL_INTENSITY8UI_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },
		{ GL_LUMINANCE_ALPHA8UI_EXT, GL_RGBA_INTEGER, GL_INT,
		{"GL_EXT_texture_integer", NULL} },

		/* Packed depth / stencil formats */
		{ GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
		{"GL_EXT_packed_depth_stencil", NULL} },
		{ GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
		{"GL_ARB_depth_buffer_float", NULL} }
	};
	int i, j;
	bool pass = true;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	for (i = 0; i < ARRAY_SIZE(formats); i++) {

		bool is_ext_supported = true;
		for (j = 0; j < ARRAY_SIZE(formats[i].extension); j++) {
			if ((formats[i].extension[j] != NULL) &&
			   !piglit_is_extension_supported(formats[i].extension[j]))
			{
				printf("Skipping %s\n",
				piglit_get_gl_enum_name(formats[i].intFormat));
				is_ext_supported = false;
			}
		}
		if (!is_ext_supported)
			continue;

		glTexImage2D(GL_TEXTURE_2D, 0, formats[i].intFormat,
			     16, 16, 0,
			     formats[i].srcFormat, formats[i].srcType, NULL);

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
		glGenerateMipmap(GL_TEXTURE_2D);
		pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	}
	glDeleteTextures(1, &tex);
	return pass;
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
	bool pass = test_genmipmap_errors();
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
