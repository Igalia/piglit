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

/** @file depthrange-clear.c
 *
 * Tests that clears are appropriately unaffected by glDepthRange().
 * Caught a regression in the intel driver with the metaops clear code.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Convenience function to draw an axis-aligned rectangle.
 */
static void
draw_rect(float x, float y, float w, float h, float d)
{
	float verts[4][4];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = d;
	verts[0][3] = 1.0;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = d;
	verts[1][3] = 1.0;
	verts[2][0] = x + w;
	verts[2][1] = y + h;
	verts[2][2] = d;
	verts[2][3] = 1.0;
	verts[3][0] = x;
	verts[3][1] = y + h;
	verts[3][2] = d;
	verts[3][3] = 1.0;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glEnableClientState(GL_VERTEX_ARRAY);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
}

static void
draw_rect_set(int y)
{
	draw_rect(10, y, 10, 10, -.75);
	draw_rect(30, y, 10, 10, -.25);
	draw_rect(50, y, 10, 10,  .25);
	draw_rect(70, y, 10, 10,  .75);
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int x, y;
	static float red[]   = {1.0, 0.0, 0.0, 0.0};
	static float green[] = {0.0, 1.0, 0.0, 0.0};

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor4fv(red);

	glClearDepth(0.5);
	glClear(GL_DEPTH_BUFFER_BIT);
	draw_rect_set(10);

	glDepthRange(0.5, 1.0);
	glClearDepth(0.5);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthRange(0.0, 1.0);
	draw_rect_set(30);

	glDepthRange(0.0, 0.5);
	glClearDepth(0.5);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthRange(0.0, 1.0);
	draw_rect_set(50);

	for (y = 0; y < 3; y++) {
		for (x = 0; x < 4; x++) {
			float *expected;

			if (x < 2)
				expected = green;
			else
				expected = red;

			pass &= piglit_probe_pixel_rgb(15 + x * 20, 15 + y * 20,
						       expected);
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
}
