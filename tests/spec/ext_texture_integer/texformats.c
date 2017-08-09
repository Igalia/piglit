/*
 * Copyright (c) 2015 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL VMWARE AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Test glTexImage2D and glGetTexImage with a variety of combinations of
 * internal formats, and user-specified formats/types.
 *
 * Brian Paul
 * 31 August 2015
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


static bool
test_format(GLenum intFormat, GLenum format, GLenum type)
{
	const int width = 8, height = 8;
	GLuint tex;
	GLubyte *image, *getimage;
	bool pass = true;
	int i, bytes;

	switch (intFormat) {
	case GL_ALPHA8I_EXT:
	case GL_ALPHA8UI_EXT:
		bytes = 1;
		break;
	case GL_ALPHA16I_EXT:
	case GL_ALPHA16UI_EXT:
		bytes = 2;
		break;
	case GL_ALPHA32I_EXT:
	case GL_ALPHA32UI_EXT:
		bytes = 4;
		break;
	case GL_RGB8I:
	case GL_RGB8UI:
		bytes = 3;
		break;
	case GL_RGBA8I:
	case GL_RGBA8UI:
		bytes = 4;
		break;
	case GL_RGB16I:
	case GL_RGB16UI:
		bytes = 6;
		break;
	case GL_RGBA16I:
	case GL_RGBA16UI:
		bytes = 8;
		break;
	case GL_RGB32I:
	case GL_RGB32UI:
		bytes = 12;
		break;
	case GL_RGBA32I:
	case GL_RGBA32UI:
		bytes = 16;
		break;
	default:
		assert(!"Unexpected format");
		bytes = 0;
	}

	image = malloc(width * height * bytes);
	for (i = 0; i < width * height * bytes; i++) {
		image[i] = i & 255;
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, intFormat, width, height, 0,
		     format, type, image);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		pass = false;
	}

	getimage = malloc(width * height * bytes);

	glGetTexImage(GL_TEXTURE_2D, 0, format, type, getimage);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		pass = false;
	}

	if (memcmp(image, getimage, width * height * bytes) != 0) {
		pass = false;
	}

	if (!pass) {
		printf("Fail for intFormat=%s, format=%s, type=%s\n",
		       piglit_get_gl_enum_name(intFormat),
		       piglit_get_gl_enum_name(format),
		       piglit_get_gl_enum_name(type));
	}

	glDeleteTextures(1, &tex);

	return pass;
}


enum piglit_result
piglit_display(void)
{
	/* nothing */
	return PIGLIT_SKIP;
}


void
piglit_init(int argc, char **argv)
{
	/* These format combinations should all work.  But this list is
	 * not exhaustive.
	 */
	static const GLenum formats[][3] = {
		/* 8-bit/channel */
		{ GL_ALPHA8UI_EXT, GL_ALPHA_INTEGER, GL_UNSIGNED_BYTE },
		{ GL_ALPHA8I_EXT, GL_ALPHA_INTEGER, GL_BYTE },
		{ GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE },
		{ GL_RGB8I, GL_RGB_INTEGER, GL_BYTE },
		{ GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE },
		{ GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE },
		{ GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT_8_8_8_8 },
		{ GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT_8_8_8_8_REV },
		/* 16-bit */
		{ GL_ALPHA16UI_EXT, GL_ALPHA_INTEGER, GL_UNSIGNED_SHORT },
		{ GL_ALPHA16I_EXT, GL_ALPHA_INTEGER, GL_SHORT },
		{ GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_SHORT },
		{ GL_RGB16I, GL_RGB_INTEGER, GL_SHORT },
		{ GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT },
		{ GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT },
		/* 32-bit */
		{ GL_ALPHA32UI_EXT, GL_ALPHA_INTEGER, GL_UNSIGNED_INT },
		{ GL_ALPHA32I_EXT, GL_ALPHA_INTEGER, GL_INT },
		{ GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT },
		{ GL_RGB32I, GL_RGB_INTEGER, GL_INT },
		{ GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT },
		{ GL_RGBA32I, GL_RGBA_INTEGER, GL_INT }
	};
	int i;
	bool pass = true;

	piglit_require_extension("GL_EXT_texture_integer");

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (i = 0; i < ARRAY_SIZE(formats); i++) {
		pass = test_format(formats[i][0], formats[i][1], formats[i][2])
			&& pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
