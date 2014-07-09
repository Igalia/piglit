/*
 * Copyright © 2009,2013 Intel Corporation
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
 * Tests that gl_PointCoord produces the expected output in a GLES2
 * context, which treats all points as point sprite enabled (so
 * gl_PointCoord returns defined values).
 *
 * To compare, the GLSL 4.3 spec says:
 *
 *     "The values in gl_PointCoord are two-dimensional coordinates
 *      indicating where within a point primitive the current fragment
 *      is located, when point sprites are enabled."
 *
 * (which is tested in tests/shaders/glsl-fs-pointcoord.c) while the
 * GLSL ES 1.00 spec says:
 *
 *     "The values in gl_PointCoord are two-dimensional coordinates
 *      indicating where within a point primitive the current fragment
 *      is located."
 *
 * which makes sense, because the GL_POINT_SPRITE enable doesn't
 * exist.
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;

	config.window_width = 256;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static GLint prog;
static GLint point_size;

const char *vs_source =
	"attribute vec4 vertex;\n"
	"uniform float point_size;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vertex;\n"
	"	gl_PointSize = point_size;\n"
	"}\n";

const char *fs_source =
	"void main()\n"
	"{\n"
	"	gl_FragColor = vec4(gl_PointCoord.xy * 1.1 - 0.05, 0, 1);\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	const float red[4] =    {1, 0, 0, 1};
	const float green[4] =  {0, 1, 0, 1};
	const float yellow[4] = {1, 1, 0, 1};
	const float black[4] =  {0, 0, 0, 1};
	const float vert[] = {
		-1.0 + 2.0 * (point_size / 2.0) / piglit_width,
		-1.0 + 2.0 * (point_size / 2.0) / piglit_height
	};
	GLint point_size_loc;

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	point_size_loc = glGetUniformLocation(prog, "point_size");
	glUniform1f(point_size_loc, point_size);

	glVertexAttribPointer(PIGLIT_ATTRIB_POS, 2, GL_FLOAT, false, 0, vert);
	glEnableVertexAttribArray(PIGLIT_ATTRIB_POS);
	glDrawArrays(GL_POINTS, 0, 1);

	pass = pass && piglit_probe_pixel_rgba(0, 0, green);
	pass = pass && piglit_probe_pixel_rgba(point_size - 1, 0, yellow);
	pass = pass && piglit_probe_pixel_rgba(0, point_size - 1, black);
	pass = pass && piglit_probe_pixel_rgba(point_size - 1, point_size - 1,
					       red);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char**argv)
{
	GLint point_size_limits[2];

	glGetIntegerv(GL_ALIASED_POINT_SIZE_RANGE, point_size_limits);
	point_size = point_size_limits[1];

	if (point_size > piglit_width)
		point_size = piglit_width;
	if (point_size > piglit_height)
		point_size = piglit_height;

	prog = piglit_build_simple_program(vs_source, fs_source);

	glUseProgram(prog);
}
