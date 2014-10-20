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

/** @file stencil-drawpixels.c
 *
 * Tests that glDrawPixels(GL_STENCIL) minimally works.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int i;
	static float red[]   = {1.0, 0.0, 0.0, 0.0};
	static float green[] = {0.0, 1.0, 0.0, 0.0};
	static float blue[]  = {0.0, 0.0, 1.0, 0.0};
	static float square[100];

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* whole window gray -- none should be visible */
	glClearColor(0.5, 0.5, 0.5, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Clear stencil to 0, which will be drawn red */
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);

	/* quad at 10, 10 that will be drawn green. */
	for (i = 0; i < 100; i++)
		square[i] = 1.0;
	glRasterPos2i(10, 10);
	glDrawPixels(10, 10, GL_STENCIL_INDEX, GL_FLOAT, square);

	/* quad at 30, 10 that will be drawn blue. */
	for (i = 0; i < 100; i++)
		square[i] = 2.0;
	glRasterPos2i(30, 10);
	glDrawPixels(10, 10, GL_STENCIL_INDEX, GL_FLOAT, square);

	glDisable(GL_SCISSOR_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	/* First quad -- stencil == 0 gets red */
	glStencilFunc(GL_EQUAL, 0, ~0);
	glColor4fv(red);
	piglit_draw_rect(0, 0, piglit_width, piglit_height);

	/* Second quad -- stencil == 1 gets green */
	glStencilFunc(GL_EQUAL, 1, ~0);
	glColor4fv(green);
	piglit_draw_rect(0, 0, piglit_width, piglit_height);

	/* Last quad -- stencil == 2 gets blue */
	glStencilFunc(GL_EQUAL, 2, ~0);
	glColor4fv(blue);
	piglit_draw_rect(0, 0, piglit_width, piglit_height);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	pass &= piglit_probe_rect_rgb(0, 0, piglit_width, 10, red);

	pass &= piglit_probe_rect_rgb(0, 10, 10, 10, red);
	pass &= piglit_probe_rect_rgb(10, 10, 10, 10, green);
	pass &= piglit_probe_rect_rgb(20, 10, 10, 10, red);
	pass &= piglit_probe_rect_rgb(30, 10, 10, 10, blue);
	pass &= piglit_probe_rect_rgb(40, 10, piglit_width-40, 10, red);

	pass &= piglit_probe_rect_rgb(0, 20, piglit_width, piglit_height - 20,
				      red);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
}
