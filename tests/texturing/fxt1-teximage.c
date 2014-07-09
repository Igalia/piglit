/*
 * Copyright © 2009 Intel Corporation
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
 *    Chris Lord <chris@openedhand.com>
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file fxt1-teximage.c
 *
 * Tests that a full 3DFX FXT1-compressed mipmap tree can be created
 * and used.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 300;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define SIZE 128

const float red[4] =   {1.0, 0.0, 0.0, 1.0};
const float green[4] = {0.0, 1.0, 0.0, 1.0};
const float blue[4] =  {0.0, 0.0, 1.0, 1.0};
const float white[4] = {1.0, 1.0, 1.0, 1.0};

static void
display_mipmaps(int start_x, int start_y)
{
	int i;

	glEnable(GL_TEXTURE_2D);

	/* Disply all the mipmap levels */
	for (i = SIZE; i > 0; i /= 2) {
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(start_x + 0, start_y + 0);
		glTexCoord2f(1.0, 0.0); glVertex2f(start_x + i, start_y + 0);
		glTexCoord2f(1.0, 1.0); glVertex2f(start_x + i, start_y + i);
		glTexCoord2f(0.0, 1.0); glVertex2f(start_x + 0, start_y + i);
		glEnd();

		start_x += i + 5;
	}
}


static GLboolean
check_resulting_mipmaps(int x, int y)
{
	GLboolean pass = GL_TRUE;
	int size;

	for (size = SIZE; size > 0; size /= 2) {
		if (size == 4)
			pass = pass && piglit_probe_pixel_rgb(x + 2, y + 2,
							      red);
		else if (size == 2)
			pass = pass && piglit_probe_pixel_rgb(x + 1, y + 1,
							      green);
		else if (size == 1)
			pass = pass && piglit_probe_pixel_rgb(x, y,
							      blue);
		else {
			pass = pass && piglit_probe_pixel_rgb(x + size / 4,
							      y + size / 4,
							      red);
			pass = pass && piglit_probe_pixel_rgb(x + size * 3 / 4,
							      y + size / 4,
							      green);
			pass = pass && piglit_probe_pixel_rgb(x + size / 4,
							      y + size * 3 / 4,
							      blue);
			pass = pass && piglit_probe_pixel_rgb(x + size * 3 / 4,
							      y + size * 3 / 4,
							      white);
		}
		x += size + 5;
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLuint tex;
	GLboolean pass = GL_TRUE;

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = piglit_rgbw_texture(GL_COMPRESSED_RGB_FXT1_3DFX, SIZE, SIZE,
				  GL_TRUE, GL_FALSE, GL_UNSIGNED_NORMALIZED);
	display_mipmaps(10, 10 + (10 + SIZE) * 0);
	glDeleteTextures(1, &tex);
	tex = piglit_rgbw_texture(GL_COMPRESSED_RGBA_FXT1_3DFX, SIZE, SIZE,
				  GL_TRUE, GL_FALSE, GL_UNSIGNED_NORMALIZED);
	display_mipmaps(10, 10 + (10 + SIZE) * 1);
	glDeleteTextures(1, &tex);

	pass = pass && check_resulting_mipmaps(10, 10 + (10 + SIZE) * 0);
	pass = pass && check_resulting_mipmaps(10, 10 + (10 + SIZE) * 1);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_3DFX_texture_compression_FXT1");

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
