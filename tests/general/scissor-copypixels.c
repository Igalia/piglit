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
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file scissor-copypixels.c
 *
 * Tests that glScissor properly affects glCopyPixels().
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLboolean
check_red_box_surrounded_by_green(int x, int y, int w, int h)
{
	GLboolean pass = GL_TRUE;
	int probe_x, probe_y;
	const float red[] = {1.0, 0.0, 0.0, 0.0};
	const float green[] = {0.0, 1.0, 0.0, 0.0};

	for (probe_y = y - 1; probe_y <= y + h; probe_y++) {
		for (probe_x = x - 1; probe_x <= x + w; probe_x++) {
			const float *expected;

			if (probe_y < y || probe_y >= y + h ||
			    probe_x < x || probe_x >= x + w)
				expected = green;
			else
				expected = red;

			pass &= piglit_probe_pixel_rgb(probe_x, probe_y,
						       expected);
		}
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int dst_x = piglit_width / 2 + 10, dst_y;
	int dst_w = 10, dst_h = 10;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* whole window red */
	glClearColor(1.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* right half green */
	glColor4f(0.0, 1.0, 0.0, 0.0);
	piglit_draw_rect(piglit_width / 2, 0, piglit_width / 2, piglit_height);

	/* Copy a 10x10 square from left to right */
	glEnable(GL_SCISSOR_TEST);
	dst_y = 10;
	glScissor(dst_x, dst_y, dst_w, dst_h);
	glRasterPos2i(dst_x - 5, dst_y - 5);
	glCopyPixels(10, 10, 20, 20, GL_COLOR);

	/* Don't copy a 10x10 square from left to right */
	dst_y = 30;
	glScissor(dst_x, dst_y, 0, 0);
	glRasterPos2i(dst_x - 5, dst_y - 5);
	glCopyPixels(10, 10, 20, 20, GL_COLOR);

	/* Copy an unscissored 10x10 square from left to right */
	glDisable(GL_SCISSOR_TEST);
	dst_y = 50;
	glRasterPos2i(dst_x, dst_y);
	glCopyPixels(10, 10, dst_w, dst_h, GL_COLOR);

	pass &= check_red_box_surrounded_by_green(dst_x, 10, dst_w, dst_h);
	pass &= check_red_box_surrounded_by_green(dst_x, 30, 0, 0);
	pass &= check_red_box_surrounded_by_green(dst_x, 50, dst_w, dst_h);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
}
