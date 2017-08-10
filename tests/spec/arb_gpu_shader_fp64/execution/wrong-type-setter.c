/*
 * Copyright © 2015 Intel Corporation
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
 * \name wronge-type-setter.c
 * Try setting a double uniform with a float setter, expect an error.
 *
 * Also try the vice-versa combinations.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_source[] =
	"#version 150\n"
	"#extension GL_ARB_gpu_shader_fp64: require\n"
	"\n"
	"uniform float   f1;\n"
	"uniform vec2    f2;\n"
	"uniform vec3    f3;\n"
	"uniform vec4    f4;\n"
	"\n"
	"uniform mat2    fm22;\n"
	"uniform mat2x3  fm23;\n"
	"uniform mat2x4  fm24;\n"
	"uniform mat3x2  fm32;\n"
	"uniform mat3    fm33;\n"
	"uniform mat3x4  fm34;\n"
	"uniform mat4x2  fm42;\n"
	"uniform mat4x3  fm43;\n"
	"uniform mat4    fm44;\n"
	"\n"
	"flat out vec4 outf;\n"
	"\n"
	"uniform double  d1;\n"
	"uniform dvec2   d2;\n"
	"uniform dvec3   d3;\n"
	"uniform dvec4   d4;\n"
	"\n"
	"uniform dmat2   dm22;\n"
	"uniform dmat2x3 dm23;\n"
	"uniform dmat2x4 dm24;\n"
	"uniform dmat3x2 dm32;\n"
	"uniform dmat3   dm33;\n"
	"uniform dmat3x4 dm34;\n"
	"uniform dmat4x2 dm42;\n"
	"uniform dmat4x3 dm43;\n"
	"uniform dmat4   dm44;\n"
	"\n"
	"flat out dvec4 outd;\n"
	"\n"
	"void main()\n"
	"{\n"
	"   outf = vec4(f1) +\n"
	"          vec4(fm22 * f2, 0, 0) +\n"
	"          vec4(fm32 * f3, 0, 0) +\n"
	"          vec4(fm42 * f4, 0, 0) +\n"
	"          vec4(fm23 * f2, 0) +\n"
	"          vec4(fm33 * f3, 0) +\n"
	"          vec4(fm43 * f4, 0) +\n"
	"          vec4(fm24 * f2) +\n"
	"          vec4(fm34 * f3) +\n"
	"          vec4(fm44 * f4);\n"
	"   outd = dvec4(d1) +\n"
	"          dvec4(dm22 * d2, 0, 0) +\n"
	"          dvec4(dm32 * d3, 0, 0) +\n"
	"          dvec4(dm42 * d4, 0, 0) +\n"
	"          dvec4(dm23 * d2, 0) +\n"
	"          dvec4(dm33 * d3, 0) +\n"
	"          dvec4(dm43 * d4, 0) +\n"
	"          dvec4(dm24 * d2) +\n"
	"          dvec4(dm34 * d3) +\n"
	"          dvec4(dm44 * d4);\n"
	"   gl_Position = vec4(0);\n"
	"}\n"
	;

static const char fs_source[] =
	"#version 150\n"
	"#extension GL_ARB_gpu_shader_fp64: require\n"
	"\n"
	"flat in vec4 outf;\n"
	"flat in dvec4 outd;\n"
	"\n"
	"out vec4 frag_color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    frag_color = outf + vec4(outd);\n"
	"}\n"
	;

#define TRY_UNIFORM(name, setter, data)					\
	do {								\
		printf("Trying \"%s\" with %s...\n", name, # setter);	\
		loc = glGetUniformLocation(prog, name);			\
		if (loc == -1) {					\
			printf("Could not get location for \"%s\".\n",	\
			       name);					\
			pass = false;					\
		} else {						\
			setter(loc, 1, data);				\
			pass = piglit_check_gl_error(GL_INVALID_OPERATION) \
				&& pass;				\
		}							\
	} while (false)

#define TRY_MATRIX(name, setter, data)					\
	do {								\
		printf("Trying \"%s\" with %s...\n", name, # setter);	\
		loc = glGetUniformLocation(prog, name);			\
		if (loc == -1) {					\
			printf("Could not get location for \"%s\".\n",	\
			       name);					\
			pass = false;					\
		} else {						\
			setter(loc, 1, GL_FALSE, data);			\
			pass = piglit_check_gl_error(GL_INVALID_OPERATION) \
				&& pass;				\
		}							\
	} while (false)

void piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint prog;
	const float float_junk[16] = { 0, };
	const double double_junk[16] = { 0, };
	GLuint loc;

	piglit_require_extension("GL_ARB_gpu_shader_fp64");

	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);

	TRY_UNIFORM("f1", glUniform1dv, double_junk);
	TRY_UNIFORM("f2", glUniform2dv, double_junk);
	TRY_UNIFORM("f3", glUniform3dv, double_junk);
	TRY_UNIFORM("f4", glUniform4dv, double_junk);

	TRY_MATRIX("fm22", glUniformMatrix2dv, double_junk);
	TRY_MATRIX("fm23", glUniformMatrix2x3dv, double_junk);
	TRY_MATRIX("fm24", glUniformMatrix2x4dv, double_junk);
	TRY_MATRIX("fm32", glUniformMatrix3x2dv, double_junk);
	TRY_MATRIX("fm33", glUniformMatrix3dv, double_junk);
	TRY_MATRIX("fm34", glUniformMatrix3x4dv, double_junk);
	TRY_MATRIX("fm42", glUniformMatrix4x2dv, double_junk);
	TRY_MATRIX("fm43", glUniformMatrix4x3dv, double_junk);
	TRY_MATRIX("fm44", glUniformMatrix4dv, double_junk);

	TRY_UNIFORM("d1", glUniform1fv, float_junk);
	TRY_UNIFORM("d2", glUniform2fv, float_junk);
	TRY_UNIFORM("d3", glUniform3fv, float_junk);
	TRY_UNIFORM("d4", glUniform4fv, float_junk);

	TRY_MATRIX("dm22", glUniformMatrix2fv, float_junk);
	TRY_MATRIX("dm23", glUniformMatrix2x3fv, float_junk);
	TRY_MATRIX("dm24", glUniformMatrix2x4fv, float_junk);
	TRY_MATRIX("dm32", glUniformMatrix3x2fv, float_junk);
	TRY_MATRIX("dm33", glUniformMatrix3fv, float_junk);
	TRY_MATRIX("dm34", glUniformMatrix3x4fv, float_junk);
	TRY_MATRIX("dm42", glUniformMatrix4x2fv, float_junk);
	TRY_MATRIX("dm43", glUniformMatrix4x3fv, float_junk);
	TRY_MATRIX("dm44", glUniformMatrix4fv, float_junk);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
