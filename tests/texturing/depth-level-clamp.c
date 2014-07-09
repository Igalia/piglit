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

/**
 * @file depth-level-clamp.c
 *
 * Tests that glTexImage2D()ing in the mipmap levels of a depth texture and then
 * rendering with them with various clamping works correctly.
 *
 * This test is designed to catch a failure in the 965 driver's depth
 * miptree copying for relayout that occurs due to the clamping.
 */

#include "piglit-util-gl.h"

#define MAX_SIZE	64
#define MAX_LOD	6
#define PAD		5

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = (MAX_SIZE*2+PAD*3);
	config.window_height = (MAX_SIZE*MAX_LOD+PAD*(MAX_LOD+1));
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static void
set_level_value(int level, int size, float val)
{
	GLfloat *tex;
	int x, y;

	tex = malloc(size * size * sizeof(GLfloat));

	for (y = 0; y < size; y++) {
		for (x = 0; x < size; x++) {
			tex[(y * size + x)] = val;
		}
	}

	glTexImage2D(GL_TEXTURE_2D, level, GL_DEPTH_COMPONENT,
		     size, size, 0,
		     GL_DEPTH_COMPONENT, GL_FLOAT, tex);

	free(tex);
}

enum piglit_result
piglit_display(void)
{
	int dim;
	GLboolean pass = GL_TRUE;
	int level, x, y;
	GLuint tex;
	float val;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* Clear background to gray */
	glClearColor(0.0, 0.7, 0.0, 1.0);
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
	val = 1.0;
	for (level = 0, dim = MAX_SIZE; dim > 0; level++, dim /= 2) {
		set_level_value(level, dim, val);
		val -= 1.0 / MAX_LOD;
	}

	glEnable(GL_TEXTURE_2D);

	/* Draw areas of the base level size with clamping to the
	 * minimum mip lod of each texture level.
	 */
	x = PAD;
	y = PAD;
	for (level = 0, dim = MAX_SIZE; dim > 1; level++, dim /= 2) {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, level);
		piglit_draw_rect_tex(x, y, MAX_SIZE, MAX_SIZE,
				     0.0, 0.0, 1.0, 1.0);
		y += MAX_SIZE + PAD;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0.0);

	/* Draw areas of the base level size with level clamping to
	 * each texture level.
	 */
	x = PAD + MAX_SIZE + PAD;
	y = PAD;
	for (level = 0, dim = MAX_SIZE; dim > 1; level++, dim /= 2) {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, level);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level);
		piglit_draw_rect_tex(x, y, MAX_SIZE, MAX_SIZE,
				     0.0, 0.0, 1.0, 1.0);
		y += MAX_SIZE + PAD;
	}

	/* Verify that the resulting images are blended between the levels. */
	x = PAD;
	y = PAD;
	val = 1.0;
	for (level = 0, dim = MAX_SIZE; dim > 1; level++, dim /= 2) {
		float expected[3];
		int i;

		for (i = 0; i < 3; i++)
			expected[i] = val;

		pass = piglit_probe_pixel_rgb(x + MAX_SIZE / 2,
					      y + MAX_SIZE / 2,
					      expected) && pass;

		pass = piglit_probe_pixel_rgb(x + MAX_SIZE + PAD + MAX_SIZE / 2,
					      y + MAX_SIZE / 2,
					      expected) && pass;

		y += MAX_SIZE + PAD;
		val -= 1.0 / MAX_LOD;
	}

	glDeleteTextures(1, &tex);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_depth_texture");
}
