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
 * @file line-aa-width.c
 *
 * Tests that width 1.0 AA lines are of the appropriate thickness.
 *
 * The 965 driver was rendering them so that when the line was
 * centered on a pixel it was fullly lit and when it was off the pixel
 * center neither of the neighbors would be lit at all.  It's quite
 * ugly.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 300;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static float
y_from_x(float x)
{
	return 2.0 + (piglit_height - 4.0) *
		(1.0 - cos(x / piglit_width * M_PI / 2));
}

/* Check that the color is approximately gray. There was a report that
 * Gen3 Intel is failing at this.
 */
static GLboolean
check_color(float *color)
{
	float max = 0.0;
	static GLboolean reported = GL_FALSE;

	if (fabs(color[1] - color[0]) > max)
		max = fabs(color[1] - color[0]) > 0.01;
	if (fabs(color[2] - color[0]) > max)
		max = fabs(color[2] - color[0]) > 0.01;

	if (max > 0.02) {
		if (!reported) {
			printf("Found color %f, %f, %f, expected %f, %f, %f\n",
			       color[0], color[1], color[2],
			       color[0], color[0], color[0]);
			reported = GL_TRUE;
		}

		return GL_FALSE;
	}

	return GL_TRUE;
}

enum piglit_result
piglit_display(void)
{
	int x1;
	int seg_width = 30;
	float *screen;
	GLboolean pass = GL_TRUE;

	/* The coverage checking assumes that we'll be sampling along
	 * the major axis, so a tall window will break that.
	 */
	if (piglit_width / piglit_height < 3)
		return PIGLIT_SKIP;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor4f(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_LINE_SMOOTH);
	/* GL AA lines produce an alpha value */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ZERO);

	/* Draw a bunch of line segments with varying slopes across
	 * the window.  They're separated by a bit of space so that we
	 * can see which regions we're going to sample in while
	 * avoiding any need to worry about end caps. */
	for (x1 = 0; x1 < piglit_width; x1 += seg_width) {
		int x2 = x1 + seg_width - 2;
		float y1, y2;

		if (x2 > piglit_width)
			x2 = piglit_width;

		y1 = y_from_x(x1);
		y2 = y_from_x(x2);

		glBegin(GL_LINES);
		glVertex2f(x1, y1);
		glVertex2f(x2, y2);
		glEnd();
	}

	screen = malloc(piglit_width * piglit_height * 4 * sizeof(float));
	glReadPixels(0, 0, piglit_width, piglit_height,
		     GL_RGBA, GL_FLOAT, screen);

	/* Now, sample the middles of the segments and compare the total
	 * coverage in each column
	 */
	for (x1 = 2; x1 < piglit_width; x1 += seg_width) {
		int x2 = x1 + seg_width - 4;
		int sample_x;
		float y1, y2;
		float avg = 0.0;
		float min = 100.0;
		float max = -100.0;
		char *err = NULL;

		if (x2 > piglit_width - 4)
			x2 = piglit_width - 4;

		/* If we don't have a couple of pixels to sample because we've
		 * hit the edge of the window, we're done.
		 */
		if (x2 - x1 < 2)
			break;

		y1 = y_from_x(x1) - 2;
		y2 = y_from_x(x2) + 2;

		avg = 0;
		for (sample_x = x1; sample_x < x2; sample_x++) {
			int y;
			float col_total = 0.0;

			for (y = y1; y < y2; y++) {
				if (y < 0 || y >= piglit_height)
					continue;

				pass = pass &&
					check_color(&screen[(y * piglit_width +
							     sample_x) * 4]);

				col_total += screen[(y * piglit_width +
						     sample_x) * 4];
			}
			if (col_total > max)
				max = col_total;
			if (col_total < min)
				min = col_total;
			avg += col_total / (x2 - x1);
		}

		if (min < 0.25)
			err = "min < 0.25";
		else if (avg / min > 2.0)
			err = "avg / min > 2.0";
		else if (max / avg > 2.0)
			err = "max / avg > 2.0";
		else if (max > 1.5)
			err = "max > 1.5";

		if (err) {
			printf("Line from %d,%d-%d,%d had bad thickness (%s):\n",
			       x1 - 2, (int)y_from_x(x1 - 2),
			       x2 + 2, (int)y_from_x(x2 + 2),
			       err);
			printf("min coverage: %f\n", min);
			printf("avg coverage: %f\n", avg);
			printf("max coverage: %f\n", max);
			pass = GL_FALSE;
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
}
