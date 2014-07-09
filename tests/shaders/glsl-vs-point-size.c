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

/** @file glsl-vs-point-size.c
 *
 * Tests whether a vertex shader can change the point size by writing
 * to gl_PointSize.
 *
 * Bug #27250
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLint prog;

static const float white[4] = {1, 1, 1, 1};
static const float black[4] = {0, 0, 0, 0};

#define POINT_SIZE 16

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float vert[2] = { piglit_width / 2, piglit_height / 2 };

	/* Clear the window to black */
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw a single white point at the centre of the window. The
	   vertex shader should make this larger */
	glColor3fv(white);
	glVertexPointer(2, GL_FLOAT, 0, vert);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_POINTS, 0, 1);
	glDisableClientState(GL_VERTEX_ARRAY);

	/* Verify that the point is large by looking at some corner pixels */
	pass &= piglit_probe_pixel_rgb(piglit_width / 2 - POINT_SIZE / 2 + 1,
				       piglit_height / 2 - POINT_SIZE / 2 + 1,
				       white);
	pass &= piglit_probe_pixel_rgb(piglit_width / 2 + POINT_SIZE / 2 - 1,
				       piglit_height / 2 + POINT_SIZE / 2 - 1,
				       white);
	/* Sanity check that the corners of the window aren't filled */
	pass &= piglit_probe_pixel_rgb(0, 0, black);
	pass &= piglit_probe_pixel_rgb(piglit_width - 1, piglit_height - 1,
				       black);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint vs;
	GLint point_size_range[2];

	piglit_require_gl_version(20);
	/* If the driver doesn't claim to support the point size the
	   shader is going to set then we should skip the test */
	glGetIntegerv(GL_ALIASED_POINT_SIZE_RANGE, point_size_range);
	if (POINT_SIZE < point_size_range[0] ||
	    POINT_SIZE > point_size_range[1]) {
		printf("Point size %i not supported\n", POINT_SIZE);
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-vs-point-size.vert");

	prog = piglit_link_simple_program(vs, 0);

	glUseProgram(prog);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
}
