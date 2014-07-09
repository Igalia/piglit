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

/** @file glsl-fs-sampler-numbering.c
 *
 * Tests that drivers correctly use the sampler uniform's value to
 * look up which gl texture unit is being used.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static int sampler_location;
static GLint prog;

static const float white[4] = {1, 1, 1, 1};
static const float black[4] = {0, 0, 0, 0};

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	/* result = 2 ^ args1 + args2 */
	int x1 = piglit_width / 4;
	int x2 = piglit_width * 3 / 4;
	int y1 = piglit_height / 4;
	int y2 = piglit_height * 3 / 4;

	piglit_draw_rect_tex(0, 0, piglit_width, piglit_height,
			     0, 0, 1, 1);

	pass &= piglit_probe_pixel_rgb(x1, y1, black);
	pass &= piglit_probe_pixel_rgb(x2, y1, white);
	pass &= piglit_probe_pixel_rgb(x1, y2, white);
	pass &= piglit_probe_pixel_rgb(x2, y2, black);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint vs, fs;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_gl_version(20);

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-tex-mvp.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   "shaders/glsl-tex.frag");

	prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);

	sampler_location = glGetUniformLocation(prog, "sampler");
	glUniform1i(sampler_location, 1);

	glActiveTexture(GL_TEXTURE1);
	piglit_checkerboard_texture(0, 0, 2, 2, 1, 1, black, white);
	glEnable(GL_TEXTURE_2D);
}
