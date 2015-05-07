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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file program-interface-query.c
 *
 * Test that checks the proper implementation of GL_ARB_program_interface_query
 * implementation for shader storage buffers
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.window_width = 100;
	config.window_height = 100;
	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

#define SSBO_SIZE 4
#define NUM_QUERIES 11

static const char vs_pass_thru_text[] =
	"#version 330\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"\n"
	"struct B {mat2 b[3]; float c;};\n"
	"layout(row_major, std140, binding=2) buffer ssbo_std140 {\n"
	"       vec4 v;\n"
	"       B s[];\n"
	"} a_std140[2];\n"
	"\n"
	"in vec4 piglit_vertex;\n"
	"\n"
	"void main() {\n"
	"	gl_Position = piglit_vertex;\n"
	"       a_std140[0].s[0].b[0] = mat2(1.0, 2.0, 3.0, 4.0);\n"
        "}\n";

static const char fs_source[] =
	"#version 330\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"\n"
	"out vec4 color;\n"
	"\n"
	"struct B {mat2 b[3]; float c;};\n"
	"\n"
	"layout(std430, column_major, binding=2) buffer ssbo_std430 {\n"
	"       vec4 v;\n"
	"       B s[2];\n"
	"} a_std430[2];\n"
	"\n"
	"void main() {\n"
	"       a_std430[0].s[0].b[0] = mat2(1.0, 2.0, 3.0, 4.0);\n"
	"       color = a_std430[0].v;\n"
	"}\n";

GLuint prog;

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLint index;
	const GLenum prop[NUM_QUERIES] = {GL_TOP_LEVEL_ARRAY_SIZE,
					  GL_TOP_LEVEL_ARRAY_STRIDE,
					  GL_TYPE,
					  GL_ARRAY_SIZE,
					  GL_BLOCK_INDEX,
					  GL_OFFSET,
					  GL_ARRAY_STRIDE,
					  GL_MATRIX_STRIDE,
					  GL_IS_ROW_MAJOR,
					  GL_REFERENCED_BY_VERTEX_SHADER,
					  GL_REFERENCED_BY_FRAGMENT_SHADER };
	const char *prop_names[NUM_QUERIES] = {"GL_TOP_LEVEL_ARRAY_SIZE",
					       "GL_TOP_LEVEL_ARRAY_STRIDE",
					       "GL_TYPE",
					       "GL_ARRAY_SIZE",
					       "GL_BLOCK_INDEX",
					       "GL_OFFSET",
					       "GL_ARRAY_STRIDE",
					       "GL_MATRIX_STRIDE",
					       "GL_IS_ROW_MAJOR",
					       "GL_REFERENCED_BY_VERTEX_SHADER",
					       "GL_REFERENCED_BY_FRAGMENT_SHADER" };
	int query_std140[NUM_QUERIES] = {0};
	int query_std430[NUM_QUERIES] = {0};
	const int expected_std140[NUM_QUERIES] =
		{ 0, 112, GL_FLOAT_MAT2, 3, 0, 16, 32, 16, 1, 1, 0 };
	const int expected_std430[NUM_QUERIES] =
		{ 2, 64, GL_FLOAT_MAT2, 3, 2, 16, 16, 8, 0, 0, 1 };
	int i;

	piglit_require_extension("GL_ARB_shader_storage_buffer_object");
	piglit_require_extension("GL_ARB_program_interface_query");

	prog = piglit_build_simple_program(vs_pass_thru_text, fs_source);

	glUseProgram(prog);

	/* First ssbo_std140 */
	index = glGetProgramResourceIndex(prog,
					  GL_BUFFER_VARIABLE,
					  "ssbo_std140.s[0].b[0]");
	glGetProgramResourceiv(prog, GL_BUFFER_VARIABLE, index,
                               NUM_QUERIES, prop, NUM_QUERIES, NULL,
			       query_std140);
	if (!piglit_check_gl_error(GL_NO_ERROR))
	   pass = false;

	/* Now ssbo_std430 */
	index = glGetProgramResourceIndex(prog,
					  GL_BUFFER_VARIABLE,
                                          "ssbo_std430.s[0].b[0]");
        glGetProgramResourceiv(prog, GL_BUFFER_VARIABLE, index,
                               NUM_QUERIES, prop, NUM_QUERIES, NULL,
			       query_std430);

	if (!piglit_check_gl_error(GL_NO_ERROR))
	   pass = false;

	for (i = 0 ; i < NUM_QUERIES; i++) {
		if (query_std140[i] != expected_std140[i]) {
			printf("std140 %s expected = %d. Value = %d.\n",
			       prop_names[i], expected_std140[i],
			       query_std140[i]);
			pass = false;
		}
		if (query_std430[i] != expected_std430[i]) {
			printf("std430 %s expected = %d. Value = %d.\n",
			       prop_names[i], expected_std430[i],
			       query_std430[i]);
			pass = false;
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
