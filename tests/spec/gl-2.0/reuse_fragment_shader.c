/* Copyright © 2015 Intel Corporation
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

/** @file reuse_fragment_shader.c
 *
 * Compile and run two programs that use the same vertex and fragment
 * shader objects.
 *
 * The spec says: "It is also permissible to attach a shader object to more
 *                 than one program object."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const float green[] = {0, 1, 0, 0};
static const float blue[] = {0, 0, 1, 0};

static GLint prog_1;
static GLint prog_2;

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	glUseProgram(prog_2);
	piglit_draw_rect(-1, -1, 1, 2);

	glUseProgram(prog_1);
	piglit_draw_rect(0, -1, 1, 2);

	pass = piglit_probe_rect_rgba(
			0, 0, piglit_width / 2, piglit_height, green);
	pass = piglit_probe_rect_rgba(
			piglit_width / 2, 0,
			piglit_width / 2, piglit_height, blue) && pass;
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static const char *vs_source =
	"void main()\n"
	"{\n"
	"	gl_Position = gl_Vertex;\n"
	"}\n";

static const char *fs_source =
	"uniform vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = color;\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	const GLuint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
						     fs_source);
	const GLuint vs = piglit_compile_shader_text(GL_VERTEX_SHADER,
						     vs_source);
	GLint color_loc_prog_1;
	GLint color_loc_prog_2;

	prog_1 = piglit_link_simple_program(vs, fs);
	glUseProgram(prog_1);
	color_loc_prog_1 = glGetUniformLocation(prog_1, "color");
	glUniform4fv(color_loc_prog_1, 1, blue);

	prog_2 = piglit_link_simple_program(vs, fs);
	glUseProgram(prog_2);
	color_loc_prog_2 = glGetUniformLocation(prog_2, "color");
	glUniform4fv(color_loc_prog_2, 1, green);

	glDeleteShader(vs);
	glDeleteShader(fs);
}
