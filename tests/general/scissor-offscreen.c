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
 *
 * Authors:
 *    Neil Roberts <neil@linux.intel.com>
 *
 */

/** @file scissor-offscreen.c
 *
 * Test case for setting a scissor that is entirely offscreen. This
 * should clip everything but under current Mesa master with i965 it
 * clips nothing
 *
 * Bug #27643
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	static const GLfloat window_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLboolean pass = GL_TRUE;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* Clear to white */
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Set a fully offscreen scissor. This should clip everything */
	glEnable(GL_SCISSOR_TEST);
	glScissor(0, piglit_height, 0, 0);

	/* Fill the window with red */
	glColor3f (1.0f, 0.0f, 0.0f);
	piglit_draw_rect(0, 0, piglit_width, piglit_height);

	glDisable(GL_SCISSOR_TEST);

	/* Verify some pixels around the window */
	pass = pass && piglit_probe_pixel_rgb(0, 0, window_color);
	pass = pass && piglit_probe_pixel_rgb(piglit_width / 2, piglit_height / 2,
				      window_color) && pass;
	pass = pass && piglit_probe_pixel_rgb(piglit_width - 1, piglit_height - 1,
				      window_color) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char**argv)
{
}
