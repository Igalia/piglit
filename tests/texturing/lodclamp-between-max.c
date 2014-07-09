/*
 * Copyright © 2008-2009 Intel Corporation
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

/**
 * @file lodclamp-between-max.c
 *
 * Tests that setting maximum LOD clamp to between two texture levels
 * results in appropriate mipmap filtering.
 */

#include "piglit-util-gl.h"

#define MAX_SIZE	32
#define MAX_LOD	5
#define PAD		5

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static GLfloat colors[][3] = {
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0},
	{1.0, 0.0, 1.0},
	{0.0, 1.0, 1.0},
	{1.0, 1.0, 0.0},
};

static void
set_level_color(int level, int size, int color)
{
	GLfloat *tex;
	int x, y;

	tex = malloc(size * size * 3 * sizeof(GLfloat));

	for (y = 0; y < size; y++) {
		for (x = 0; x < size; x++) {
			tex[(y * size + x) * 3 + 0] = colors[color][0];
			tex[(y * size + x) * 3 + 1] = colors[color][1];
			tex[(y * size + x) * 3 + 2] = colors[color][2];
		}
	}

	glTexImage2D(GL_TEXTURE_2D, level, GL_RGB,
		     size, size, 0,
		     GL_RGB, GL_FLOAT, tex);

	free(tex);
}

enum piglit_result
piglit_display(void)
{
	int dim;
	GLboolean pass = GL_TRUE;
	int level, x, y;
	GLuint tex;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* Clear background to gray */
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Create the texture. */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* Fill in each level */
	for (level = 0, dim = MAX_SIZE; dim > 0; level++, dim /= 2) {
		set_level_color(level, dim, level);
	}

	glEnable(GL_TEXTURE_2D);

	/* Draw areas of the base level size with clamping to mip lods
	 * between each texture level.
	 */
	x = 10;
	y = 10;
	for (level = 0, dim = MAX_SIZE; dim > 1; level++, dim /= 2) {
		float clamp = (float)level + 0.5;

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, clamp);

		piglit_draw_rect_tex(x, y, 1, 1,
				     0.0, 0.0, 1.0, 1.0);

		y += 1 + PAD;
	}

	/* Verify that the resulting images are blended between the levels. */
	y = 10;
	for (level = 0, dim = MAX_SIZE; dim > 1; level++, dim /= 2) {
		float expected[3];
		int i;

		for (i = 0; i < 3; i++) {
			expected[i] = (colors[level    ][i] +
				       colors[level + 1][i]) / 2.0;
		}

		pass = piglit_probe_pixel_rgb(x,
					      y,
					      expected) && pass;

		y += 1 + PAD;
	}

	glDeleteTextures(1, &tex);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
}
