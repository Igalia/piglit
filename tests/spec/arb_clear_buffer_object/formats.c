/*
 * Copyright © 2014 Intel Corporation
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
 * \file formats.c
 *
 * Test clearing the entire buffer with multiple internal formats. Pass the data
 * to clear the buffer with in a format so that the GL doesn't have to do any
 * format conversion.
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 15;
	config.supports_gl_core_version = 31;

PIGLIT_GL_TEST_CONFIG_END


static const struct {
	GLenum internal_format;
	GLenum format;
	GLenum type;

	int size;
	bool core_profile;
	const char *ext[3];
} formats[] = {
/*	internal_format			format				type			size	core	extensions	*/
	{GL_ALPHA8,			GL_ALPHA,			GL_UNSIGNED_BYTE,	1,	false,	{NULL,	NULL,	NULL}},
	{GL_ALPHA16,			GL_ALPHA,			GL_UNSIGNED_SHORT,	2,	false,	{NULL,	NULL,	NULL}},
	{GL_LUMINANCE8,			GL_LUMINANCE,			GL_UNSIGNED_BYTE,	1,	false,	{NULL,	NULL,	NULL}},
	{GL_LUMINANCE16,		GL_LUMINANCE,			GL_UNSIGNED_SHORT,	2,	false,	{NULL,	NULL,	NULL}},
	{GL_LUMINANCE8_ALPHA8,		GL_LUMINANCE_ALPHA,		GL_UNSIGNED_BYTE,	2,	false,	{NULL,	NULL,	NULL}},
	{GL_LUMINANCE16_ALPHA16,	GL_LUMINANCE_ALPHA,		GL_UNSIGNED_SHORT,	4,	false,	{NULL,	NULL,	NULL}},
	{GL_INTENSITY8,			GL_RED,				GL_UNSIGNED_BYTE,	1,	false,	{NULL,	NULL,	NULL}},
	{GL_INTENSITY16,		GL_RED,				GL_UNSIGNED_SHORT,	2,	false,	{NULL,	NULL,	NULL}},
	{GL_RGBA8,			GL_RGBA,			GL_UNSIGNED_BYTE,	4,	true,	{NULL,	NULL,	NULL}},
	{GL_RGBA16,			GL_RGBA,			GL_UNSIGNED_SHORT,	8,	true,	{NULL,	NULL,	NULL}},
/* texture_float */
	{GL_ALPHA32F_ARB,		GL_ALPHA,			GL_FLOAT,		4,	false,	{NULL,	"GL_ARB_texture_float",	NULL}},
	{GL_LUMINANCE32F_ARB,		GL_LUMINANCE,			GL_FLOAT,		4,	false,	{NULL,	"GL_ARB_texture_float",	NULL}},
	{GL_LUMINANCE_ALPHA32F_ARB,	GL_LUMINANCE_ALPHA,		GL_FLOAT,		8,	false,	{NULL,	"GL_ARB_texture_float",	NULL}},
	{GL_INTENSITY32F_ARB,		GL_RED,				GL_FLOAT,		4,	false,	{NULL,	"GL_ARB_texture_float",	NULL}},
	{GL_RGB32F,			GL_RGB,				GL_FLOAT,		12,	true,	{NULL,	"GL_ARB_texture_float",	"GL_ARB_texture_buffer_object_rgb32"}},
	{GL_RGBA32F_ARB,		GL_RGBA,			GL_FLOAT,		16,	true,	{NULL,	"GL_ARB_texture_float",	NULL}},
/* texture_half_float */
	{GL_ALPHA16F_ARB,		GL_ALPHA,			GL_HALF_FLOAT,		2,	false,	{NULL,	"GL_ARB_texture_float",	"GL_ARB_half_float_pixel"}},
	{GL_LUMINANCE16F_ARB,		GL_LUMINANCE,			GL_HALF_FLOAT,		2,	false,	{NULL,	"GL_ARB_texture_float",	"GL_ARB_half_float_pixel"}},
	{GL_LUMINANCE_ALPHA16F_ARB,	GL_LUMINANCE_ALPHA,		GL_HALF_FLOAT,		4,	false,	{NULL,	"GL_ARB_texture_float",	"GL_ARB_half_float_pixel"}},
	{GL_INTENSITY16F_ARB,		GL_RED,				GL_HALF_FLOAT,		2,	false,	{NULL,	"GL_ARB_texture_float",	"GL_ARB_half_float_pixel"}},
	{GL_RGBA16F_ARB,		GL_RGBA,			GL_HALF_FLOAT,		8,	true,	{NULL,	"GL_ARB_texture_float",	"GL_ARB_half_float_pixel"}},
