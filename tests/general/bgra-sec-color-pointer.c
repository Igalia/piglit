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
 *    Ben Holmes <shranzel@hotmail.com>
 */

/*
 * this test draws quads with RGBA and BGRA formats using
 * glSecondaryColorPointer. Two quads are drawn without blending and two
 * with alpha blending.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLfloat verts[12] = {225.0, 175.0, 0.0,
				225.0, 225.0, 0.0,
				175.0, 175.0, 0.0,
				175.0, 225.0, 0.0};


static GLubyte colors[16] = {255, 0, 0, 127,
				255, 0, 0, 127,
				255, 0, 0, 127,
				255, 0, 0, 127};


void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(14);

	piglit_require_extension("GL_EXT_vertex_array_bgra");

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glEnable(GL_COLOR_SUM);
	glColor3f(0.0, 0.0, 0.0);

	glClearColor(0.6, 0.6, 0.6, 1.0);
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLfloat red[3]={1.0, 0.0, 0.0};
	GLfloat blue[3]={0.0, 0.0, 1.0};
	GLfloat greyRed[3]={1.0, 0.6, 0.6};
	GLfloat greyBlue[3]={0.6, 0.6, 1.0};

	glClear(GL_COLOR_BUFFER_BIT);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_SECONDARY_COLOR_ARRAY);

	glSecondaryColorPointer(3, GL_UNSIGNED_BYTE, 4, colors);
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glPushMatrix();
	glTranslatef(75.0, 0.0, 0.0);

	glSecondaryColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, 0, colors);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glPopMatrix();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glPushMatrix();
	glTranslatef(0.0, -75.0, 0.0);

	glSecondaryColorPointer(3, GL_UNSIGNED_BYTE, 4, colors);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glPushMatrix();
	glTranslatef(75.0, 0.0, 0.0);

	glSecondaryColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, 0, colors);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glPopMatrix();
	glPopMatrix();

	pass = pass && piglit_probe_pixel_rgb(200, 200, red);
	pass = pass && piglit_probe_pixel_rgb(275, 200, blue);
	pass = pass && piglit_probe_pixel_rgb(200, 125, greyRed);
	pass = pass && piglit_probe_pixel_rgb(275, 125, greyBlue);

	glFinish();
	piglit_present_results();

	glDisable(GL_BLEND);
	glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
