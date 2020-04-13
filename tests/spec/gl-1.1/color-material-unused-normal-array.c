/*
 * Copyright © 2020 Intel Corporation
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
 * @file color-material-unused-normal-array.c
 * Tests that unused GL_NORMAL_ARRAY doesn't get mixed up with other arrays.
 * See https://gitlab.freedesktop.org/mesa/mesa/issues/2758
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 11;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static GLfloat pos[4][3] =
{
	{  1.0,  -1.0, 0.0 },
	{  1.0,   1.0, 0.0 },
	{  -1.0,  1.0, 0.0 },
	{  -1.0, -1.0, 0.0 }
};

static GLfloat norms[4][3] =
{
	{ 1, 0, 0},
	{ 1, 0, 0},
	{ 1, 0, 0},
	{ 1, 0, 0}
};

static GLfloat colors[4][4] =
{
	{ 0, 1, 0, 1},
	{ 0, 1, 0, 1},
	{ 0, 1, 0, 1},
	{ 0, 1, 0, 1}
};

enum piglit_result
piglit_display(void)
{
	static float black[4] = {0.0, 0.0, 0.0, 0.0};
	static float green[4] = {0.0, 1.0, 0.0, 1.0};

	bool pass = true;

	glClearColor(0.0, 0.0, 0.0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glColorMaterial(GL_FRONT_AND_BACK, GL_EMISSION);
	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, pos);
	glNormalPointer(GL_FLOAT, 0, norms);
	glColorPointer(4, GL_FLOAT, 0, colors);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);

	pass &= piglit_probe_pixel_rgba(piglit_width - 1, piglit_height - 1, green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	/* nothing */
}
