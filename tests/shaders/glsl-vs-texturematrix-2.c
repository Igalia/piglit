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

/** @file glsl-vs-texturematrix-2.c
 *
 * Tests that we can access gl_TextureMatrix[n] in the vertex shader.
 *
 * Compared to glsl-vs-texturematrix-1, this uses varying access of the array.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLint prog;

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	static const float red[]   = {1.0, 0.0, 0.0, 1.0};
	static const float green[] = {0.0, 1.0, 0.0, 1.0};
	static const float blue[]  = {0.0, 0.0, 1.0, 1.0};
	static const float white[] = {1.0, 1.0, 1.0, 1.0};
	GLuint tex;

	glActiveTexture(GL_TEXTURE1);
	tex = piglit_rgbw_texture(GL_RGBA, 8, 8, GL_FALSE, GL_FALSE, GL_UNSIGNED_NORMALIZED);
	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glRotatef(90, 0, 0, 1);

	piglit_draw_rect(-1, -1, 2, 2);

	pass = piglit_probe_pixel_rgba(piglit_width * 1 / 4,
				       piglit_height * 1 / 4,
				       blue) && pass;
	pass = piglit_probe_pixel_rgba(piglit_width * 3 / 4,
				       piglit_height * 1 / 4,
				       red) && pass;
	pass = piglit_probe_pixel_rgba(piglit_width * 1 / 4,
				       piglit_height * 3 / 4,
				       white) && pass;
	pass = piglit_probe_pixel_rgba(piglit_width * 3 / 4,
				       piglit_height * 3 / 4,
				       green) && pass;

	piglit_present_results();

	glDeleteTextures(1, &tex);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLint vs, fs;
	int loc;

	piglit_require_gl_version(20);

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-vs-texturematrix-2.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   "shaders/glsl-tex.frag");

	prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);

	loc = glGetUniformLocation(prog, "sampler");
	glUniform1i(loc, 1);
	loc = glGetUniformLocation(prog, "i");
	glUniform1f(loc, 1.0);
}
