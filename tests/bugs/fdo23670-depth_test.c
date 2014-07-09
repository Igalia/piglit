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
 *    Shuang he <shuang.he@intel.com>
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	/* Don't use piglit_ortho_projection!  This uses a non-default
	 * depth range!
	 */
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, piglit_width, 0, piglit_height, 2, -2);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

enum piglit_result
piglit_display(void)
{
	static float white[] = {1.0, 1.0, 1.0, 0.0};
	static float red[] = {1.0, 0.0, 0.0, 0.0};
	static float blue[] = {0.0, 0.0, 1.0, 0.0};
	GLboolean pass = GL_TRUE;

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glDisable(GL_DITHER);
	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT);
	glDepthFunc(GL_ALWAYS);
	glRasterPos3f(0.0, 0.0, 0.5);
	glDrawPixels(1, 1, GL_RGBA, GL_FLOAT, white);
	glRasterPos3f(2.0, 0.0, 0.5);
	glDrawPixels(1, 1, GL_RGBA, GL_FLOAT, white);

	glDepthFunc(GL_LESS);
	glRasterPos3f(0.0, 0.0, 0.0);
	glDrawPixels(1, 1, GL_RGBA, GL_FLOAT, red);
	glRasterPos3f(2.0, 0.0, 1.0);
	glDrawPixels(1, 1, GL_RGBA, GL_FLOAT, blue);

	pass &= piglit_probe_pixel_rgb(0, 0, red);
	pass &= piglit_probe_pixel_rgb(2, 0, white);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
