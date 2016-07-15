/*
 * Copyright © 2016 Intel Corporation
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
 * @file copy-pixels.c
 *
 * Test to verify glCopyPixels with GL_COLOR, GL_DEPTH and GL_STENCIL
 */

#include "piglit-util-gl.h"

#define IMAGE_WIDTH 60
#define IMAGE_HEIGHT 60
#define OFFSET 16

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_STENCIL | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

bool
test_color_copypix(int x, int y)
{
	bool pass = true;
	GLuint tex;

	float *expected = piglit_rgbw_image(GL_RGBA,
					    IMAGE_WIDTH, IMAGE_HEIGHT,
					    GL_FALSE, /* alpha */
					    GL_UNSIGNED_NORMALIZED);

	/* Initialize color data */
	tex = piglit_rgbw_texture(GL_RGBA, IMAGE_WIDTH, IMAGE_HEIGHT,
				  GL_FALSE, GL_FALSE,
				  GL_UNSIGNED_NORMALIZED);
	glBindTexture(GL_TEXTURE_2D, tex);
	glEnable(GL_TEXTURE_2D);

	piglit_draw_rect_tex(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, 0, 0, 1, 1);

	glDisable(GL_TEXTURE_2D);

	glRasterPos2i(x, y);
	glCopyPixels(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, GL_COLOR);
	pass = piglit_probe_image_color(x, y, IMAGE_WIDTH, IMAGE_HEIGHT,
					GL_RGBA, expected) && pass;
	free(expected);
	return pass;
}

bool
test_depth_copypix(int x, int y)
{
	int i;
	bool pass = true;
	float depth_val = 0.75;
	float *buf = malloc(IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(float));
	assert(buf);

	/* Initialize depth data */
	for (i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++)
		buf[i] = depth_val;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	glRasterPos2i(0, 0);
	glDrawPixels(IMAGE_WIDTH, IMAGE_HEIGHT,
		     GL_DEPTH_COMPONENT, GL_FLOAT, buf);
	glRasterPos2i(x, y);
	glCopyPixels(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, GL_DEPTH);
	free(buf);
	pass = piglit_probe_rect_depth(x, y, IMAGE_WIDTH, IMAGE_HEIGHT,
				       depth_val);
	return pass;
}

bool
test_stencil_copypix(int x, int y)
{
	int i;
	bool pass = true;
	float stencil_val = 2.0;
	float *buf = malloc(IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(float));
	assert(buf);

	/* Initialize stencil data */
	for (i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++)
		buf[i] = stencil_val;

	glRasterPos2i(0, 0);
	glDrawPixels(IMAGE_WIDTH, IMAGE_HEIGHT,
		     GL_STENCIL_INDEX, GL_FLOAT, buf);
	glRasterPos2i(x, y);
	glCopyPixels(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, GL_STENCIL);
	free(buf);
	pass = piglit_probe_rect_stencil(x, y, IMAGE_WIDTH, IMAGE_HEIGHT,
					 stencil_val);
	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	/* Test overlapping and non-overlapping copypixels with color, depth
	 * and stencil buffers.
	 */
	glClear(GL_COLOR_BUFFER_BIT);
	pass = test_color_copypix(IMAGE_WIDTH, 0) && pass;
	pass = test_color_copypix(IMAGE_WIDTH + OFFSET, IMAGE_HEIGHT + OFFSET)
	       && pass;
	pass = test_color_copypix(0, IMAGE_HEIGHT - OFFSET) && pass;

	piglit_present_results();

	glClear(GL_DEPTH_BUFFER_BIT);
	pass = test_depth_copypix(IMAGE_WIDTH, 0) && pass;
	pass = test_depth_copypix(IMAGE_WIDTH + OFFSET, IMAGE_HEIGHT + OFFSET)
	       && pass;
	pass = test_depth_copypix(0, IMAGE_HEIGHT - OFFSET) && pass;

	glClear(GL_STENCIL_BUFFER_BIT);
	pass = test_stencil_copypix(IMAGE_WIDTH, 0) && pass;
	pass = test_stencil_copypix(IMAGE_WIDTH + OFFSET, IMAGE_HEIGHT + OFFSET)
	       && pass;
	pass = test_stencil_copypix(0, IMAGE_HEIGHT - OFFSET) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	if ((piglit_get_gl_version() < 14) &&
	    !piglit_is_extension_supported("GL_ARB_window_pos")) {
		printf("Requires GL 1.4 or GL_ARB_window_pos");
		piglit_report_result(PIGLIT_SKIP);
	}

	glClearColor(0.25, 0.25, 0.25, 1.0);
	glClearDepth(0.0);
	glClearStencil(0.0);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
