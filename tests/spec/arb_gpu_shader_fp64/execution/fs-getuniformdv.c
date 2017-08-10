/*
 * Copyright 2011 VMware, Inc.
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
 * Asst. gl[Get]Uniformdv tests.
 * based on getunifom02.c from Brian Paul.
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

static char *TestName = "fs-getuniformdv";

static const char vs_text[] =
	"#version 150\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
	"}\n";

static const char fs_text[] =
	"#version 150\n"
	"#extension GL_ARB_gpu_shader_fp64 : require\n"
	"\n"
	"struct s1 {\n"
	"	double a, b, c, d;\n"
	"};\n"
	"\n"
	"uniform double d1;\n"
	"uniform dvec2 u1[2];\n"
	"uniform dvec3 u2[4];\n"
	"uniform dvec4 v[3];\n"
	"uniform dmat2 m1;\n"
	"uniform dmat3 m2;\n"
	"uniform dmat4 m3[3];\n"
	"uniform dmat2x3 m4;\n"
	"uniform dmat2x4 m5;\n"
	"uniform dmat3x2 m6;\n"
	"uniform dmat3x4 m7;\n"
	"uniform dmat4x2 m8[2];\n"
	"uniform dmat4x3 m9;\n"
	"uniform s1 s;\n"
	"uniform double d2;\n"
	"\n"
	"out vec4 fscolor;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	dvec4 t = dvec4(s.a, s.b, s.c, s.d) * d1 + d2 + u1[0]*m8[0] + u1[1]*m8[1];\n"
	"	t += v[0]*m3[0] + v[1]*m3[1] + v[2]*m3[2]  + u2[0]*m9;\n"
	"	t.rb += u1[0]*m1 + u1[1] + u2[0]*m4 + v[0]*m5;\n"
	"	t.xyw += u2[0]*m2 + u2[1] + u2[2] + u2[3] + u1[1]*m6 + v[0]*m7;\n"
	"	fscolor = vec4(t);\n"
	"}\n";

#define MAX_VALUES 16

#define EXPECTED_ACTIVE_UNIFORMS 18

static struct {
	char *name;
	char *altName;
	GLint expectedType;
	GLenum expectedSize;
} uniforms[] = {
	{ "v",  "v[0]", GL_DOUBLE_VEC4,   3},
	{"u1", "u1[0]", GL_DOUBLE_VEC2,   2},
	{"u2", "u2[0]", GL_DOUBLE_VEC3,   4},
	{"m1",    NULL, GL_DOUBLE_MAT2,   1},
	{"m2",    NULL, GL_DOUBLE_MAT3,   1},
	{"m3", "m3[0]", GL_DOUBLE_MAT4,   3},
	{"m4",    NULL, GL_DOUBLE_MAT2x3, 1},
	{"m5",    NULL, GL_DOUBLE_MAT2x4, 1},
	{"m6",    NULL, GL_DOUBLE_MAT3x2, 1},
	{"m7",    NULL, GL_DOUBLE_MAT3x4, 1},
	{"m8", "m8[0]", GL_DOUBLE_MAT4x2, 2},
	{"m9",    NULL, GL_DOUBLE_MAT4x3, 1},
	{NULL,    NULL, GL_DOUBLE,        1}};  //default

enum uniform_enum {
	d1 = 0, d2, sa, sd,
	u1_0, u1_1, u2_0, u2_2,
	v_0, v_1,
	m1, m2, m3, m4, m5, m6, m7, m8_0, m9, _last
};

static struct {
	char *location;
	GLint size;
	GLdouble values[MAX_VALUES];
} uniform_values[] = {
	{    "d1",  1, { 5.0 }},
	{    "d2",  1, {10.0}},
	{   "s.a",  1, {15.0}},
	{   "s.d",  1, {20.0}},
	{ "u1[0]",  2, {12.0, 14.0}},
	{ "u1[1]",  2, {5.0, 8.0}},
	{ "u2[0]",  3, {1.0, 1.0, 2.0}},
	{ "u2[2]",  3, {20.0, 20.0, 15.0}},
	{  "v[0]",  4, {2.0, 3.0, 4.0, 5.0}},
	{  "v[1]",  4, {1.0, 2.0, 3.0, 4.0}},
	{    "m1",  4, {1.0, 2.0,
			3.0, 4.0}},
	{    "m2",  9, {1.0, 1.0, 1.0,
			2.0, 2.0, 2.0,
			3.0, 3.0, 3.0}},
	{ "m3[1]", 16, {1.0, 2.0, 3.0, 4.0,
			5.0, 6.0, 7.0, 8.0,
			1.5, 2.5, 3.5, 4.5,
			5.5, 6.5, 7.5, 8.5}},
	{    "m4",  6, {15.0, 16.0,
			17.0, 18.0,
			19.0, 20.0}},
	{    "m5",  8, {10.0, 11.0,
			12.0, 13.0,
			14.0, 15.0,
			15.0, 17.0}},
	{    "m6",  6, {51.0, 52.0, 53.0,
			54.0, 55.0, 56.0 }},
	{    "m7", 12, {28.0, 29.0, 30.0,
			31.0, 32.0, 33.0,
			34.0, 35.0, 36.0,
			37.0, 38.0, 39.0}},
	{ "m8[0]",  8, {2.7, 3.7, 4.7, 5.7,
			6.7, 8.7, 9.7, 1.7}},
	{    "m9", 12, {11.1, 12.1, 13.1, 14.1,
			15.1, 16.1, 17.1, 18.1,
			19.1, 20.1, 21.1, 22.1}}};

enum piglit_result
piglit_display(void)
{
	/* never called */
	return PIGLIT_FAIL;
}

