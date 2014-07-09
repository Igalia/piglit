/*
 * Copyright © 2011 Intel Corporation
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
 *    Shuang He <shuang.he@intel.com>
 *
 */

/** @file clear_accum.c
 *
 * Tests that clear accum buffer
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_ACCUM;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	GLboolean pass = GL_TRUE;
	static float green[] = {0.0, 1.0, 0.0, 0.0};

	glClearAccum(0, 1, 0, 0);
	glClear(GL_ACCUM_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glAccum(GL_RETURN, 1.0);
	pass &= piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height, green);

	if (pass)
		piglit_report_result(PIGLIT_PASS);
	else
		piglit_report_result(PIGLIT_FAIL);
}
