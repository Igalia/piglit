/*
 * Copyright © 2012 Intel Corporation
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

/** @file glsl-render-bad-attach.c
 *
 * Tests that rendering with a good program after attaching a bad
 * shader to it still works.
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
	"}\n";

const char *good_fs_source =
	"void main()\n"
	"{\n"
	"	gl_FragColor = vec4(0.0, 1.0, 0.0, 0.0);\n"
	"}\n";

const char *bad_fs_source =
	"void BAD_FUNC()\n"
	"{\n"
	"	BAD;\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	bool pass = GL_TRUE;
	GLint ok;
	float green[4] = {0, 1, 0, 0};
	GLuint good_fs, bad_fs, vs, prog;

	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);
	good_fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
					     good_fs_source);
	prog = piglit_link_simple_program(vs, good_fs);
	if (!vs || !good_fs || !prog)
		piglit_report_result(PIGLIT_FAIL);

	glUseProgram(prog);

	piglit_draw_rect(-1, -1, 1, 2);

	bad_fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(bad_fs, 1, (const GLchar **) &bad_fs_source, NULL);
	glCompileShader(bad_fs);
	glGetShaderiv(bad_fs, GL_COMPILE_STATUS, &ok);
	if (ok)
		piglit_report_result(PIGLIT_FAIL);
	glAttachShader(prog, bad_fs);

	piglit_draw_rect(0, -1, 1, 2);

	pass = piglit_probe_rect_rgba(0, 0,
				      piglit_width, piglit_height,
				      green);

	piglit_present_results();

	glDeleteShader(good_fs);
	glDeleteShader(bad_fs);
	glDeleteShader(vs);
	glDeleteProgram(prog);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_GLSL();
}