static bool
verify_uniform(GLuint prog, enum uniform_enum u)
{
	GLint loc;
	GLdouble val[MAX_VALUES];
	bool match = true;
	int i;

	loc = glGetUniformLocation(prog, uniform_values[u].location);
	glGetUniformdv(prog, loc, val);
	for (i = 0; i < uniform_values[u].size; i++) {
		match = match && (val[i] == uniform_values[u].values[i]);
	}

	if (!match) {
		printf("%s: wrong value for %s (found ",
		       TestName, uniform_values[u].location);
		for (i = 0; i < uniform_values[u].size; i++) {
			printf("%g,", val[i]);
		}
		printf(" expected ");
		for (i = 0; i < (uniform_values[u].size - 1); i++) {
			printf("%g,", uniform_values[u].values[i]);
		}
		printf ("%g)\n",
			uniform_values[u].values[uniform_values[u].size - 1]);
	}

	return match;
}

void
piglit_init(int argc, char **argv)
{
	bool piglit_pass = true;
	GLuint vs, fs, prog;
	GLint numUniforms, i;
	GLint loc;
	enum uniform_enum u;

	piglit_require_extension("GL_ARB_gpu_shader_fp64");

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);
	prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);

	glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &numUniforms);
	if (numUniforms != EXPECTED_ACTIVE_UNIFORMS) {
		printf("%s: incorrect number of uniforms"
		       " (found %d, expected %d)\n",
		       TestName, numUniforms, EXPECTED_ACTIVE_UNIFORMS);
		piglit_pass = false;
	}

	/* check types, sizes */
	for (i = 0; i < numUniforms; i++) {
		GLchar name[100];
		GLsizei len;
		GLint size, j;
		GLenum type;
		GLint loc;

		glGetActiveUniform(prog, i, sizeof(name),
				   &len, &size, &type, name);
		loc = glGetUniformLocation(prog, name);

		if (loc < 0) {
			printf("%s: bad uniform location for %s: %d\n",
			       TestName, name, loc);
			piglit_pass = false;
		}

		if (!piglit_automatic) {
			printf("%d: %s loc=%d size=%d type=0x%x\n",
			       i, name, loc, size, type);
		}

		/* OpenGL ES 3.0 and OpenGL 4.2 require that the "[0]"
		 * be appended to the name.  Earlier versions of the
		 * spec are ambiguous.  Accept either name.
		 */
		j = 0;
		while (uniforms[j].name != NULL) {
			if (strcmp(name, uniforms[j].name) == 0) {
				break;
			}
			if (uniforms[j].altName &&
			    strcmp(name, uniforms[j].altName) == 0) {
				break;
			}
			j++;
		}

		if (type != uniforms[j].expectedType) {
			printf("%s: wrong type for '%s'"
			       " (found 0x%x, expected 0x%x)\n",
			       TestName,
			       uniforms[j].name ? uniforms[j].name : name,
			       type, uniforms[j].expectedType);
			piglit_pass = false;
		}

		if (size != uniforms[j].expectedSize) {
			printf("%s: wrong size for '%s'"
			       " (found %d, expected %d)\n",
			       TestName,
			       uniforms[j].name ? uniforms[j].name : name,
			       size, uniforms[j].expectedSize);
			piglit_pass = false;
		}
	}

	/* Check setting/getting values */

	loc = glGetUniformLocation(prog, uniform_values[d1].location);
	glUniform1d(loc, uniform_values[d1].values[0]);

	loc = glGetUniformLocation(prog, uniform_values[d2].location);
	glUniform1d(loc, uniform_values[d2].values[0]);

	loc = glGetUniformLocation(prog, uniform_values[sa].location);
	glUniform1dv(loc, 1, uniform_values[sa].values);

	loc = glGetUniformLocation(prog, uniform_values[sd].location);
	glUniform1d(loc, uniform_values[sd].values[0]);

	loc = glGetUniformLocation(prog, uniform_values[u1_0].location);
	glUniform2dv(loc, 1, uniform_values[u1_0].values);

	loc = glGetUniformLocation(prog, uniform_values[u2_0].location);
	glUniform3dv(loc, 1, uniform_values[u2_0].values);

	loc = glGetUniformLocation(prog, uniform_values[v_1].location);
	glUniform4dv(loc, 1, uniform_values[v_1].values);

	loc = glGetUniformLocation(prog, uniform_values[m1].location);
	glUniformMatrix2dv(loc, 1, false, uniform_values[m1].values);

	loc = glGetUniformLocation(prog, uniform_values[m2].location);
	glUniformMatrix3dv(loc, 1, false, uniform_values[m2].values);

	loc = glGetUniformLocation(prog, uniform_values[m3].location);
	glUniformMatrix4dv(loc, 1, false, uniform_values[m3].values);

	loc = glGetUniformLocation(prog, uniform_values[m4].location);
	glUniformMatrix2x3dv(loc, 1, false, uniform_values[m4].values);

	loc = glGetUniformLocation(prog, uniform_values[m5].location);
	glUniformMatrix2x4dv(loc, 1, false, uniform_values[m5].values);

	loc = glGetUniformLocation(prog, uniform_values[m6].location);
	glUniformMatrix3x2dv(loc, 1, false, uniform_values[m6].values);

	loc = glGetUniformLocation(prog, uniform_values[m7].location);
	glUniformMatrix3x4dv(loc, 1, false, uniform_values[m7].values);

	loc = glGetUniformLocation(prog, uniform_values[m8_0].location);
	glUniformMatrix4x2dv(loc, 1, false, uniform_values[m8_0].values);

	loc = glGetUniformLocation(prog, uniform_values[m9].location);
	glUniformMatrix4x3dv(loc, 1, false, uniform_values[m9].values);

	loc = glGetUniformLocation(prog, uniform_values[u1_1].location);
	glUniform2d(loc,
		    uniform_values[u1_1].values[0],
		    uniform_values[u1_1].values[1]);

	loc = glGetUniformLocation(prog, uniform_values[u2_2].location);
	glUniform3d(loc,
		    uniform_values[u2_2].values[0],
		    uniform_values[u2_2].values[1],
		    uniform_values[u2_2].values[2]);

	loc = glGetUniformLocation(prog, uniform_values[v_0].location);
	glUniform4d(loc,
		    uniform_values[v_0].values[0],
		    uniform_values[v_0].values[1],
		    uniform_values[v_0].values[2],
		    uniform_values[v_0].values[3]);

	for (u = 0; u < _last; u++) {
		piglit_pass = piglit_pass && verify_uniform(prog, u);
	}

	piglit_report_result(piglit_pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
