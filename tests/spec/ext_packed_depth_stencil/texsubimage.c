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

/** @file texsubimage.c
 *
 * A test of using glTexSubImage2D to update a region of a
 * depth-stencil texture. A 4x4 depth-stencil is created and then two
 * of the texels are set using different values. The whole texture is
 * read back using glGetTexImage and compared to the expected values.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint
create_texture(void)
{
	static const GLubyte data[] = {
		0xff, 0xff, 0xff, 0xff,
		0x04, 0x05, 0x06, 0x07,
		0xff, 0xff, 0xff, 0xff,
		0x0c, 0x0d, 0x0e, 0x0f,
	};
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D,
		     0, /* level */
		     GL_DEPTH24_STENCIL8_EXT,
		     2, 2, /* width/height */
		     0, /* border */
		     GL_DEPTH_STENCIL_EXT,
		     GL_UNSIGNED_INT_24_8_EXT,
		     data);

	return tex;
}

static void
update_texture(void)
{
	static const GLubyte bottom_left_pixel[] = {
		0x00, 0x01, 0x02, 0x03
	};
	static const GLubyte top_left_pixel[] = {
		0x08, 0x09, 0x0a, 0x0b
	};
	glTexSubImage2D(GL_TEXTURE_2D,
			0, /* level */
			0, 0, /* x/y */
			1, 1, /* width/height */
			GL_DEPTH_STENCIL_EXT,
			GL_UNSIGNED_INT_24_8_EXT,
			bottom_left_pixel);
	glTexSubImage2D(GL_TEXTURE_2D,
			0, /* level */
			0, 1, /* x/y */
			1, 1, /* width/height */
			GL_DEPTH_STENCIL_EXT,
			GL_UNSIGNED_INT_24_8_EXT,
			top_left_pixel);
}

static bool
check_texels(void)
{
	GLubyte texels[2 * 2 * 4];
	int i;

	glGetTexImage(GL_TEXTURE_2D,
		      0, /* level */
		      GL_DEPTH_STENCIL_EXT,
		      GL_UNSIGNED_INT_24_8_EXT,
		      texels);

	for (i = 0; i < sizeof texels; i++)
		if (texels[i] != i)
			return false;

	return true;
}

void
piglit_init(int argc, char **argv)
{
	GLuint tex;
	bool pass;

        /* We can create depth/stencil textures if either:
         * 1. We have GL 3.0 or later
         * 2. We have GL_EXT_packed_depth_stencil and GL_ARB_depth_texture
         */
	if (piglit_get_gl_version() < 30
	    && !(piglit_is_extension_supported("GL_EXT_packed_depth_stencil") &&
		 piglit_is_extension_supported("GL_ARB_depth_texture"))) {
		printf("OpenGL 3.0 or GL_EXT_packed_depth_stencil + "
		       "GL_ARB_depth_texture is required.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	tex = create_texture();

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	update_texture();

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glBindTexture(GL_TEXTURE_2D, tex);

	pass = check_texels();

	glBindTexture(GL_TEXTURE_2D, 0);

	glDeleteTextures(1, &tex);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* unused */
	return PIGLIT_FAIL;
}
