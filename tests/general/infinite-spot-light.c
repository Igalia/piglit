/*
 * Copyright (C) 2011 Intel Corporation
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
 *    Yuanhan Liu <yuanhan.liu@linux.intel.com>
 */

/** @file infinite-spot-light.c
 *
 * This is a case that sounds doesn't make sense, but it is allowed by glSpec
 * (see section 2.14.1 Lightin of glspec 2.1.pdf).  While writing this case,
 * it servers as two purposes:
 *
 *   1. Test if swrast is OK with this case.
 *      The old mesa code would always compute a zero attenuation, thus always
 *      get a black lighting color.
 *
 *   2. Test if hardware rendering(only i965 tested) is OK with this patch.
 *      The old mesa code would skip the attenuation and spot computation while
 *      infinite light is met. This is somehow not permitted by glSpec.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/* Already normalized, and 0.5 would be the expected color */
static GLfloat dir[3] = {0.866025404, 0.0, 0.5};
static GLfloat pos[4] = {0.0, 0.0, -1.0, 0.0};		/* infinite */
static GLfloat light_ambient[3] = {1.0, 0.0, 0.0};

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLfloat expected[4] = {0.5, 0.0, 0.0, 1.0};

	glClear(GL_COLOR_BUFFER_BIT);

	glPointSize(10);
	glBegin(GL_POINTS);
		glVertex2f(0.5, 0.5);
	glEnd();

	pass = piglit_probe_pixel_rgba(0, 0, expected);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	GLfloat buf[4] = {0.0, 0.0, 0.0, 1.0};

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glEnable(GL_LIGHT0);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 89.0);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 1.0);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, buf);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	glMaterialfv(GL_FRONT, GL_DIFFUSE, buf);
	glMaterialfv(GL_FRONT, GL_SPECULAR, buf);

	buf[0] = 1.0;
	buf[1] = 1.0;
	buf[2] = 1.0;
	glMaterialfv(GL_FRONT, GL_AMBIENT, buf);

	glEnable(GL_LIGHTING);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
