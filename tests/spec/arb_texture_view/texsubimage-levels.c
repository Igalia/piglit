/*
 * Copyright © 2014 Intel Corporation
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
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 */

/**
 * \file texsubimage-levels.c
 * This tests that TexSubImage* into a view behaves correctly when the view
 * has a nonzero MinLevel.
 *
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 15;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

#define NUM_LEVELS 7
#define VIEW_MIN_LEVEL 2
#define VIEW_NUM_LEVELS 3
#define TEX_SIZE 64

void
piglit_init(int argc, char **argv)
{
	GLuint tex, view;
	int i, j;
	bool pass = true;

	piglit_require_extension("GL_ARB_texture_view");

	/* build a texture with full miptree */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, NUM_LEVELS,
		       GL_RGBA8, TEX_SIZE, TEX_SIZE);

	for (i=0; i < NUM_LEVELS; i++) {
		int dim = TEX_SIZE >> i;
		GLubyte *pixels = create_solid_image(dim, dim, 1, 4, i);
		if (!pixels) {
			printf("Failed to allocate image for level %d\n", i);
			piglit_report_result(PIGLIT_FAIL);
		}
		glTexSubImage2D(GL_TEXTURE_2D,
				i, 0, 0, dim, dim,
				GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		free(pixels);
	}

	/* create a view to a subset of the layers */
	glGenTextures(1, &view);
	glTextureView(view, GL_TEXTURE_2D, tex, GL_RGBA8,
		      VIEW_MIN_LEVEL, VIEW_NUM_LEVELS, 0, 1);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* upload through the view */
	glBindTexture(GL_TEXTURE_2D, view);
	for (i = 0; i < VIEW_NUM_LEVELS; i++) {
		int dim = TEX_SIZE >> (VIEW_MIN_LEVEL + i);
		GLubyte *pixels = create_solid_image(dim, dim,
						     1, 4, i + NUM_LEVELS);
		if (!pixels) {
			printf("Failed to allocate image for view level %d\n", i);
			piglit_report_result(PIGLIT_FAIL);
		}
		glTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, dim, dim,
				GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		free(pixels);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* bind the underlying texture and readback */
	glBindTexture(GL_TEXTURE_2D, tex);
	for (i = 0; i < NUM_LEVELS; i++) {
		/* the levels inside the view should have been replaced.
		 * everything else should be untouched.
		 */

		float expected_color[4];
		int dim = TEX_SIZE >> i;
		int color_index = i;
		if (i >= VIEW_MIN_LEVEL &&
		    i < VIEW_MIN_LEVEL + VIEW_NUM_LEVELS) {
			color_index = i + NUM_LEVELS - VIEW_MIN_LEVEL;
		}

		printf("Testing level %d\n", i);

		for (j = 0; j < 4; j++)
			expected_color[j] = Colors[color_index][j] / 255.0f;

		pass = piglit_probe_texel_rect_rgba(GL_TEXTURE_2D,
						    i, 0, 0, dim, dim,
						    expected_color) && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
