/*
 * Copyright © 2011 Intel Corporation
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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	int x, y;
	float gray[4] = {0.5, 0.5, 0.5, 0.5};
	float green[4] = {0.0, 1.0, 0.0, 0.0};

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	piglit_ortho_projection(piglit_width, piglit_height, false);

	glColor4f(0.0, 1.0, 0.0, 0.0);
	for (x = 0; x <= 5; x++) {
		for (y = 0; y <= 5; y++) {
			float x1 = 5 + (10 + 5) * x + y * 0.2;
			float y1 = 5 + (10 + 5) * y + x * 0.2;

			piglit_draw_rect(x1, y1, 10, 10);
		}
	}

	for (x = 0; x <= 5; x++) {
		for (y = 0; y <= 5; y++) {
			int x1 = 5 + (10 + 5) * x;
			int y1 = 5 + (10 + 5) * y;
			bool p = true;

			if (x >= 3)
				y1++;
			if (y >= 3)
				x1++;

			p = p && piglit_probe_rect_rgba(x1, y1, 10, 10, green);

			p = p && piglit_probe_rect_rgba(x1 - 1, y1,
							1, 10, gray);
			p = p && piglit_probe_rect_rgba(x1 + 10, y1,
							1, 10, gray);
			p = p && piglit_probe_rect_rgba(x1, y1 - 1,
							10, 1, gray);
			p = p && piglit_probe_rect_rgba(x1, y1 + 10,
							10, 1, gray);

			if (!p) {
				printf("Failure on rectangle (%d, %d): "
				       "offset (%f, %f)\n",
				       x, y, y * 0.2, x * 0.2);
				pass = false;
			}
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{

	printf("The test's expectation is that the implementation samples\n"
	       "at pixel centers to produce fragments, so the fourth\n"
	       "(subpixel offset = 0.6) rectangle in each axis will\n"
	       "be offset compared to the previous.\n\n");

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
