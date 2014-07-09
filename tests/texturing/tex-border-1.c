/*
 * Copyright © 2010 Intel Corporation
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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file tex-border-1.c
 *
 * Tests that the texture border color on a GL_RGBA texture is sampled
 * correctly.
 *
 * This is intended to be the first test in a series.  Other tests
 * that could be used are behavior of sampling texture border color
 * for GL_RGB textures, and sampling the border color depending on the
 * texture format (gen5 Intel hardware and up stores format-dependent
 * border colors.).
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	float black[4] = {0.0, 0.0, 0.0, 0.0};
	float white[4] = {1.0, 1.0, 1.0, 0.0};
	float red[4] =   {1.0, 0.0, 0.0, 0.0};
	float green[4] = {0.0, 1.0, 0.0, 0.0};
	float blue[4] =  {0.0, 0.0, 1.0, 0.0};
	GLboolean pass = GL_TRUE;
	GLuint tex;

	tex = piglit_checkerboard_texture(0, /* name */
					  0, /* level */
					  2, 2, /* width/height */
					  1, 1, /* checkerboard size */
					  black, white);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, red);
	piglit_draw_rect_tex(-1, -1, 1, 1,
			     -2, -2, 0, 0);

	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, green);
	piglit_draw_rect_tex( 0, -1, 1, 1,
			     -2, -2, 0, 0);

	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, blue);
	piglit_draw_rect_tex(-1,  0, 1, 1,
			     -2, -2, 0, 0);

	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, white);
	piglit_draw_rect_tex( 0,  0, 1, 1,
			     -2, -2, 0, 0);

	pass = piglit_probe_pixel_rgb(piglit_width * 1 / 4,
				      piglit_height * 1 / 4, red) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width * 3 / 4,
				      piglit_height * 1 / 4, green) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width * 1 / 4,
				      piglit_height * 3 / 4, blue) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width * 3 / 4,
				      piglit_height * 3 / 4, white) && pass;

	glDeleteTextures(1, &tex);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(13);
}
