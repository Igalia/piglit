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

/** @file glsl-fs-fragcoord-zw-perspective.c
 *
 * Tests that gl_FragCoord.zw produces the expected output in a fragment shader
 * with a perspective projection.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 256;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

static GLfloat Znear = 1.0, Zfar = 10.0;


/**
 * Draw a quad where the bottom edge is on the near clip plane and the
 * top edge is on the far clip plane.  Adjust the far coordinates' X and Y
 * values to fill the window.  So the quad doesn't look like it's drawn in
 * perspective, but it is (the top is much wider than the bottom).
 */
static void
draw_quad(void)
{
	float xBotLeft = -Znear, xBotRight = Znear;
	float xTopLeft = -Zfar, xTopRight = Zfar;
	float yBotLeft = -Znear, yBotRight = -Znear;
	float yTopLeft = Zfar, yTopRight = Zfar;
	float zBottom = -Znear, zTop = -Zfar;
	float verts[4][4];

	verts[0][0] = xBotLeft;
	verts[0][1] = yBotLeft;
	verts[0][2] = zBottom;
	verts[0][3] = 1.0;
	verts[1][0] = xBotRight;
	verts[1][1] = yBotRight;
	verts[1][2] = zBottom;
	verts[1][3] = 1.0;
	verts[2][0] = xTopRight;
	verts[2][1] = yTopRight;
	verts[2][2] = zTop;
	verts[2][3] = 1.0;
	verts[3][0] = xTopLeft;
	verts[3][1] = yTopLeft;
	verts[3][2] = zTop;
	verts[3][3] = 1.0;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glEnableClientState(GL_VERTEX_ARRAY);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
}


/** Convert t in [0,1] into a 1/w value */
static float
t_to_w(float t)
{
	float zEye = -(Znear + t * (Zfar - Znear));
	float wClip = -zEye;
	return 1.0 / wClip;
}


enum piglit_result
piglit_display(void)
{
	float w0, w1;
	GLboolean pass = GL_TRUE;
	int y;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	draw_quad();

	/* Spot-test Z values */
	pass = piglit_probe_pixel_depth(0, 0, 0.0) && pass;
	pass = piglit_probe_pixel_depth(piglit_width-1, 0, 0.0) && pass;
	pass = piglit_probe_pixel_depth(piglit_width-2, piglit_height-1, 1.0) && pass;
	pass = piglit_probe_pixel_depth(1, piglit_height-2, 1.0) && pass;

	w0 = t_to_w(0.0);
	w1 = t_to_w(1.0);

	/* Test a column of pixel colors */
	for (y = 8; y < piglit_height; y += 16) {
		float expected[3];
		float t = y / (float) (piglit_height - 1);
		expected[0] = t;                   /* gl_FragCoord.z */
		expected[1] = w0 + t * (w1 - w0);  /* gl_FragCoord.w */
		expected[2] = 0.0;

		pass = piglit_probe_pixel_rgb(piglit_width/2, y, expected) & pass;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint vs, fs, prog;

	piglit_require_gl_version(20);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -1.0, 1.0, Znear, Zfar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-mvp.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   "shaders/glsl-fs-fragcoord-zw.frag");

	prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);

	glEnable(GL_DEPTH_TEST);
}
