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

/** @file depth-clamp-range.c
 *
 * Tests that ARB_depth_clamp enablement didn't break DepthRange functionality,
 * and properly uses the min/max selection.
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

void
quad(float base_x, float base_y, float z)
{
	glBegin(GL_QUADS);
	glVertex3f(base_x,      base_y,      z);
	glVertex3f(base_x + 10, base_y,      z);
	glVertex3f(base_x + 10, base_y + 10, z);
	glVertex3f(base_x,      base_y + 10, z);
	glEnd();
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float white[3] = {1.0, 1.0, 1.0};
	float clear[3] = {0.0, 0.0, 0.0};

	glClearDepth(0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glColor3fv(white);

	/* Keep in mind that the ortho projection flips near and far's signs,
	 * so 1.0 to quad()'s z maps to glDepthRange's near, and -1.0 maps to
	 * glDepthRange's far.
	 */

	/* Basic glDepthRange testing. */
	glDisable(GL_DEPTH_CLAMP);
	glDepthRange(0, 1);
	quad(10, 10, 0.5); /* .25 - drawn. */

	glDepthRange(1, 0);
	quad(10, 30, 0.5); /* 0.75 - not drawn. */

	/* Now, test that near depth clamping works.*/
	glEnable(GL_DEPTH_CLAMP);
	glDepthRange(0.25, 1.0);
	quad(30, 10, 2); /* 0.25 - drawn. */

	glDepthRange(0.75, 1.0);
	quad(30, 30, 2); /* 0.75 - not drawn. */

	/* Test that far clamping works.*/
	glDepthRange(0.0, 0.25);
	quad(50, 10, -2); /* 0.25 - drawn. */

	glDepthRange(0.0, 0.75);
	quad(50, 30, -2); /* 0.75 - not drawn. */

	/* Now, flip near and far around and make sure that it's doing the
	 * min/max of near and far in the clamping.
	 */

	/* Test that near (max) clamping works. */
	glDepthRange(0.25, 0.0);
	quad(70, 10, 2); /* 0.25 - drawn. */

	glDepthRange(0.75, 0.0);
	quad(70, 30, 2); /* 0.75 - not drawn. */

	/* Now, test far (min) clamping works. */
	glDepthRange(1.0, 0.0);
	quad(90, 10, -2); /* 0.0 - drawn */

	glDepthRange(1.0, 0.75);
	quad(90, 30, -2); /* 0.75 - not drawn*/

	pass = piglit_probe_pixel_rgb(15, 15, white) && pass;
	pass = piglit_probe_pixel_rgb(15, 35, clear) && pass;
	pass = piglit_probe_pixel_rgb(35, 15, white) && pass;
	pass = piglit_probe_pixel_rgb(35, 35, clear) && pass;
	pass = piglit_probe_pixel_rgb(55, 15, white) && pass;
	pass = piglit_probe_pixel_rgb(55, 35, clear) && pass;
	pass = piglit_probe_pixel_rgb(75, 15, white) && pass;
	pass = piglit_probe_pixel_rgb(75, 35, clear) && pass;
	pass = piglit_probe_pixel_rgb(95, 15, white) && pass;
	pass = piglit_probe_pixel_rgb(95, 35, clear) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