/* texture_integer */
	{GL_ALPHA8I_EXT,		GL_ALPHA_INTEGER,		GL_BYTE,		1,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_ALPHA16I_EXT,		GL_ALPHA_INTEGER,		GL_SHORT,		2,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_ALPHA32I_EXT,		GL_ALPHA_INTEGER,		GL_INT,			4,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_ALPHA8UI_EXT,		GL_ALPHA_INTEGER,		GL_UNSIGNED_BYTE,	1,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_ALPHA16UI_EXT,		GL_ALPHA_INTEGER,		GL_UNSIGNED_SHORT,	2,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_ALPHA32UI_EXT,		GL_ALPHA_INTEGER,		GL_UNSIGNED_INT,	4,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},

	{GL_LUMINANCE8I_EXT,		GL_LUMINANCE_INTEGER_EXT,	GL_BYTE,		1,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_LUMINANCE16I_EXT,		GL_LUMINANCE_INTEGER_EXT,	GL_SHORT,		2,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_LUMINANCE32I_EXT,		GL_LUMINANCE_INTEGER_EXT,	GL_INT,			4,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_LUMINANCE8UI_EXT,		GL_LUMINANCE_INTEGER_EXT,	GL_UNSIGNED_BYTE,	1,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_LUMINANCE16UI_EXT,		GL_LUMINANCE_INTEGER_EXT,	GL_UNSIGNED_SHORT,	2,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_LUMINANCE32UI_EXT,		GL_LUMINANCE_INTEGER_EXT,	GL_UNSIGNED_INT,	4,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},

	{GL_LUMINANCE_ALPHA8I_EXT,	GL_LUMINANCE_ALPHA_INTEGER_EXT,	GL_BYTE,		2,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_LUMINANCE_ALPHA16I_EXT,	GL_LUMINANCE_ALPHA_INTEGER_EXT,	GL_SHORT,		4,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_LUMINANCE_ALPHA32I_EXT,	GL_LUMINANCE_ALPHA_INTEGER_EXT,	GL_INT,			8,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_LUMINANCE_ALPHA8UI_EXT,	GL_LUMINANCE_ALPHA_INTEGER_EXT,	GL_UNSIGNED_BYTE,	2,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_LUMINANCE_ALPHA16UI_EXT,	GL_LUMINANCE_ALPHA_INTEGER_EXT,	GL_UNSIGNED_SHORT,	4,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_LUMINANCE_ALPHA32UI_EXT,	GL_LUMINANCE_ALPHA_INTEGER_EXT,	GL_UNSIGNED_INT,	8,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},

	{GL_INTENSITY8I_EXT,		GL_RED_INTEGER,			GL_BYTE,		1,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_INTENSITY16I_EXT,		GL_RED_INTEGER,			GL_SHORT	,	2,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_INTENSITY32I_EXT,		GL_RED_INTEGER,			GL_INT,			4,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_INTENSITY8UI_EXT,		GL_RED_INTEGER,			GL_UNSIGNED_BYTE,	1,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_INTENSITY16UI_EXT,		GL_RED_INTEGER,			GL_UNSIGNED_SHORT,	2,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_INTENSITY32UI_EXT,		GL_RED_INTEGER,			GL_UNSIGNED_INT,	4,	false,	{NULL,	"GL_EXT_texture_integer",	NULL}},

	{GL_RGB32I,			GL_RGB_INTEGER,			GL_INT,			12,	true,	{NULL,	"GL_EXT_texture_integer",	"GL_ARB_texture_buffer_object_rgb32"}},
	{GL_RGB32UI,			GL_RGB_INTEGER,			GL_UNSIGNED_INT,	12,	true,	{NULL,	"GL_EXT_texture_integer",	"GL_ARB_texture_buffer_object_rgb32"}},

	{GL_RGBA8I_EXT,			GL_RGBA_INTEGER,		GL_BYTE,		4,	true,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_RGBA16I_EXT,		GL_RGBA_INTEGER,		GL_SHORT,		8,	true,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_RGBA32I_EXT,		GL_RGBA_INTEGER,		GL_INT,			16,	true,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_RGBA8UI_EXT,		GL_RGBA_INTEGER,		GL_UNSIGNED_BYTE,	4,	true,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_RGBA16UI_EXT,		GL_RGBA_INTEGER,		GL_UNSIGNED_SHORT,	8,	true,	{NULL,	"GL_EXT_texture_integer",	NULL}},
	{GL_RGBA32UI_EXT,		GL_RGBA_INTEGER,		GL_UNSIGNED_INT,	16,	true,	{NULL,	"GL_EXT_texture_integer",	NULL}},
