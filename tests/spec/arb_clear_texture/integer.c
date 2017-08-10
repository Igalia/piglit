/*
 * Copyright (c) 2014 Intel Corporation
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

/** @file integer.c
 *
 * A test of using glClearTexSubImage to clear sub-regions of integer
 * tetures with a range of formats. Each format is created as a 4x4
 * texture where the first four texels are cleared to known values
 * using separate calls to glClearTexSubImage. The values are chosen
 * to potentially trigger problems with signed conversions. The rest
 * of the texture is initalised to zeroes. The textures are then read
 * back with glGetTexImage and compared with the expected values.
 */

#define TEX_WIDTH 4
#define TEX_HEIGHT 4

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

/* Values to try clearing the texture to. The number of bytes used
 * will depend on the component size for the format. The actual value
 * used will depend on the endianness of the architecture but this
 * shouldn't really matter for the test */

static const uint32_t
clear_values[][4] = {
	{ 0xffffffffU, 0x00000000U, 0x12345678U, 0x78274827U },
	{ 0x00000000U, 0xffffffffU, 0x12345678U, 0x78274827U },
	{ 0x12345678U, 0x00000000U, 0xffffffffU, 0x78274827U },
	{ 0xa82748b7U, 0x12345678U, 0x00000000U, 0xffffffffU },
};

struct format {
	GLenum internal_format;
	GLenum format;
	GLenum type;
	int component_size;
	int n_components;
};

static const struct format
formats[] = {
        { GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, 4, 4 },
        { GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT, 4, 3 },
        { GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, 2, 4 },
        { GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_SHORT, 2, 3 },
        { GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, 1, 4 },
        { GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE, 1, 3 },
        { GL_RGBA32I, GL_RGBA_INTEGER, GL_INT, 4, 4 },
        { GL_RGB32I, GL_RGB_INTEGER, GL_INT, 4, 3 },
        { GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT, 2, 4 },
        { GL_RGB16I, GL_RGB_INTEGER, GL_SHORT, 2, 3 },
        { GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE, 1, 4 },
        { GL_RGB8I, GL_RGB_INTEGER, GL_BYTE, 1, 3 },
};

static GLuint
create_texture(const struct format *format)
{
	GLubyte *tex_data;
	int texel_size;
	GLuint tex;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	texel_size = format->component_size * format->n_components;
	tex_data = malloc(texel_size * TEX_WIDTH * TEX_HEIGHT);

	memset(tex_data, 0, texel_size * TEX_WIDTH * TEX_HEIGHT);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D,
		     0, /* level */
		     format->internal_format,
		     TEX_WIDTH, TEX_HEIGHT,
		     0, /* border */
		     format->format,
		     format->type,
		     tex_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	free(tex_data);

	return tex;
}

static bool
clear_texture(GLuint tex,
	      const struct format *format)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(clear_values); i++) {
		glClearTexSubImage(tex,
				   0, /* level */
				   i % TEX_WIDTH, /* x */
				   i / TEX_WIDTH, /* y */
				   0, /* z */
				   1, 1, 1, /* width/height/depth */
				   format->format,
				   format->type,
				   clear_values[i]);

		if (!piglit_check_gl_error(GL_NO_ERROR))
			return false;
	}

	return true;
}

static bool
check_texture(GLuint tex,
	      const struct format *format)
{
	GLubyte *tex_data, *p;
	int texel_size;
	int i, j;

	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	texel_size = format->component_size * format->n_components;
	p = tex_data = malloc(texel_size * TEX_WIDTH * TEX_HEIGHT);

	glGetTexImage(GL_TEXTURE_2D, 0, format->format, format->type, tex_data);

	for (i = 0; i < ARRAY_SIZE(clear_values); i++) {
		if (memcmp(p, clear_values[i], texel_size))
			return false;
		p += texel_size;
	}

	/* The rest of the values should be zeroes */
	for (i = ARRAY_SIZE(clear_values); i < TEX_WIDTH * TEX_HEIGHT; i++)
		for (j = 0; j < texel_size; j++)
			if (*(p++) != 0)
				return false;

	return true;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint tex;
	int i;

	/* glClearTexture is either in the GL_ARB_clear_texture
	 * extension or in core in GL 4.4
	 */
	if (piglit_get_gl_version() < 44 &&
	    !piglit_is_extension_supported("GL_ARB_clear_texture")) {
		printf("OpenGL 4.4 or GL_ARB_clear_texture is required.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	/* Integer textures are either in GL 3.0 or GL_EXT_texture_integer
	 */
	if (piglit_get_gl_version() < 30 &&
	    !piglit_is_extension_supported("GL_EXT_texture_integer")) {
		printf("OpenGL 3.0 or GL_EXT_texture_integer is required.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	for (i = 0; i < ARRAY_SIZE(formats); i++) {
		tex = create_texture(formats + i);
		pass &= clear_texture(tex, formats + i);
		pass &= check_texture(tex, formats + i);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &tex);
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* unused */
	return PIGLIT_FAIL;
}
