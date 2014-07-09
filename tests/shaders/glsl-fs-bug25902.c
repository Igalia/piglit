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
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file glsl-fs-bug25902.c
 *
 * Tests for a hang in the i965 driver with a masked texture sample operation
 * in the GLSL path.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static int args_location, tex_location;
static GLint prog;

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	static const float args[4] = {0.0, 1.0, 0.0, 1.0};
	float black[4] = {0.0, 0.0, 0.0, 0.0};

	glClearColor(0.5, 0.5, 0.5, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUniform4fv(args_location, 1, args);
	glUniform1i(tex_location, 0);
	glColor4f(1.0, 0.0, 0.0, 0.0);
	piglit_draw_rect_tex(10, 10, 10, 10,
			     0, 0, 1, 1);

	pass &= piglit_probe_pixel_rgb(12, 12, black);
	pass &= piglit_probe_pixel_rgb(17, 12, args);
	pass &= piglit_probe_pixel_rgb(12, 17, args);
	pass &= piglit_probe_pixel_rgb(17, 17, black);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint vs, fs;
	float black[4] = {0.0, 0.0, 0.0, 0.0};
	float white[4] = {1.0, 1.0, 1.0, 0.0};

	piglit_require_gl_version(20);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-tex-mvp.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   "shaders/glsl-fs-bug25902.frag");

	prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);

	piglit_checkerboard_texture(0, 0,
				    2, 2,
				    1, 1,
				    black, white);

	args_location = glGetUniformLocation(prog, "args");
	tex_location = glGetUniformLocation(prog, "sampler");
}
