/*
 * Copyright © 2013 Intel Corporation
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
 * \file clear.c
 * Verify that glClear uses the scissor rectangle from viewport 0.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLint num_viewports;

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_viewport_array");
	glGetIntegerv(GL_MAX_VIEWPORTS, &num_viewports);
}

enum piglit_result
piglit_display(void)
{
	static const float expected[] = {
		0.0f, 1.0f, 0.0f, 1.0f
	};

	const int slice_height = (piglit_height + num_viewports - 2)
		/ (num_viewports - 1);

	bool pass = false;
	int i;

	glDisable(GL_SCISSOR_TEST);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glScissorIndexed(0, 0, 0, piglit_width, piglit_height);
	glEnablei(GL_SCISSOR_TEST, 0);

	for (i = 1; i < num_viewports; i++) {
		glEnablei(GL_SCISSOR_TEST, i);
		glScissorIndexed(i,
				 0, i * slice_height,
				 piglit_width, slice_height);
	}

	glClearColor(0.0, 1.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      expected);

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
