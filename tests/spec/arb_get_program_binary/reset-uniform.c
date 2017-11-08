/*
 * Copyright (c) 2017 Intel Corporation
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
 * \file reset-uniform.c
 *
 * From the ARB_get_program_binary extension spec:
 *
 * "A successful call to ProgramBinary will reset all uniform variables
 *  to their initial values. The initial value is either the value of
 *  the variable's initializer as specified in the original shader
 *  source, or 0 if no initializer was present."
 *
 * Verify that a uniform value as read through the OpenGL API is
 * restored to its initial value when glProgramBinary is used.
 */

#include "piglit-util-gl.h"
#include "gpb-common.h"
#include <stdlib.h>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static GLuint prog;

static void
check_color(const float *exp, const float *clr)
{
	if (exp[0] != clr[0] || exp[1] != clr[1] || exp[2] != clr[2] ||
	    exp[3] != clr[3]) {
		fprintf(stderr, "uniform color didn't match expected color\n");
		piglit_report_result(PIGLIT_FAIL);
	}
}

void
piglit_init(int argc, char **argv)
{
	int num_formats = 0;
	float red[] = { 1.0, 0.0, 0.0, 1.0 };
	float green[] = { 0.0, 1.0, 0.0, 1.0 };
	float clr[4];
	GLint loc;

	static const char vs_source[] =
		"void main()\n"
		"{\n"
		"    gl_Position = gl_Vertex;\n"
		"}\n";
	static const char fs_source[] =
		"#version 120\n"
		"uniform vec4 color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"\n"
		"void main()\n"
		"{\n"
		"        gl_FragColor = color;\n"
		"}\n";

	piglit_require_extension("GL_ARB_get_program_binary");

	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &num_formats);
	if (num_formats == 0)
		piglit_report_result(PIGLIT_SKIP);

	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);

	loc = glGetUniformLocation(prog, "color");
	if (loc < 0)
		piglit_report_result(PIGLIT_FAIL);

	glGetUniformfv(prog, loc, clr);
	check_color(green, clr);

	glUniform4fv(loc, 1, red);
	glGetUniformfv(prog, loc, clr);
	check_color(red, clr);

	gpb_save_restore(&prog);

	loc = glGetUniformLocation(prog, "color");
	if (loc < 0)
		piglit_report_result(PIGLIT_FAIL);

	glGetUniformfv(prog, loc, clr);
	check_color(green, clr);

	piglit_report_result(PIGLIT_PASS);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
