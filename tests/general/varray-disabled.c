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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/**
 * @file varray-disabled.c
 *
 * Test whether no vertices are drawn when we call DrawArrays with no
 * vertex array enabled.
 *
 * http://bugs.freedesktop.org/show_bug.cgi?id=19911
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static void
set_colors(GLfloat *color_array, GLfloat *color)
{
	int i;

	for (i = 0; i < 4; i++) {
		color_array[i * 4 + 0] = color[0];
		color_array[i * 4 + 1] = color[1];
		color_array[i * 4 + 2] = color[2];
		color_array[i * 4 + 3] = 1.0;
	}
}

enum piglit_result
piglit_display(void)
{
	GLfloat vertices[4][2];
	GLfloat colors[16];
	GLboolean pass = GL_TRUE;
	GLfloat red[3]    = {1.0, 0.0, 0.0};
	GLfloat green[3]  = {0.0, 1.0, 0.0};
	GLfloat blue[3]   = {0.0, 0.0, 1.0};
	GLfloat black[3]  = {0.0, 0.0, 0.0};

	piglit_ortho_projection(1.0, 1.0, GL_FALSE);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColorPointer(4, GL_FLOAT, 0, colors);
	glEnableClientState(GL_COLOR_ARRAY);

	/* Draw the vertices enabled once on the left side for sanity */
	vertices[0][0] = 0.0;
	vertices[0][1] = 0.0;
	vertices[1][0] = 0.3;
	vertices[1][1] = 0.0;
	vertices[2][0] = 0.3;
	vertices[2][1] = 1.0;
	vertices[3][0] = 0.0;
	vertices[3][1] = 1.0;
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	set_colors(colors, red);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	/* Now disable and draw again. */
	vertices[0][0] = 0.3;
	vertices[0][1] = 0.0;
	vertices[1][0] = 0.7;
	vertices[1][1] = 0.0;
	vertices[2][0] = 0.7;
	vertices[2][1] = 1.0;
	vertices[3][0] = 0.3;
	vertices[3][1] = 1.0;
	/* This NULL pointer set was key in triggering the bug reported. */
	glVertexPointer (2, GL_FLOAT, 0, NULL);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	set_colors(colors, green);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	/* Now draw again enabled, to make sure the hardware hasn't given
	 * up on us.
	 */
	vertices[0][0] = 0.7;
	vertices[0][1] = 0.0;
	vertices[1][0] = 1.0;
	vertices[1][1] = 0.0;
	vertices[2][0] = 1.0;
	vertices[2][1] = 1.0;
	vertices[3][0] = 0.7;
	vertices[3][1] = 1.0;
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	set_colors(colors, blue);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	pass &= piglit_probe_pixel_rgb(piglit_width * 1 / 6,
				       piglit_height / 2, red);
	pass &= piglit_probe_pixel_rgb(piglit_width * 3 / 6,
				       piglit_height / 2, black);
	pass &= piglit_probe_pixel_rgb(piglit_width * 5 / 6,
				       piglit_height / 2, blue);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char *argv[])
{
}
