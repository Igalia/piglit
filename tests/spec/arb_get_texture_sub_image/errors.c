/*
 * Copyright 2015 VMware, Inc.
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
 * Test glGetTextureSubImage and glGetCompressedTextureSubImage error checking.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


static bool
test_texture_id(void)
{
	GLubyte buffer[8*8*4];
	GLuint tex = 42;
	bool pass = true;

	/* Test get with bad texture ID */
	glGetTextureSubImage(tex, 0,
			     0, 0, 0, /* offset */
			     8, 8, 1, /* size */
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     sizeof(buffer), buffer);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;

	/* Test compressed get with bad texture ID */
	glGetCompressedTextureSubImage(tex, 0,
				       0, 0, 0, /* offset */
				       8, 8, 1, /* size */
				       sizeof(buffer), buffer);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;

	/* Test get with undefined texture */
	glGenTextures(1, &tex);
	glGetTextureSubImage(tex, 0,
			     0, 0, 0, /* offset */
			     8, 8, 1, /* size */
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     sizeof(buffer), buffer);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;

	/* Test compressed get with undefined texture */
	glGetCompressedTextureSubImage(tex, 0,
				       0, 0, 0, /* offset */
				       8, 8, 1, /* size */
				       sizeof(buffer), buffer);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;

	glDeleteTextures(1, &tex);

	return pass;
}


static bool
test_buffer_size(void)
{
	GLubyte buffer[8*8*4];
	GLubyte quadrant_buffer[4*4*4];
	GLuint tex = 42;
	bool pass = true;

	/* setup 8x8 texture */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, 8, 8);

	/* Test too small of dest buffer */
	glGetTextureSubImage(tex, 0,
			     0, 0, 0, /* offset */
			     8, 8, 1, /* size */
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     sizeof(buffer) - 1, buffer);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;

	/* Test with pixel unpack params, sufficient buffer size */
	glPixelStorei(GL_PACK_SKIP_ROWS, 4);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 4);
	glPixelStorei(GL_PACK_ROW_LENGTH, 8);
	glGetTextureSubImage(tex, 0,
			     4, 4, 0, /* offset */
			     4, 4, 1, /* size */
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     sizeof(buffer), buffer);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	/* Test with pixel unpack params, insufficient buffer size */
	glGetTextureSubImage(tex, 0,
			     4, 4, 0, /* offset */
			     4, 4, 1, /* size */
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     sizeof(buffer) - 1, buffer);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;

	/* Test getting a quadrant, sufficent buffer size */
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glGetTextureSubImage(tex, 0,
			     4, 4, 0, /* offset */
			     4, 4, 1, /* size */
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     sizeof(quadrant_buffer), quadrant_buffer);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	/* Test getting a quadrant, insufficent buffer size */
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glGetTextureSubImage(tex, 0,
			     4, 4, 0, /* offset */
			     4, 4, 1, /* size */
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     sizeof(quadrant_buffer) - 1, quadrant_buffer);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;

	glDeleteTextures(1, &tex);

	return pass;
}


static bool
test_invalid_values(void)
{
	GLubyte buffer[8*8*4];
	GLuint tex = 42;
	bool pass = true;

	/* setup 8x8 texture */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, 8, 8);

	/* Test bad format/type */
	glGetTextureSubImage(tex, 0,
			     0, 0, 0, /* offset */
			     8, 8, 1, /* size */
			     GL_RGBA, GL_DEPTH_FUNC,  /* bad enum */
			     sizeof(buffer), buffer);
	if (!piglit_check_gl_error(GL_INVALID_ENUM))
		pass = false;

	/* Test getting invalid negative level */
	glGetTextureSubImage(tex, -1,
			     0, 0, 0, /* offset */
			     1, 1, 1, /* size */
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     sizeof(buffer), buffer);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	/* Test getting invalid large level */
	glGetTextureSubImage(tex, 99,
			     0, 0, 0, /* offset */
			     1, 1, 1, /* size */
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     sizeof(buffer), buffer);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	/* Test non-existant level */
	glGetTextureSubImage(tex, 4,
			     0, 0, 0, /* offset */
			     8, 8, 1, /* size */
			     GL_RGBA, GL_FLOAT,  /* bad enum */
			     sizeof(buffer), buffer);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	/* Test getting invalid offset */
	glGetTextureSubImage(tex, 0,
			     -1, 0, 0, /* offset */
			     1, 1, 1, /* size */
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     sizeof(buffer), buffer);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	/* Test getting invalid size */
	glGetTextureSubImage(tex, 0,
			     0, 0, 0, /* offset */
			     -1, 1, 1, /* size */
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     sizeof(buffer), buffer);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	/* Test getting zero size - not an error */
	glGetTextureSubImage(tex, 0,
			     0, 0, 0, /* offset */
			     0, 1, 1, /* size */
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     sizeof(buffer), buffer);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	glDeleteTextures(1, &tex);

	return pass;
}


static bool
test_cubemap_faces(void)
{
	GLubyte results[8*8*6*4];
	GLuint tex = 42;
	bool pass = true;
        int face;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

        /* create 5 cube faces, purposely omitting 6th face */
        for (face = 0; face < 5; face++) {
           glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                        0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_FLOAT, NULL);
        }

        /* try to get all six cube faces, should fail */
        glGetTextureSubImage(tex, 0,
                             0, 0, 0,
                             8, 8, 6,
                             GL_RGBA, GL_UNSIGNED_BYTE,
                             sizeof(results), results);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;

        /* try to get five cube faces, should pass */
        glGetTextureSubImage(tex, 0,
                             0, 0, 0,
                             8, 8, 5,
                             GL_RGBA, GL_UNSIGNED_BYTE,
                             sizeof(results), results);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	glDeleteTextures(1, &tex);

        return pass;
}


static bool
test_zero_size_image(void)
{
	GLubyte image[8*8*4];
	GLuint tex;
	bool pass = true;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D,
		     0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	/* getting 0x0 image from 8x8 source should work */
	glGetTextureSubImage(tex, 0,
			     0, 0, 0,
			     0, 0, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     sizeof(image), image);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	/* replace image with 0x0 image (deallocates old one) */
	glTexImage2D(GL_TEXTURE_2D,
		     0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	/* getting 0x0 image from 0x0 source should work */
	glGetTextureSubImage(tex, 0,
			     0, 0, 0,
			     0, 0, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     sizeof(image), image);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	/* getting 0x0 image at an offset from 0x0 source should error */
	glGetTextureSubImage(tex, 0,
			     1, 2, 0,  /* offset */
			     0, 0, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     sizeof(image), image);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	/* getting 2x2 image from 0x0 source should generate error */
	glGetTextureSubImage(tex, 0,
			     0, 0, 0,
			     2, 2, 1,
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     sizeof(image), image);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	glDeleteTextures(1, &tex);

	return pass;
}


void
piglit_init(int argc, char **argv)
{
	bool pass;

	piglit_require_extension("GL_ARB_get_texture_sub_image");
	piglit_require_extension("GL_ARB_texture_storage");

	pass = test_texture_id();
	pass = test_buffer_size() && pass;
	pass = test_invalid_values() && pass;
	pass = test_cubemap_faces() && pass;
	pass = test_zero_size_image() && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* never called */
	return PIGLIT_PASS;
}


