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

/** @file depthfunc.c
 *
 * Tests that glDepthFunc()'s various modes all work correctly.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Convenience function to draw an axis-aligned rectangle.
 */
static void
draw_rect_depth(float x, float y, float w, float h, float d)
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

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int i;
	GLenum funcs[] = {
		GL_NEVER,
		GL_LESS,
		GL_EQUAL,
		GL_LEQUAL,
		GL_GREATER,
		GL_NOTEQUAL,
		GL_GEQUAL,
		GL_ALWAYS,
	};
	const float green[4] = {0, 1, 0, 0};
	const float blue[4] =  {0, 0, 1, 0};

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	/* Clear gray. */
	glClearColor(0.5, 0.5, 0.5, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor4f(0.0, 1.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	for (i = 0; i < ARRAY_SIZE(funcs); i++) {
		draw_rect_depth(10, 10 + 20 * i, 10, 10, 0.0);
		draw_rect_depth(30, 10 + 20 * i, 10, 10, 0.0);
		draw_rect_depth(50, 10 + 20 * i, 10, 10, 0.0);
	}

	glColor4f(0.0, 0.0, 1.0, 0.0);
	for (i = 0; i < ARRAY_SIZE(funcs); i++) {
		glDepthFunc(funcs[i]);
		draw_rect_depth(10, 10 + 20 * i, 10, 10, 0.5);
		draw_rect_depth(30, 10 + 20 * i, 10, 10, 0.0);
		draw_rect_depth(50, 10 + 20 * i, 10, 10, -0.5);
	}

	for (i = 0; i < ARRAY_SIZE(funcs); i++) {
		glDepthFunc(funcs[i]);
		pass = pass && piglit_probe_pixel_rgb(15, 15 + 20 * i,
						      i & 1 ? blue : green);
		pass = pass && piglit_probe_pixel_rgb(35, 15 + 20 * i,
						      i & 2 ? blue : green);
		pass = pass && piglit_probe_pixel_rgb(55, 15 + 20 * i,
						      i & 4 ? blue : green);
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char**argv)
{
}
