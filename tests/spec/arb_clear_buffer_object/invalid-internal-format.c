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
 * \file invalid-internal-format.c
 *
 * From the GL_ARB_clear_buffer_object spec:
 * "<internalformat> must be set to one of the format tokens listed in
 *  Table 3.15, "Internal formats for buffer textures"."
 *
 * This table only includes a subset of available internal formats. In
 * particular, the table does not include:
 * -- unsized formats (e.g.: GL_RGBA)
 * -- depth or stencil formats
 * -- compressed formats
 * -- formats with a component with a bitwidth that is not a multiple of 8
 *    (e.g.: GL_RGB5_A1).
 * -- formats with a total bitwidth that is not a multiple of 32
 *    (e.g.: GL_RGB8).
 *
 * Test that the required GL_INVALID_ENUM error is generated for these formats.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 15;

PIGLIT_GL_TEST_CONFIG_END


static const struct {
	GLenum internal_format;
	GLenum format;
	GLenum type;
} formats[] = {
	/* legacy OpenGL 1.0 "formats" */
	{1, GL_ALPHA, GL_UNSIGNED_BYTE},
	{2, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE},
	{3, GL_RGB, GL_UNSIGNED_BYTE},
	{4, GL_RGBA, GL_UNSIGNED_BYTE},

	/* unsized formats */
	{GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE},
	{GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE},
	{GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE},
	{GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE},
	{GL_INTENSITY, GL_INTENSITY, GL_UNSIGNED_BYTE},
	{GL_RGB, GL_RGB, GL_UNSIGNED_BYTE},
	{GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE},

	/* depth formats */
	{GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT},
	{GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT},
	{GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT},

	/* component not multiple of 8 bit wide */
	{GL_ALPHA4, GL_ALPHA, GL_UNSIGNED_BYTE},
	{GL_ALPHA12, GL_ALPHA, GL_UNSIGNED_SHORT},
	{GL_LUMINANCE4, GL_LUMINANCE, GL_UNSIGNED_BYTE},
	{GL_LUMINANCE12, GL_LUMINANCE, GL_UNSIGNED_SHORT},
	{GL_LUMINANCE12_ALPHA4, GL_LUMINANCE_ALPHA, GL_UNSIGNED_INT},
	{GL_LUMINANCE12_ALPHA12, GL_LUMINANCE_ALPHA, GL_UNSIGNED_INT},
	{GL_INTENSITY4, GL_INTENSITY, GL_UNSIGNED_BYTE},
	{GL_INTENSITY12, GL_INTENSITY, GL_UNSIGNED_SHORT},
	{GL_R3_G3_B2, GL_RGB, GL_UNSIGNED_BYTE_3_3_2},
	{GL_RGB4, GL_RGB, GL_UNSIGNED_SHORT_4_4_4_4},
	{GL_RGB5, GL_RGB, GL_UNSIGNED_SHORT_5_5_5_1},
	{GL_RGB10, GL_RGB, GL_UNSIGNED_INT_10_10_10_2},
	{GL_RGB12, GL_RGB, GL_UNSIGNED_SHORT},
	{GL_RGBA2, GL_RGBA, GL_UNSIGNED_BYTE},
	{GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4},
	{GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1},
	{GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_10_10_10_2},
	{GL_RGBA12, GL_RGBA, GL_UNSIGNED_SHORT},

	/* format not multiple of 32 bit wide */
	{GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE},

	/* compressed formats */
	{GL_COMPRESSED_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE},
	{GL_COMPRESSED_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE},
	{GL_COMPRESSED_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE},
	{GL_COMPRESSED_INTENSITY, GL_INTENSITY, GL_UNSIGNED_BYTE},
	{GL_COMPRESSED_RGB, GL_RGB, GL_UNSIGNED_BYTE},
	{GL_COMPRESSED_RGBA, GL_RGBA, GL_UNSIGNED_BYTE},
};


static bool
test_format(const int i)
{
	printf("Testing %s... ",
			piglit_get_gl_enum_name(formats[i].internal_format));

	glClearBufferData(GL_ARRAY_BUFFER, formats[i].internal_format,
			formats[i].format, formats[i].type, NULL);

	if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
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

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	for (i = 0; i < ARRAY_SIZE(formats); ++i)
		test_format(i);

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

