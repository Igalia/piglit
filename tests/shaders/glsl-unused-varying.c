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

/** @file glsl-unused-varying.c
 *
 * Tests that a vertex/fragment program combination with a varying that's
 * unused gets the right varying contents for the one that is used.
 *
 * This reveals a bug in the 965 brw_wm_glsl.c code.  Note that the
 * conditional in the fragment shader is required to trigger brw_wm_glsl.c.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static int do_red_location;
static int red_location;
static int green_location;
static GLint prog;

enum piglit_result
piglit_display(void)
{
	static const float red[] = {1.0, 0.0, 0.0, 0.0};
	static const float green[] = {0.0, 1.0, 0.0, 0.0};
	GLboolean pass = GL_TRUE;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	glUniform4fv(red_location, 1, red);
	glUniform4fv(green_location, 1, green);

	glUniform1i(do_red_location, 1);
	piglit_draw_rect(10, 10, 10, 10);

	glUniform1i(do_red_location, 0);
	piglit_draw_rect(10, 30, 10, 10);

	pass &= piglit_probe_pixel_rgb(15, 15, red);
	pass &= piglit_probe_pixel_rgb(15, 35, green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLint vs, fs;

	piglit_require_gl_version(20);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-unused-varying.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   "shaders/glsl-unused-varying.frag");

	prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);

	red_location = glGetUniformLocation(prog, "red");
	green_location = glGetUniformLocation(prog, "green");
	do_red_location = glGetUniformLocation(prog, "do_red");
}
