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
 *    Shuang he <shuang.he@intel.com>
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	(void) argc;
	(void) argv;
}

enum piglit_result
piglit_display(void)
{
	GLubyte stencil_rect[20 * 20];
	int i;
	GLboolean pass = GL_TRUE;
	static float red[] = {1.0, 0.0, 0,0, 0.0};
	static float black[] = {0.0, 0.0, 0,0, 0.0};

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearStencil(0);
	glClearDepth(1.0);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glDisable(GL_DITHER);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 20);

	for (i=0; i<20*20; i++)
		if (i < 20*20/2)
			stencil_rect[i] = 1;
		else
			stencil_rect[i] = 0;


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_LESS, 0, (GLuint)~0);
	glRasterPos2i(50, 50);
	glDrawPixels(20, 20, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencil_rect);
	glColor3f(1.0, 0.0, 0.0);
	glRectf(50, 50, 50+20, 50+20);
	glDisable(GL_STENCIL_TEST);

	pass &= piglit_probe_rect_rgb(50, 50, 20, 10, red);
	pass &= piglit_probe_rect_rgb(50, 60, 20, 10, black);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
