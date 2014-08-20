/*
 * Copyright (c) 2014 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file stream_value_too_large.c
 *
 * Test that exceeding the implementation's maximum streams
 * value (GL_MAX_VERTEX_STREAMS) results in a compile
 * error.
 *
 * From the ARB_gpu_shader5 specification:
 *
 * "If an implementation supports <N> vertex streams, the
 *  individual streams are numbered 0 through <N>-1"
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

GLint vs;
GLint fs;

static const char *vs_source =
	"#version 150\n"
	"\n"
	"void main()\n"
	"{\n"
	"gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
	"}\n";

static const char *gs_template =
	"#version 150\n"
	"#extension GL_ARB_gpu_shader5: enable\n"
	"\n"
	"layout(points) in;\n"
	"layout(points, stream = %d, max_vertices=1) out;\n"
	"\n"
	"void main()\n"
	"{\n"
	"gl_Position = vec4(1.0, 1.0, 1.0, 1.0);\n"
	"EmitStreamVertex(0);\n"
	"EndStreamPrimitive(0);\n"
	"}\n";

static const char *fs_source =
	"#version 150\n"
	"out vec3 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"color = vec3(0.0, 0.0, 0.0);\n"
	"}\n";

static GLint
compile_shaders(GLint size, bool expect_ok, GLint *ok)
{

	char *shader_text;
	GLint shader;

	printf("Stream count of %d should %s: ", size,
	       expect_ok ? "compile successfully" :
	       "produce a compile error");

	asprintf(&shader_text, gs_template, size);
	shader = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(shader, 1, (const GLchar **) &shader_text, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, ok);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		/* Details of the error have already been printed. */
		printf("GL Error occurred.\n");
	}

	if (*ok)
		printf("Successful compile.\n");
	else
		printf("Compile error.\n");
	return shader;
}

static bool
test_streams_size(GLint size, bool expect_compile_ok, bool expect_link_ok)
{
	GLint program;
	GLint ok;
	GLint gs;

	gs = compile_shaders(size, expect_compile_ok, &ok);
	if (!ok)
		return ok == expect_compile_ok;

	/* Check if test was compiled but a compilation error was expected */
	if (ok != expect_compile_ok)
		return false;

	printf("Stream count of %d should %s: ", size,
	       expect_link_ok ? "link successfully" : "produce a link error");

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, gs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &ok);

	if (!piglit_link_check_status(program)) {
		/* Details of the error have already been printed. */
		printf("Link error occurred.\n");
	}

	if (ok)
		printf("Successful link.\n");
	else
		printf("Link error.\n");

	glDetachShader(program, fs);
	glDetachShader(program, gs);
	glDetachShader(program, vs);
	glDeleteProgram(program);
	glDeleteShader(gs);
	return ok == expect_link_ok;
}


void
piglit_init(int argc, char **argv)
{
	GLint max_streams;
	int i;
	bool pass = true;

	piglit_require_extension("GL_ARB_gpu_shader5");

	glGetIntegerv(GL_MAX_VERTEX_STREAMS, &max_streams);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, (const GLchar **) &vs_source, NULL);
	glCompileShader(vs);
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, (const GLchar **) &fs_source, NULL);
	glCompileShader(fs);

	for (i = 0; i < max_streams; i++) {
		/* Test that this value is allowed. */
		pass = test_streams_size(i, true, true) && pass;
	}

	/* Test max multiple vertex stream value */
	pass = test_streams_size(i, false, false) && pass;

	glDeleteShader(vs);
	glDeleteShader(fs);
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
