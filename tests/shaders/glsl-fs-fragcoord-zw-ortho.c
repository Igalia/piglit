/*
 * Copyright © 2009 Intel Corporation
 * Copyright © 2011 VMware, Inc.
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
 *    Brian Paul
 *
 */

/** @file glsl-fs-fragcoord-zw-ortho.c
 *
 * Tests that gl_FragCoord.zw produces the expected output in a fragment shader
 * with an orthographic projection
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 256;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

static void
draw_quad(float x, float y, float w, float h)
{
	float zLeft = -1.0, zRight = 1.0;
	float verts[4][4];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = zLeft;
	verts[0][3] = 1.0;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = zRight;
	verts[1][3] = 1.0;
	verts[2][0] = x + w;
	verts[2][1] = y + h;
	verts[2][2] = zRight;
	verts[2][3] = 1.0;
	verts[3][0] = x;
	verts[3][1] = y + h;
	verts[3][2] = zLeft;
	verts[3][3] = 1.0;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glEnableClientState(GL_VERTEX_ARRAY);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
}



enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int x, y;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	draw_quad(0, 0, piglit_width, piglit_height);

	/* Test Z values */
	pass = piglit_probe_pixel_depth(0, 0, 1.0) && pass;
	pass = piglit_probe_pixel_depth(piglit_width-1, 0, 0.0) && pass;
	pass = piglit_probe_pixel_depth(piglit_width-1, piglit_height-1, 0.0) && pass;
	pass = piglit_probe_pixel_depth(0, piglit_height-1, 1.0) && pass;

	/* Test colors */
	for (y = 8; y < piglit_height && pass; y += 16) {
		for (x = 8; x < piglit_width; x += 16) {
			float color[3];

			color[0] = 1.0 - x / 256.0;  /* gl_FragCoord.z */
			color[1] = 1.0;              /* gl_FragCoord.w */
			color[2] = 0;

			pass &= piglit_probe_pixel_rgb(x, y, color);
			if (!pass)
				break;
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint vs, fs, prog;

	piglit_require_gl_version(20);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-mvp.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   "shaders/glsl-fs-fragcoord-zw.frag");

	prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);

	glEnable(GL_DEPTH_TEST);
}
