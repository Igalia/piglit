/*
 * Copyright © 2010 Intel Corporation
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

/** @file bump.c
 *
 * Test of ATI_envmap_bumpmap texture combiners.
 */

#include "piglit-util-gl.h"

#define TEXSIZE 32

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

GLvoid
draw_rect_tex(float x, float y, float w, float h,
	      float tx, float ty, float tw, float th)
{
	float verts[4][4];
	float tex[4][2];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	tex[0][0] = tx;
	tex[0][1] = ty;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	tex[1][0] = tx + tw;
	tex[1][1] = ty;
	verts[2][0] = x + w;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	tex[2][0] = tx + tw;
	tex[2][1] = ty + th;
	verts[3][0] = x;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;
	tex[3][0] = tx;
	tex[3][1] = ty + th;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glEnableClientState(GL_VERTEX_ARRAY);

	glClientActiveTexture(GL_TEXTURE1);
	glTexCoordPointer(2, GL_FLOAT, 0, tex);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint tex;
	float red[4]   = {1.0, 0.0, 0.0, 1.0};
	float green[4] = {0.0, 1.0, 0.0, 1.0};
	float blue[4]  = {0.0, 0.0, 1.0, 1.0};
	float white[4] = {1.0, 1.0, 1.0, 1.0};

	tex = piglit_rgbw_texture(GL_RGBA, TEXSIZE, TEXSIZE,
				  GL_FALSE, GL_FALSE, GL_UNSIGNED_NORMALIZED);
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);

	draw_rect_tex(-1, -1, 2, 2,
		      0, 0, 1, 1);

	pass = pass && piglit_probe_rect_rgba(0,
					      0,
					      piglit_width / 2,
					      piglit_height / 2,
					      red);
	pass = pass && piglit_probe_rect_rgba(piglit_width / 2,
					      0,
					      piglit_width / 2,
					      piglit_height / 2,
					      green);
	pass = pass && piglit_probe_rect_rgba(0,
					      piglit_height / 2,
					      piglit_width / 2,
					      piglit_height / 2,
					      blue);
	pass = pass && piglit_probe_rect_rgba(piglit_width / 2,
					      piglit_height / 2,
					      piglit_width / 2,
					      piglit_height / 2,
					      white);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	if (piglit_get_gl_version() < 12) {
		piglit_report_result(PIGLIT_SKIP);
	}
}
