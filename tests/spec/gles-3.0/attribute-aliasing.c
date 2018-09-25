/*
 * Copyright © 2018 Intel Corporation
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
 * @file
 * Test that vertex attribute aliasing is disallowed.
 *
 * From OpenGL ES 3.0.5 spec "2.12.5 Vertex Attributes":
 *
 *    "Binding more than one attribute name to the same location is referred
 *     to as aliasing, and is not permitted in OpenGL ES Shading Language 3.00
 *     vertex shaders. LinkProgram will fail when this condition exists."
 *
 * From OpenGL ES SL 3.10/3.20 spec:
 *
 *    "The existence of aliasing is determined by declarations present
 *    after preprocessing."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_es_version = 30;
PIGLIT_GL_TEST_CONFIG_END

static const char vs_source[] =
	"#version 300 es\n"
	"in highp float a;\n"
	"in highp float b;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(0.0);\n"
	"}\n";

static const char fs_source[] =
	"#version 300 es\n"
	"out highp vec4 color;\n"
	"void main()\n"
	"{\n"
	"	color = vec4(0.0);\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint vs, fs, prog;

	prog = glCreateProgram();

	/* Bind 2 attributes to the same location. */
	glBindAttribLocation(prog, 0, "a");
	glBindAttribLocation(prog, 0, "b");

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_source);

	glAttachShader(prog, vs);
	glAttachShader(prog, fs);

	glLinkProgram(prog);

	if (piglit_link_check_status_quiet(prog))
		piglit_report_result(PIGLIT_FAIL);

	glDeleteShader(vs);
	glDeleteShader(fs);
	glDeleteProgram(prog);

	piglit_report_result(PIGLIT_PASS);
}
