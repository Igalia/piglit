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

#include "common.h"

#define TEX_WIDTH 64
#define TEX_HEIGHT 256

#define ZERO_CLEAR_X 10
#define ZERO_CLEAR_Y 15
#define ZERO_CLEAR_WIDTH 8
#define ZERO_CLEAR_HEIGHT 12

#define VALUE_CLEAR_X 30
#define VALUE_CLEAR_Y 50
#define VALUE_CLEAR_WIDTH 9
#define VALUE_CLEAR_HEIGHT 13

/* Arbitrary values that is big enough for four doubles */
static const GLubyte clearValue[] = {
	0x1f, 0x1e, 0x1d, 0x1c,
	0x1b, 0x1a, 0x19, 0x18,
	0x17, 0x16, 0x15, 0x14,
	0x13, 0x12, 0x11, 0x10,
	0x0f, 0x0e, 0x0d, 0x0c,
	0x0b, 0x0a, 0x09, 0x08,
	0x07, 0x06, 0x05, 0x04,
	0x03, 0x02, 0x01, 0x00,
};

static GLuint
create_texture(GLenum internalFormat,
	       GLenum format,
	       GLenum type,
	       GLsizei texelSize)
{
	GLubyte *data, *p;
	GLuint tex;
	int i;

	data = malloc(TEX_WIDTH * TEX_HEIGHT * texelSize);
	p = data;

	/* Fill the data with increasing bytes */
	for (i = 0; i < TEX_WIDTH * TEX_HEIGHT * texelSize; i++)
		*(p++) = i;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D,
		     0, /* level */
		     internalFormat,
		     TEX_WIDTH, TEX_HEIGHT,
		     0, /* border */
		     format, type,
		     data);

	free(data);

	return tex;
}

static void
clear_texture(GLuint tex, GLenum format, GLenum type, GLsizei texelSize)
{
	/* Clear one region using a NULL (all zeroes) value */
	glClearTexSubImage(tex,
			   0, /* level */
			   ZERO_CLEAR_X,
			   ZERO_CLEAR_Y,
			   0, /* z */
			   ZERO_CLEAR_WIDTH,
			   ZERO_CLEAR_HEIGHT,
			   1, /* depth */
			   format,
			   type,
			   NULL /* value */);

	/* Clear another region to a known value */
	glClearTexSubImage(tex,
			   0, /* level */
			   VALUE_CLEAR_X,
			   VALUE_CLEAR_Y,
			   0, /* z */
			   VALUE_CLEAR_WIDTH,
			   VALUE_CLEAR_HEIGHT,
			   1, /* depth */
			   format,
			   type,
			   clearValue);
}

static bool
is_value_clear(const GLubyte *texel, GLsizei texelSize)
{
	int i;

	for (i = 0; i < texelSize; i++)
		if (texel[i] != clearValue[i])
			return false;

	return true;
}

static bool
is_zero_clear(const GLubyte *texel, GLsizei texelSize)
{
	int i;

	for (i = 0; i < texelSize; i++)
		if (texel[i] != 0)
			return false;

	return true;
}

static bool
check_texels(GLenum format, GLenum type, GLsizei texelSize)
{
	GLubyte *data, *p;
	bool success = true;
	int x, y, b;

	data = malloc(TEX_WIDTH * TEX_HEIGHT * texelSize);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glGetTexImage(GL_TEXTURE_2D,
		      0, /* level */
		      format, type,
		      data);

	p = data;

	for (y = 0; y < TEX_HEIGHT; y++) {
		for (x = 0; x < TEX_WIDTH; x++) {
			if (x >= VALUE_CLEAR_X &&
			    x < VALUE_CLEAR_X + VALUE_CLEAR_WIDTH &&
			    y >= VALUE_CLEAR_Y &&
			    y < VALUE_CLEAR_Y + VALUE_CLEAR_HEIGHT) {
				if (!is_value_clear(p, texelSize))
					success = false;
			} else if (x >= ZERO_CLEAR_X &&
				   x < ZERO_CLEAR_X + ZERO_CLEAR_WIDTH &&
				   y >= ZERO_CLEAR_Y &&
				   y < ZERO_CLEAR_Y + ZERO_CLEAR_HEIGHT) {
				if (!is_zero_clear(p, texelSize))
					success = false;
			} else {
				for (b = 0; b < texelSize; b++)
					if (p[b] != ((p + b - data) & 0xff))
						success = false;
			}

			p += texelSize;
		}
	}

	free(data);

	return success;
}

bool
test_format(GLenum internalFormat,
	    GLenum format,
	    GLenum type,
	    GLsizei texelSize)
{
	GLuint tex;
	bool pass;

	/* glClearTexture is either in the GL_ARB_clear_texture
	 * extension or in core in GL 4.4
	 */
	if (piglit_get_gl_version() < 44 &&
	    !piglit_is_extension_supported("GL_ARB_clear_texture")) {
		printf("OpenGL 4.4 or GL_ARB_clear_texture is required.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	tex = create_texture(internalFormat, format, type, texelSize);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	clear_texture(tex, format, type, texelSize);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	glBindTexture(GL_TEXTURE_2D, tex);

	pass = check_texels(format, type, texelSize);

	glBindTexture(GL_TEXTURE_2D, 0);

	glDeleteTextures(1, &tex);

	return pass;
}

bool
test_formats(const struct format *formats,
	     int n_formats)
{
	bool overallResult = true;
	bool pass;
	int i;

	for (i = 0; i < n_formats; i++) {
		const struct format *format = formats + i;

		pass = test_format(format->internalFormat,
				   format->format,
				   format->type,
				   format->texelSize);

		printf("internalFormat = %s, format = %s, type = %s : %s\n",
		       piglit_get_gl_enum_name(format->internalFormat),
		       piglit_get_gl_enum_name(format->format),
		       piglit_get_gl_enum_name(format->type),
		       pass ? "pass" : "fail");

		overallResult &= pass;
	}

	return overallResult;
}

bool
test_invalid_format(GLenum internalFormat,
		    GLenum texImageFormat,
		    GLenum texImageType,
		    GLenum clearValueFormat,
		    GLenum clearValueType)
{
	static const GLubyte dummy_data[sizeof (float) * 4];
	bool pass = true;
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D,
		     0, /* level */
		     internalFormat,
		     1, 1, /* width/height */
		     0, /* border */
		     texImageFormat,
		     texImageType,
		     dummy_data);

	pass &= piglit_check_gl_error(GL_NO_ERROR);

	glClearTexImage(tex, 0, clearValueFormat, clearValueType, dummy_data);

	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &tex);

	return pass;
}
