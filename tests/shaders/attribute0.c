/*
 * Copyright © 2010, 2011 Intel Corporation
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
 * \file attribute0.c
 * Verify that applications can use attribute 0 with a user-defined attribute
 * instead of using gl_Vertex.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_text[] =
	"attribute vec4 vertex;\n"
	"void main() { gl_Position = vertex; }\n"
	;

static const char fs_text[] =
	"void main() { gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); }\n"
	;

static GLuint prog;

static const float green[] = { 0.0, 1.0, 0.0, 1.0 };
static const float blue[] = { 0.0, 0.0, 1.0, 1.0 };

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;

	glClear(GL_COLOR_BUFFER_BIT);

	piglit_draw_rect(-1.0, -1.0, 1.0, 2.0);

	pass &= piglit_probe_pixel_rgb(piglit_width / 4, piglit_height / 2,
				       green);
	pass &= piglit_probe_pixel_rgb(piglit_width * 3 / 4, piglit_height / 2,
				       blue);

	assert(!glGetError());

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint vs;
	GLuint fs;
	GLboolean ok;

	piglit_require_vertex_shader();
	piglit_require_fragment_shader();

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);
	prog = piglit_link_simple_program(vs, fs);

	glBindAttribLocation(prog, 0, "vertex");
	glLinkProgram(prog);
	ok = piglit_link_check_status(prog);
	if (!ok)
		piglit_report_result(PIGLIT_FAIL);

	glUseProgram(prog);

	glClearColor(blue[0], blue[1], blue[2], blue[3]);
}
