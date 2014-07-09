/*
 * Copyright © 2009-2010 Intel Corporation
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
 *    Ian Romanick <idr@freedesktop.org>
 *
 */

/** @file glsl-fs-loop.c
 *
 * Tests that loops in the fragment shader work.
 *
 * Since a value from an attribute is used as a loop counter, the compiler
 * cannot simply unroll the loop.  This verifies that GLSL loops can be
 * correctly generated in the vertex shader.
 *
 * This was conceived as a test case for freedesktop.org bug #25173.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static int color_location;
static GLint prog;

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;

	float color[] = {1.0, 0.0, 0.0, 0.0};

	unsigned i;
	unsigned j;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; i < 3; i++) {
		float temp;
		float line_color[4];
		unsigned x = 5 + (25 * i);

		memcpy(line_color, color, sizeof(line_color));
		for (j = 0; j < 3; j++) {
			unsigned y = 5 + (25 * j);

			line_color[3] = (float) j;

			glUniform4fv(color_location, 1, line_color);
			piglit_draw_rect(x, y, 20, 20);

			pass &= piglit_probe_pixel_rgb(x + 5, y + 5, color);

			temp = line_color[2];
			line_color[2] = line_color[1];
			line_color[1] = line_color[0];
			line_color[0] = temp;
		}

		temp = color[2];
		color[2] = color[1];
		color[1] = color[0];
		color[0] = temp;
	}


	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint vs, fs;

	/* Set up projection matrix so we can just draw using window
	 * coordinates.
	 */
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_gl_version(20);

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-mvp.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   "shaders/glsl-fs-loop.frag");

	prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);

	color_location = glGetUniformLocation(prog, "color");
}
