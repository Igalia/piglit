/*
 * Copyright © 2016 Intel Corporation
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
 * \file uniform-invalid-operation.c
 *
 * From the GL_ARB_gpu_shader_fp64 spec:
 *     "regarding INVALID_OPERATION errors in Uniform* comamnds, if the type of
 *     the uniform declared in the shader does not match the component type and
 *     count indicated in the Uniform* command name (where a boolean uniform
 *     component type is considered to match any of the Uniform*i{v},
 *     Uniform*ui{v}, or Uniform*f{v} commands)"
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

static const char vs_text[] =
	"#version 150\n"
	"#extension GL_ARB_gpu_shader_fp64 : require\n"
	"\n"
	"uniform double d;\n"
	"uniform dvec3 v;\n"
	"uniform bool b;\n"
	"\n"
	"out vec4 vscolor;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	if (b)\n"
	"		gl_Position = vec4(v, d);\n"
	"	else\n"
	"		gl_Position = vec4(v, 0.0);\n"
	"	vscolor = vec4(v, d);\n"
	"}\n";

static const char fs_text[] =
	"#version 150\n"
	"\n"
	"in vec4 vscolor;\n"
	"out vec4 fscolor;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	fscolor = vscolor;\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	/* never called */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	bool piglit_pass = true;
	GLuint vs, fs, prog;
	GLint loc;
	float vf[] = { 1.0, 2.0, 3.0 };
	double vd[] = { 1.0, 2.0, 3.0, 4.0, 5.0};

	piglit_require_extension("GL_ARB_gpu_shader_fp64");

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);
	prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);

	// Setting different type should fail
	loc = glGetUniformLocation(prog, "d");
	glUniform1i(loc, 3);
	piglit_pass = piglit_pass
		&& piglit_check_gl_error(GL_INVALID_OPERATION);
	glUniform1f(loc, 3.0);
	piglit_pass = piglit_pass
		&& piglit_check_gl_error(GL_INVALID_OPERATION);
	glUniform1d(loc, 3.0);
	piglit_pass = piglit_pass && piglit_check_gl_error(GL_NO_ERROR);

	loc = glGetUniformLocation(prog, "v");
	glUniform3fv(loc, 1, vf);
	piglit_pass = piglit_pass
		&& piglit_check_gl_error(GL_INVALID_OPERATION);
	glUniform3d(loc, vd[0], vd[1], vd[2]);
	piglit_pass = piglit_pass && piglit_check_gl_error(GL_NO_ERROR);

	// Setting different size should fail
	loc = glGetUniformLocation(prog, "v");
	glUniform2d(loc, vd[0], vd[1]);
	piglit_pass = piglit_pass
		&& piglit_check_gl_error(GL_INVALID_OPERATION);
	glUniform4d(loc, vd[0], vd[1], vd[2], vd[3]);
	piglit_pass = piglit_pass
		&& piglit_check_gl_error(GL_INVALID_OPERATION);
	glUniform3d(loc, vd[0], vd[1], vd[2]);
	piglit_pass = piglit_pass && piglit_check_gl_error(GL_NO_ERROR);

	// Special case for booleans
	loc = glGetUniformLocation(prog, "b");
	glUniform1d(loc, 1.0);
	piglit_pass = piglit_pass
		&& piglit_check_gl_error(GL_INVALID_OPERATION);
	glUniform1f(loc, 1.0);
	piglit_pass = piglit_pass
		&& piglit_check_gl_error(GL_NO_ERROR);

	piglit_report_result(piglit_pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
