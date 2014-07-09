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
 * Tests that all the appropriate integer values of GL_TEXTURE_MIN_LOD and
 * GL_TEXTURE_MAX_LOD work on a mipmapped 2D texture.
 */

#include "piglit-util-gl.h"

#define MAX_SIZE	32
#define MAX_LOD	5
#define PAD		5

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 500;
	config.window_height = 600;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static GLfloat colors[][3] = {
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0},
	{1.0, 1.0, 0.0},
	{1.0, 0.0, 1.0},
	{0.0, 1.0, 1.0},
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

/**
 * Tests that the mipmap drawn at (x,y)-(x+size,y+size) has the color for the
 * clamped level.
 */
static GLboolean
test_results(int x, int y, int size, int level, int min_lod, int max_lod)
{
	GLboolean pass = GL_TRUE;
	int x1 = x + size / 4, x2 = x + size * 3 / 4;
	int y1 = y + size / 4, y2 = y + size * 3 / 4;
	int clamped_lod;

	clamped_lod = level;
	if (clamped_lod > max_lod)
		clamped_lod = max_lod;
	if (clamped_lod < min_lod)
		clamped_lod = min_lod;

	if (size == 1) {
		pass = pass && piglit_probe_pixel_rgb(x1, y1, colors[clamped_lod]);
	} else {
		pass = pass && piglit_probe_pixel_rgb(x1, y1, colors[clamped_lod]);
		pass = pass && piglit_probe_pixel_rgb(x2, y1, colors[clamped_lod]);
		pass = pass && piglit_probe_pixel_rgb(x2, y2, colors[clamped_lod]);
		pass = pass && piglit_probe_pixel_rgb(x1, y2, colors[clamped_lod]);
	}

	if (!pass) {
		printf("failed at level %d (%dx%d) with LOD clamped to "
		       "(%d,%d)\n",
		       level, size, size, min_lod, max_lod);
	}

	return pass;
}

static GLboolean
draw_and_test(int x_offset, int y_offset, float min_lod, float max_lod)
{
	GLfloat y;
	int dim;
	int level;
	GLboolean pass = GL_TRUE;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, max_lod);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, min_lod);

	y = y_offset;
	for (level = 0, dim = MAX_SIZE; dim > 0; level++, dim /= 2) {
		piglit_draw_rect_tex(x_offset, y, dim, dim,
				     0.0, 0.0, 1.0, 1.0);

		y += dim + PAD;
	}

	y = y_offset;
	for (level = 0, dim = MAX_SIZE; dim > 0; level++, dim /= 2) {
		pass = pass && test_results(x_offset, y,
					    dim, level,
					    min_lod, max_lod) && pass;

		y += dim + PAD;
	}

	return pass;
}


enum piglit_result
piglit_display(void)
{
	int dim;
	GLboolean pass = GL_TRUE;
	int level, min_lod, max_lod, x_offset, y_offset;
	GLuint tex;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* Clear background to gray */
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Create the texture. */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* Fill in each level */
	for (level = 0, dim = MAX_SIZE; dim > 0; level++, dim /= 2) {
		set_level_color(level, dim, level);
	}

	/* Draw all the levels with varying clamp ranges. */
	glEnable(GL_TEXTURE_2D);
	y_offset = 10;
	for (min_lod = 0; min_lod <= MAX_LOD; min_lod++) {
		x_offset = 10;

		for (max_lod = MAX_LOD; max_lod >= min_lod; max_lod--) {
			pass = draw_and_test(x_offset, y_offset,
					     min_lod, max_lod) && pass;
			x_offset += MAX_SIZE + PAD;
		}

		y_offset += (MAX_SIZE * 2 + PAD * 7);
	}

	glDeleteTextures(1, &tex);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
}
