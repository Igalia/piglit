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
 * \file clear-into-view-2d-array.c
 * This tests that glClear() into a layer of a 2D array view (with nonzero MinLayer)
 * works.
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

#define NUM_LAYERS 7
#define VIEW_MIN_LAYER 2
#define VIEW_NUM_LAYERS 3
#define CLEAR_LAYER 1
#define TEX_SIZE 64

void
piglit_init(int argc, char **argv)
{
	GLuint tex, view;
	GLuint fbo;
	int i, j;
	bool pass = true;

	piglit_require_extension("GL_ARB_texture_view");

	/* build a 2d array texture; no mip levels */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1,
		GL_RGBA8, TEX_SIZE, TEX_SIZE, NUM_LAYERS);

	for (i=0; i < NUM_LAYERS; i++) {
		GLubyte *pixels = create_solid_image(TEX_SIZE, TEX_SIZE, 1, 4, i);
		if (!pixels) {
			printf("Allocation failure for layer %d\n", i);
			piglit_report_result(PIGLIT_FAIL);
		}
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
				0, 0, i, TEX_SIZE, TEX_SIZE, 1,
				GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		free(pixels);
	}

	/* create a view to a subset of the layers */
	glGenTextures(1, &view);
	glTextureView(view, GL_TEXTURE_2D_ARRAY, tex, GL_RGBA8,
		      0, 1, VIEW_MIN_LAYER, VIEW_NUM_LAYERS);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* set up for rendering into the view.
	 * we attach just the middle layer of the view, to exercise both minlayer
	 * and the attachment's layer index.
	 */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  view, 0, CLEAR_LAYER);
	if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		piglit_report_result(PIGLIT_FAIL);
	glViewport(0, 0, TEX_SIZE, TEX_SIZE);

	/* clear through the view */
	glClearColor(Colors[NUM_LAYERS][0] / 255.0f,
		     Colors[NUM_LAYERS][1] / 255.0f,
		     Colors[NUM_LAYERS][2] / 255.0f,
		     Colors[NUM_LAYERS][3] / 255.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* bind the underlying texture and readback */
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
	for (i = 0; i < NUM_LAYERS; i++) {
		/* the layer we cleared should have been replaced;
		 * everything else should be untouched.
		 */

		int color_index = (i == VIEW_MIN_LAYER + CLEAR_LAYER) ? NUM_LAYERS : i;
		float expected_color[4];

		printf("Testing layer %d\n", i);

		for (j = 0; j < 4; j++)
			expected_color[j] = Colors[color_index][j] / 255.0f;

		pass = piglit_probe_texel_volume_rgba(GL_TEXTURE_2D_ARRAY, 0,
						      0, 0, i,
						      TEX_SIZE, TEX_SIZE, 1,
						      expected_color) && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
