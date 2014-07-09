/*
 * Copyright Â© 2009 Intel Corporation
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
 *   Ben Holmes <shranzel@hotmail.com>
 */

/* This test draws to the screen using glDrawPixels with data formats of GL_RED
 * and GL_RG and tests for the correct color output.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_rg");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glEnable(GL_TEXTURE_2D);
	glClearColor(0.2, 0.2, 0.2, 1.0);
}

enum piglit_result
piglit_display(void)
{
	static const GLfloat red[3] = {1.0, 0.0, 0.0};
	static const GLfloat rg[3] = {1.0, 1.0, 0.0};
	static const GLfloat black[3] = {0.0, 0.0, 0.0};
	#define height 16
	#define width 16
	int i, j;
	GLfloat texData[width][height][4];
	GLboolean pass = GL_TRUE;

	glClear(GL_COLOR_BUFFER_BIT);


	for (i=0; i < width; ++i) {
		for (j=0; j < height; ++j) {
			if ((i+j) & 1) {
				texData[i][j][0] = 1.0;
				texData[i][j][1] = 1.0;
				texData[i][j][2] = 1.0;
				texData[i][j][3] = 1.0;
			}
			else {
				texData[i][j][0] = 0.0;
				texData[i][j][1] = 0.0;
				texData[i][j][2] = 0.0;
				texData[i][j][3] = 0.0;
			}
		}
	}

	/* The image consists of alternating patterns of 4 0.0 values followed
	 * by 4 1.0 values.  When drawn as GL_RED, this results in 4x4 pixel
	 * blocks of black and red.  When drawn as GL_RG, this results in 2x2
	 * pixel blocks of gold (full green and full red).
	 */
	glRasterPos2i(0,0);
	glDrawPixels(width,height,GL_RED,GL_FLOAT,texData);

	glRasterPos2i(18,0);
	glDrawPixels(width,height,GL_RG,GL_FLOAT,texData);

	pass = pass && piglit_probe_pixel_rgb(0,0,black);
	pass = pass && piglit_probe_pixel_rgb(2,0,black);
	pass = pass && piglit_probe_pixel_rgb(4,0,red);
	pass = pass && piglit_probe_pixel_rgb(6,0,red);

	pass = pass && piglit_probe_pixel_rgb(18,0,black);
	pass = pass && piglit_probe_pixel_rgb(19,0,black);
	pass = pass && piglit_probe_pixel_rgb(20,0,rg);
	pass = pass && piglit_probe_pixel_rgb(21,0,rg);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;

	#undef height
	#undef width
}
