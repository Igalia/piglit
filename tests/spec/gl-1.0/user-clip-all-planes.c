/*
 * Copyright © 2017 Fabian Bieler
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
 * \file user-clip-all-planes.c
 *
 * Arrange all clip planes perpendicular to the x-y-plane with equal angles
 * between them and a distance of 0.5 to the origin.
 * The user defined clip space should thus form a n-prism of infinite height
 * centered around the z-axis where n is GL_MAX_CLIP_PLANES.
 *
 * Draw a green quad filling the screen.
 *
 * The resulting render should be an n-sided regular polygon.
 *
 * Disable clipping, enable blending and draw the expected polygon in blue.
 *
 * Check that the entire screen is either black (clear color) or teal.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.window_width = 500;
	config.window_height = 250;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	int max_clip_planes;
	const float black[4] = {0, 0, 0, 1};
	const float green[4] = {0, 1, 0, 1};
	const float blue[4] = {0, 0, 1, 1};
	const float teal[4] = {0, 1, 1, 1};
	bool pass = true;

	/* Use some coordinate transformation to check that clip planes are
	 * transformed correctly.
	 */
	glLoadIdentity();
	glScalef(0.5, 1, 1);

	glGetIntegerv(GL_MAX_CLIP_PLANES, &max_clip_planes);

	for (int i = 0; i < max_clip_planes; ++i) {
		const double phi = 2 * M_PI / max_clip_planes * i;
		const double clip_plane[] = { -cos(phi), -sin(phi), 0, 0.5 };
		glClipPlane(GL_CLIP_PLANE0 + i, clip_plane);
	}

	glClear(GL_COLOR_BUFFER_BIT);

	/* First pass: Clipped quad */
	for (int i = 0; i < max_clip_planes; ++i)
		glEnable(GL_CLIP_PLANE0 + i);

	glColor4fv(green);
	piglit_draw_rect(-2, -1, 4, 2);

	for (int i = 0; i < max_clip_planes; ++i)
		glDisable(GL_CLIP_PLANE0 + i);

	/* Second pass: Polygon */
	glEnable(GL_BLEND);

	glColor4fv(blue);
	glBegin(GL_POLYGON);
	const double alpha = M_PI / max_clip_planes; /* half exterior angle */
	const double r = 0.5 / cos(alpha); /* circumradius */
	for (int i = 0; i < max_clip_planes; ++i) {
		const double phi = 2 * M_PI / max_clip_planes * i + alpha;
		const double x = cos(phi) * r;
		const double y = sin(phi) * r;
		glVertex2d(x, y);
	}
	glEnd();

	glDisable(GL_BLEND);

	/* Check render */
	pass = piglit_probe_rect_two_rgb(0, 0, piglit_width, piglit_height,
					 black, teal);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	glBlendFunc(GL_ONE, GL_ONE);
}
