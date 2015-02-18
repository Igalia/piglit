/*
 * Copyright 2015, VMware, Inc.
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


/** @file drawpixels-color-index.c
 *
 * Test glDrawPixels(format=GL_COLOR_INDEX, type=GL_UNSIGNED_BYTE) and
 * glDrawPixels(format=GL_COLOR_INDEX, type=GL_BITMAP).
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


static bool
test_ci(int x, int y)
{
	bool pass = true;
	static const float red_map[4] = {1.0, 0.0, 0.0, 1.0};
	static const float green_map[4] = {0.0, 1.0, 0.0, 1.0};
	static const float blue_map[4] = {0.0, 0.0, 1.0, 1.0};
	static const float alpha_map[4] = {1.0, 1.0, 1.0, 1.0};
	int x1, y1, x2, y2;
	int i, j;
	int width = 28, height = 18;
	GLubyte *image = malloc(width * height);

	/* Setup CI image with each quadrant an index in [0,3] */
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			int index = ((i < height / 2) ? 0 : 2)
				+ (j > width / 2);
			image[i * width + j] = index;
		}
	}

	glPixelMapfv(GL_PIXEL_MAP_I_TO_R, 4, red_map);
	glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 4, green_map);
	glPixelMapfv(GL_PIXEL_MAP_I_TO_B, 4, blue_map);
	glPixelMapfv(GL_PIXEL_MAP_I_TO_A, 4, alpha_map);
	glPixelTransferi(GL_MAP_COLOR, GL_TRUE);

	glWindowPos2i(x, y);
	glDrawPixels(width, height, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, image);

	glPixelTransferi(GL_MAP_COLOR, GL_FALSE);

	free(image);

	x1 = x + width/4;
	x2 = x + width*3/4;
	y1 = y + height/4;
	y2 = y + height*3/4;

	pass = piglit_probe_pixel_rgba(x1, y1, red_map) && pass;
	pass = piglit_probe_pixel_rgba(x2, y1, green_map) && pass;
	pass = piglit_probe_pixel_rgba(x1, y2, blue_map) && pass;
	pass = piglit_probe_pixel_rgba(x2, y2, alpha_map) && pass;

	if (!pass)
		printf("glDrawPixels(format=GL_COLOR_INDEX) test failed\n");

	return pass;
}


static bool
test_bitmap(int x, int y)
{
	bool pass = true;
	static const float red_map[2] = {1.0, 0.0};
	static const float green_map[2] = {0.0, 0.0};
	static const float blue_map[2] = {0.0, 1.0};
	static const float alpha_map[2] = {1.0, 1.0};
	static const float red[4] = {1.0, 0.0, 0.0, 1.0}; /* left half */
	static const float blue[4] = {0.0, 0.0, 1.0, 1.0}; /* right half */
	int x1, x2, y1;
	int i, j;
	int width = 32, height = 20;
	GLubyte *image = malloc(width/8 * height);

	/* Setup CI image with left half = 0, right half = 1 */
	for (i = 0; i < height; i++) {
		for (j = 0; j < width/8; j++) {
			/* setup 8 bits (pixels) at a time */
			int index = (j >= width / 16) ? 255 : 0;
			image[i * width/8 + j] = index;
		}
	}

	glPixelMapfv(GL_PIXEL_MAP_I_TO_R, 2, red_map);
	glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 2, green_map);
	glPixelMapfv(GL_PIXEL_MAP_I_TO_B, 2, blue_map);
	glPixelMapfv(GL_PIXEL_MAP_I_TO_A, 2, alpha_map);
	glPixelTransferi(GL_MAP_COLOR, GL_TRUE);

	glWindowPos2i(x, y);
	glDrawPixels(width, height, GL_COLOR_INDEX, GL_BITMAP, image);
	glPixelTransferi(GL_MAP_COLOR, GL_FALSE);

	free(image);

	x1 = x + width/4;
	x2 = x + width*3/4;
	y1 = y + height/2;

	pass = piglit_probe_pixel_rgba(x1, y1, red) && pass;
	pass = piglit_probe_pixel_rgba(x2, y1, blue) && pass;

	if (!pass)
		printf("glDrawPixels(type=GL_BITMAP) test failed\n");

	return pass;
}


enum piglit_result
piglit_display(void)
{
	bool pass;

	glClear(GL_COLOR_BUFFER_BIT);

	pass = test_ci(10, 10);
	pass = test_bitmap(70, 10) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	glClearColor(0.25, 0.25, 0.25, 0.25);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}
