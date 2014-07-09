/*
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */


/**
 * Test glDrawArrays with glColorMaterial.
 * This exercises a Mesa bug where glColor() calls didn't effect
 * the color of lit surfaces when color material mode was used.
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define DX0 -0.6

static GLfloat pos0[4][3] =
{
	{  0.5 + DX0, -0.5, 0.0 },
	{  0.5 + DX0,  0.5, 0.0 },
	{  -0.5 + DX0,  0.5, 0.0 },
	{  -0.5 + DX0, -0.5, 0.0 }
};

#define DX1 0.6

static GLfloat pos1[4][3] =
{
	{  0.5 + DX1, -0.5, 0.0 },
	{  0.5 + DX1,  0.5, 0.0 },
	{  -0.5 + DX1,  0.5, 0.0 },
	{  -0.5 + DX1, -0.5, 0.0 }
};

static GLfloat norms[4][3] =
{
	{ 0, 0, 1},
	{ 0, 0, 1},
	{ 0, 0, 1},
	{ 0, 0, 1}
};


enum piglit_result
piglit_display(void)
{
	static const GLfloat red[3] = {1.0f, 0.0f, 0.0f};
	static const GLfloat green[3] = {0.0f, 1.0f, 0.0f};
	GLboolean pass = GL_TRUE;

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	glClearColor(0.3, 0.3, 0.3, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	/* red */
	glColor3f(1, 0, 0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, pos0);
	glNormalPointer(GL_FLOAT, 0, norms);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	/* green */
	glColor3f(0, 1, 0);
	glVertexPointer(3, GL_FLOAT, 0, pos1);
	glNormalPointer(GL_FLOAT, 0, norms);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glDisable(GL_LIGHTING);

	if (!piglit_probe_pixel_rgb(piglit_width * 1 / 3,
				    piglit_height / 2, red)) {
		pass = GL_FALSE;
	}

	if (!piglit_probe_pixel_rgb(piglit_width * 2 / 3,
				    piglit_height / 2, green)) {
		pass = GL_FALSE;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	/* nop */
}
