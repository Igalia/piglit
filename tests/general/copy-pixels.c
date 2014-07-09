/*
 * Copyright © 2012 Intel Corporation
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
 * @file copy-pixels.c
 *
 * Test to verify glCopyPixels with GL_COLOR, GL_DEPTH and GL_STENCIL
 *
 * Author: Anuj Phogat
 */

#include "piglit-util-gl.h"

#define IMAGE_WIDTH 16
#define IMAGE_HEIGHT 16

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_STENCIL | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	GLint i, j;
	GLuint width = 16, height = 16;
	GLint x = 12, y = 12;
	GLfloat buf[IMAGE_WIDTH][IMAGE_HEIGHT];
	GLfloat depth_val = 0.75, stencil_val = 2.0;
	GLfloat green[4] = {0.0, 1.0, 0.0, 0.0};
	bool pass = true;

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor4fv(green);
	piglit_draw_rect(0, 0, width, height);

	glRasterPos2i(x, y);
	glCopyPixels(0, 0, width, height, GL_COLOR);
	pass = pass && piglit_probe_rect_rgba(x, y, width, height, green);

	/* Initialize depth data */
	for (i = 0; i < IMAGE_HEIGHT; i++) {
		for(j = 0; j < IMAGE_WIDTH; j++)
			buf[i][j] = depth_val;
	}

	glClearDepth(0.0);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	glRasterPos2i(0, 0);
	glDrawPixels(width, height,
		     GL_DEPTH_COMPONENT, GL_FLOAT, buf);
	glRasterPos2i(x, y);
	glCopyPixels(0, 0, width, height, GL_DEPTH);
	pass = piglit_probe_rect_depth(x, y, width, height, depth_val)
	       && pass;

	/* Initialize stencil data */
	for (i = 0; i < IMAGE_HEIGHT; i++) {
		for(j = 0; j < IMAGE_WIDTH; j++)
			buf[i][j] = stencil_val;
	}

	glClearStencil(0.0);
	glClear(GL_STENCIL_BUFFER_BIT);

	glRasterPos2i(0, 0);
	glDrawPixels(width, height,
		     GL_STENCIL_INDEX, GL_FLOAT, buf);
	glRasterPos2i(x, y);
	glCopyPixels(0, 0, width, height, GL_STENCIL);
	pass = piglit_probe_rect_stencil(x, y, width, height, stencil_val)
	       && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	if ((piglit_get_gl_version() < 14) &&
	    !piglit_is_extension_supported("GL_ARB_window_pos")) {
		printf("Requires GL 1.4 or GL_ARB_window_pos");
		piglit_report_result(PIGLIT_SKIP);
	}
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
