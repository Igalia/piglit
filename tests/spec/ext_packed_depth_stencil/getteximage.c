/*
 * Copyright © 2014 VMware, Inc.
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
 * \file getteximage.c
 *
 * Test glGetTexImage with packed depth/stencil formats.
 *
 * This exercises a bug in Mesa where we failed to do proper texel
 * conversion for depth/stencil values in glGetTexImage.  But, the bug
 * would only appear depending on whether the driver stores depth/stencil
 * textures as z24s8 versus s8z24.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 12;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


#define WIDTH 16
#define HEIGHT 16


static bool
test_z24_s8(void)
{
	GLuint tex[WIDTH * HEIGHT];
	GLuint buf[WIDTH * HEIGHT];
	GLuint i;

	/* init tex data */
	for (i = 0; i < WIDTH * HEIGHT; i++) {
		GLuint s = 255 - (i & 255);
		GLuint z = i * 100;
		tex[i] = (z << 24) | s;
	}

	/* create texture */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8,
		     WIDTH, HEIGHT, 0,
		     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, tex);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* read back the texture */
	glGetTexImage(GL_TEXTURE_2D, 0,
		      GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, buf);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* compare */
	for (i = 0; i < WIDTH * HEIGHT; i++) {
		if (buf[i] != tex[i]) {
			printf("Wrong texel data at position %d: "
			       "Expected 0x%08x, found 0x%08x\n",
			       i, tex[i], buf[i]);
			return false;
		}
	}

	return true;
}


static bool
test_z32_s8(void)
{
	const double epsilon = 2.0 / (float) 0xffffff;  /* 2-bit error */
	GLuint tex[2 * WIDTH * HEIGHT];
	GLuint buf[2 * WIDTH * HEIGHT];
	GLuint i;
	GLfloat *ftex = (GLfloat *) tex;
	GLfloat *fbuf = (GLfloat *) buf;

	/* init tex data */
	for (i = 0; i < WIDTH * HEIGHT; i++) {
		GLuint s = 255 - (i & 255);
		GLfloat z = (float) i / (float) (WIDTH * HEIGHT - 1);
		ftex[i*2+0] = z;
		tex[i*2+1] = s;
	}

	/* create texture */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8,
		     WIDTH, HEIGHT, 0,
		     GL_DEPTH_STENCIL,
		     GL_FLOAT_32_UNSIGNED_INT_24_8_REV, tex);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* read back the texture as float32/stencil8 */
	glGetTexImage(GL_TEXTURE_2D, 0,
		      GL_DEPTH_STENCIL,
		      GL_FLOAT_32_UNSIGNED_INT_24_8_REV, buf);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* compare */
	for (i = 0; i < WIDTH * HEIGHT; i++) {
		if (fbuf[i*2+0] != ftex[i*2+0]) {
			printf("Wrong depth data at position %d: "
			       "Expected %g, found %g\n",
			       i, ftex[i*2+0], fbuf[i*2+0]);
			return false;
		}
		if (buf[i*2+1] != tex[i*2+1]) {
			printf("Wrong stencil data at position %d: "
			       "Expected 0x%08x, found 0x%08x\n",
			       i, tex[i*2+1], buf[i*2+1]);
			return false;
		}
	}

	/* read back the texture as depth24/stencil8 */
	glGetTexImage(GL_TEXTURE_2D, 0,
		      GL_DEPTH_STENCIL,
		      GL_UNSIGNED_INT_24_8, buf);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* compare */
	for (i = 0; i < WIDTH * HEIGHT; i++) {
		GLuint s = buf[i] & 0xff;
		GLfloat z = (buf[i] >> 8) / (float) 0xffffff;
		if (fabs(z - ftex[i*2+0]) > epsilon) {
			printf("Wrong depth data at position %d: "
			       "Expected %g, found %g\n",
			       i, ftex[i*2+0], z);
			return false;
		}
		if (s != tex[i*2+1]) {
			printf("Wrong stencil data at position %d: "
			       "Expected 0x%02x, found 0x%02x\n",
			       i, tex[i*2+1], s);
			return false;
		}
	}

	return true;
}


void
piglit_init(int argc, char **argv)
{
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

	pass = test_z24_s8();

	if (piglit_get_gl_version() >= 30 ||
	    piglit_is_extension_supported("GL_ARB_depth_buffer_float")) {
		pass = test_z32_s8() && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* unused */
	return PIGLIT_FAIL;
}
