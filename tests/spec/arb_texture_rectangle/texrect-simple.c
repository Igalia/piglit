/* Copyright © 2013 Linaro Inc
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
 * Ported from glean ttexrect.cpp into piglit.
 * Test the ARB_texture_rectangle extension
 *   Create a 255x127 texture of varying colors and bind it as a
 *   GL_ARB_texture_recangle target.  Draw that rectangle to the window, and
 *   check that the texture was drawn correctly.  The common failure to be
 *   caught with this test is not adjusting the non-normalized coordinates on
 *   hardware that expects normalized coordinates.
 * \author: Eric Anholt <eric@anholt.net> (original)
 * \author: Tom Gall <tom.gall@linaro.org> (port)
 */
#include "piglit-util-gl.h"

#define TEXTURE_WIDTH   255
#define TEXTURE_HEIGHT  127
#define WINDOW_SIZE     256

float image[TEXTURE_WIDTH * TEXTURE_HEIGHT * 3];

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_width = WINDOW_SIZE;
	config.window_height = WINDOW_SIZE;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DEPTH
		| PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END


enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_FAIL;

	/* Draw our texture to the window such that each texel should map
	 * to the corresponding pixel of the window.
	 */
	glClear(GL_COLOR_BUFFER_BIT);

	piglit_draw_rect_tex(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT,
		0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);

	if (piglit_probe_image_rgb(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT,
		(const float *) &image))
		result = PIGLIT_PASS;

	piglit_present_results();

	return result;
}


void
piglit_init(int argc, char *argv[])
{
	int x, y, i;

	/* Set up a texture that it's color ramps with red to black top to
	 * bottom and green to black left to right.
	 */
	for (y = 0; y < TEXTURE_HEIGHT; y++) {
		for (x = 0; x < TEXTURE_WIDTH; x++) {
			i = (y * TEXTURE_WIDTH + x) * 3;

			image[i + 0] = (float)x / (TEXTURE_WIDTH - 1);
			image[i + 1] = 1.0 - ((float)  y / (TEXTURE_HEIGHT - 1));
			image[i + 2] = 0.0;
		}
	}

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB,
		TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGB, GL_FLOAT, image);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,
		GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
		GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
}