/* texture_rg */
	{GL_R8,				GL_RED,				GL_UNSIGNED_BYTE,	1,	true,	{"GL_ARB_texture_rg",	NULL,				NULL}},
	{GL_R16,			GL_RED,				GL_UNSIGNED_SHORT,	2,	true,	{"GL_ARB_texture_rg",	NULL,				NULL}},
	{GL_R16F,			GL_RED,				GL_HALF_FLOAT,		2,	true,	{"GL_ARB_texture_rg",	"GL_ARB_texture_float",		"GL_ARB_half_float_pixel"}},
	{GL_R32F,			GL_RED,				GL_FLOAT,		4,	true,	{"GL_ARB_texture_rg",	"GL_ARB_texture_float",		NULL}},
	{GL_R8I,			GL_RED_INTEGER,			GL_BYTE,		1,	true,	{"GL_ARB_texture_rg",	"GL_EXT_texture_integer",	NULL}},
	{GL_R16I,			GL_RED_INTEGER,			GL_SHORT,		2,	true,	{"GL_ARB_texture_rg",	"GL_EXT_texture_integer",	NULL}},
	{GL_R32I,			GL_RED_INTEGER,			GL_INT,			4,	true,	{"GL_ARB_texture_rg",	"GL_EXT_texture_integer",	NULL}},
	{GL_R8UI,			GL_RED_INTEGER,			GL_UNSIGNED_BYTE,	1,	true,	{"GL_ARB_texture_rg",	"GL_EXT_texture_integer",	NULL}},
	{GL_R16UI,			GL_RED_INTEGER,			GL_UNSIGNED_SHORT,	2,	true,	{"GL_ARB_texture_rg",	"GL_EXT_texture_integer",	NULL}},
	{GL_R32UI,			GL_RED_INTEGER,			GL_UNSIGNED_INT,	4,	true,	{"GL_ARB_texture_rg",	"GL_EXT_texture_integer",	NULL}},

	{GL_RG8,			GL_RG,				GL_UNSIGNED_BYTE,	2,	true,	{"GL_ARB_texture_rg",	NULL,				NULL}},
	{GL_RG16,			GL_RG,				GL_UNSIGNED_SHORT,	4,	true,	{"GL_ARB_texture_rg",	NULL,				NULL}},
	{GL_RG16F,			GL_RG,				GL_HALF_FLOAT,		4,	true,	{"GL_ARB_texture_rg",	"GL_ARB_texture_float",		"GL_ARB_half_float_pixel"}},
	{GL_RG32F,			GL_RG,				GL_FLOAT,		8,	true,	{"GL_ARB_texture_rg",	"GL_ARB_texture_float",		NULL}},
	{GL_RG8I,			GL_RG_INTEGER,			GL_BYTE,		2,	true,	{"GL_ARB_texture_rg",	"GL_EXT_texture_integer",	NULL}},
	{GL_RG16I,			GL_RG_INTEGER,			GL_SHORT,		4,	true,	{"GL_ARB_texture_rg",	"GL_EXT_texture_integer",	NULL}},
	{GL_RG32I,			GL_RG_INTEGER,			GL_INT,			8,	true,	{"GL_ARB_texture_rg",	"GL_EXT_texture_integer",	NULL}},
	{GL_RG8UI,			GL_RG_INTEGER,			GL_UNSIGNED_BYTE,	2,	true,	{"GL_ARB_texture_rg",	"GL_EXT_texture_integer",	NULL}},
	{GL_RG16UI,			GL_RG_INTEGER,			GL_UNSIGNED_SHORT,	4,	true,	{"GL_ARB_texture_rg",	"GL_EXT_texture_integer",	NULL}},
	{GL_RG32UI,			GL_RG_INTEGER,			GL_UNSIGNED_INT,	8,	true,	{"GL_ARB_texture_rg",	"GL_EXT_texture_integer",	NULL}},
};


static bool
test_format(const int i)
{
	static const char *const data_7f   = "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f"
					     "\x7f\x7f\x7f\x7f";
	static const char *const data_init = "\xff\xff\xff\xff"
					     "\xff\xff\xff\xff"
					     "\x00\x00\x00\x00"
					     "\x00\x00\x00\x00"
					     "\x55\x55\x55\x55"
					     "\x55\x55\x55\x55"
					     "\xaa\xaa\xaa\xaa"
					     "\xaa\xaa\xaa\xaa"
					     "\xff\x00\xff\x00"
					     "\xff\x00\xff\x00"
					     "\x00\xff\x00\xff"
					     "\x00\xff\x00\xff"
					     "\x91\xcc\x45\x36"
					     "\xd3\xe4\xe3\x5b"
					     "\x79\x1e\x21\x39"
					     "\xa8\xfa\x69\x6a";
	int j;


	if (piglit_is_core_profile && !formats[i].core_profile)
		return true;

	for (j = 0; j < 3; ++j)
		if (formats[i].ext[j])
			if (!piglit_is_extension_supported(
						formats[i].ext[j]))
				return true;

	printf("Testing %s... ",
			piglit_get_gl_enum_name(formats[i].internal_format));
	fill_array_buffer(64, data_init);
	glClearBufferData(GL_ARRAY_BUFFER, formats[i].internal_format,
			formats[i].format, formats[i].type, data_7f);

	if (!piglit_check_gl_error(GL_NO_ERROR) ||
			!check_array_buffer_data(formats[i].size, data_7f)) {
		printf("Failed!\n");
		return false;
	}
	printf("Passed.\n");
	return true;
}


void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	int i;
	const int buffer_size = 3<<20;
	unsigned int buffer;

	piglit_require_extension("GL_ARB_clear_buffer_object");

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STREAM_READ);

	for (i = 0; i < ARRAY_SIZE(formats); ++i)
		pass = test_format(i) && pass;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &buffer);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

