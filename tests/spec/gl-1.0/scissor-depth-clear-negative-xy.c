/*
 * Copyright © 2009, 2016 Intel Corporation
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
 *    Matt Turner <mattst88@gmail.com>
 *
 */

/** @file
 *
 * Tests that glScissor properly affects glClear(GL_DEPTH_BUFFER_BIT) when
 * x or y is negative.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	static float green[] = {0.0, 1.0, 0.0, 0.0};
	static float blue[]  = {0.0, 0.0, 1.0, 0.0};

	/* whole window green -- depth fail will be this. */
	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Clear depth to 0 (fail) */
	glClearDepth(0.0);
	glClear(GL_DEPTH_BUFFER_BIT);

	/* Clear depth quad the size of the fb at -16, -16 to be drawn blue. */
	glEnable(GL_SCISSOR_TEST);
	glScissor(-16, -16, piglit_width, piglit_height);
	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT);

	/* Now draw a quad midway between 0.0 and 1.0 depth so only that
	 * scissored depth clear will get rasterized.
	 */
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
	glDepthFunc(GL_LESS);
	glColor4fv(blue);
	piglit_draw_rect(0, 0, piglit_width, piglit_height);

	pass &= piglit_probe_rect_rgb(0, 0, piglit_width - 16, piglit_height - 16, blue);

	pass &= piglit_probe_rect_rgb(piglit_width - 16, 0, 16, piglit_height, green);
	pass &= piglit_probe_rect_rgb(0, piglit_height - 16, piglit_width - 16, 16, green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
