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
 * Test glGetTexImage with stencil formats.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 12;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


#define WIDTH 16
#define HEIGHT 16

static bool
test_s8(void)
{
	GLubyte tex[WIDTH * HEIGHT];
	GLubyte buf[WIDTH * HEIGHT];
	GLuint i;

	/* init tex data */
	for (i = 0; i < WIDTH * HEIGHT; i++) {
		GLuint s = 255 - (i & 255);
		tex[i] = s;
	}

	/* create texture */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX8,
		     WIDTH, HEIGHT, 0,
		     GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, tex);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* read back the texture */
	glGetTexImage(GL_TEXTURE_2D, 0,
		      GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, buf);
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

void
piglit_init(int argc, char **argv)
{
	bool pass;

	piglit_require_extension("GL_ARB_texture_stencil8");

	pass = test_s8();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* unused */
	return PIGLIT_FAIL;
}
