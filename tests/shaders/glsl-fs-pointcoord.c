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

/** @file glsl-fs-pointcoord.c
 *
 * Tests that gl_PointCoord produces the expected output in a fragment shader
 * with point sprites enabled.
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 256;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static GLint prog;
static GLint point_size;

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	const float red[4] =    {1, 0, 0, 0};
	const float green[4] =  {0, 1, 0, 0};
	const float yellow[4] = {1, 1, 0, 0};
	const float black[4] =  {0, 0, 0, 0};

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	glPointSize(point_size);
	glBegin(GL_POINTS);
	glVertex2f(point_size / 2.0, point_size / 2.0);
	glEnd();

	pass = pass && piglit_probe_pixel_rgb(0, 0, green);
	pass = pass && piglit_probe_pixel_rgb(point_size - 1, 0, yellow);
	pass = pass && piglit_probe_pixel_rgb(0, point_size - 1, black);
	pass = pass && piglit_probe_pixel_rgb(point_size - 1, point_size - 1, red);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char**argv)
{
	GLint vs, fs;
	GLint point_size_limits[2];

	piglit_require_gl_version(20);
	piglit_require_extension("GL_ARB_point_sprite");

	glGetIntegerv(GL_ALIASED_POINT_SIZE_RANGE, point_size_limits);
	point_size = point_size_limits[1];

	if (point_size > piglit_width)
		point_size = piglit_width;
	if (point_size > piglit_height)
		point_size = piglit_height;

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-fs-pointcoord.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   "shaders/glsl-fs-pointcoord.frag");

	prog = piglit_link_simple_program(vs, fs);

	glEnable(GL_POINT_SPRITE);

	glUseProgram(prog);
}
