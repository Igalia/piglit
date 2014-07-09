/*
 * Copyright © 2009 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 */

/** @file depth_clamp.c
 * Tests ARB_depth_clamp functionality by drawing side-by-side triangles,
 * lines, points, and raster images that go behind the near plane, and
 * testing that when DEPTH_CLAMP is enabled they get rasterized as they should.
 *
 * An extension of this test would be to test that the depth values are
 * correctly clamped to the near/far plane, not just unclipped, and to test
 * the same operations against the far plane.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_depth_clamp");

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float white[3] = {1.0, 1.0, 1.0};
	float clear[3] = {0.0, 0.0, 0.0};
	float white_rect[20 * 20 * 3];
	int i;

	for (i = 0; i < ARRAY_SIZE(white_rect); i++)
		white_rect[i] = 1.0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* 1: unclamped quad */
	glDisable(GL_DEPTH_CLAMP);
	glColor4fv(white);
	glBegin(GL_QUADS);
	glVertex3f(10, 10, 0);
	glVertex3f(30, 10, 0);
	glVertex3f(30, 30, -2);
	glVertex3f(10, 30, -2);
	glEnd();

	/* 2: clamped quad */
	glEnable(GL_DEPTH_CLAMP);
	glBegin(GL_QUADS);
	glVertex3f(40, 10, 0);
	glVertex3f(60, 10, 0);
	glVertex3f(60, 30, -2);
	glVertex3f(40, 30, -2);
	glEnd();

	/* 3: unclamped line */
	glDisable(GL_DEPTH_CLAMP);
	glBegin(GL_LINES);
	glVertex3f(10.5, 40.5, 0);
	glVertex3f(10.5, 60.5, -2);
	glEnd();

	/* 4: clamped line */
	glEnable(GL_DEPTH_CLAMP);
	glBegin(GL_LINES);
	glVertex3f(40.5, 40.5, 0);
	glVertex3f(40.5, 60.5, -2);
	glEnd();

	/* 5: unclamped point */
	glDisable(GL_DEPTH_CLAMP);
	glBegin(GL_POINTS);
	glVertex3f(10.5, 70.5, -2);
	glEnd();

	/* 6: clamped point */
	glEnable(GL_DEPTH_CLAMP);
	glBegin(GL_POINTS);
	glVertex3f(40.5, 70.5, -2);
	glEnd();

	/* 7: unclamped raster */
	glDisable(GL_DEPTH_CLAMP);
	glRasterPos3f(10, 80, -2);
	glDrawPixels(20, 20, GL_RGB, GL_FLOAT, white_rect);

	/* 8: clamped raster */
	glEnable(GL_DEPTH_CLAMP);
	glRasterPos3f(40, 80, -2);
	glDrawPixels(20, 20, GL_RGB, GL_FLOAT, white_rect);

	/* 1: unclamped quad */
	pass = piglit_probe_pixel_rgb(20, 15, white) && pass;
	pass = piglit_probe_pixel_rgb(20, 25, clear) && pass;

	/* 2: clamped quad */
	pass = piglit_probe_pixel_rgb(50, 15, white) && pass;
	pass = piglit_probe_pixel_rgb(50, 25, white) && pass;

	/* 3: unclamped line */
	pass = piglit_probe_pixel_rgb(10, 45, white) && pass;
	pass = piglit_probe_pixel_rgb(10, 55, clear) && pass;

	/* 4: unclamped line */
	pass = piglit_probe_pixel_rgb(40, 45, white) && pass;
	pass = piglit_probe_pixel_rgb(40, 55, white) && pass;

	/* 5: unclamped point */
	pass = piglit_probe_pixel_rgb(10, 70, clear) && pass;

	/* 6: clamped point */
	pass = piglit_probe_pixel_rgb(40, 70, white) && pass;

	/* 7: unclamped raster */
	pass = piglit_probe_pixel_rgb(20, 90, clear) && pass;

	/* 8: clamped raster */
	pass = piglit_probe_pixel_rgb(50, 90, white) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
