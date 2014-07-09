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
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file glsl-vs-statechange-1.c
 *
 * Tests for a bug that appeared to exist in Mesa 7.11 where the
 * constant attribute color from the previous fixed function setup
 * would be used, but was hidden by multiple state updates occurring
 * per draw call.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

const char *vs_source =
	"void main()\n"
	"{\n"
	"	gl_Position = gl_Vertex;\n"
	"	gl_FrontColor = vec4(0.0, 0.0, 1.0, 0.0);\n"
	"}\n";

static GLint prog;

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	float green[4] = {0, 1, 0, 0};
	float blue[4] =  {0, 0, 1, 0};

	glColor4fv(green);
	piglit_draw_rect(-1, -1, 1, 2);

	glUseProgram(prog);
	piglit_draw_rect(0, -1, 1, 2);

	glUseProgram(0);

	pass &= piglit_probe_rect_rgba(0, 0,
				       piglit_width / 2, piglit_height, green);
	pass &= piglit_probe_rect_rgba(piglit_width / 2, 0,
				       piglit_width / 2, piglit_height, blue);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint vs;

	piglit_require_gl_version(20);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);

	prog = piglit_link_simple_program(vs, 0);

	glDeleteShader(vs);
}
