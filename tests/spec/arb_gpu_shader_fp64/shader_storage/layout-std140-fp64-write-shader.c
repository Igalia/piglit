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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file layout-std140-fp64-write-shader.c
 *
 * Tests that shader storage block writes in GLSL works correctly (offsets and
 * values) when interface packing qualifier is std140 and row_major, using
 * doubles.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.window_width = 100;
	config.window_height = 100;
	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

#define SSBO_SIZE 44

static const char vs_pass_thru_text[] =
	"#version 150\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
        "#extension GL_ARB_gpu_shader_fp64 : require\n"
	"\n"
	"struct A {\n"
	"       double a1;\n"
	"       dvec2 a2[2];\n"
	"       dmat2 a4;\n"
	"};\n"
	"layout(std140, row_major, binding=2) buffer ssbo {\n"
	"       dvec4 v;\n"
	"       double d;\n"
	"       A s;\n"
	"       double unsized_array[];\n"
	"};\n"
	"in vec4 piglit_vertex;\n"
	"void main() {\n"
	"	gl_Position = piglit_vertex;\n"
	"       d = 4.0lf;\n"
	"       s.a2[0] = dvec2(6.0, 7.0); \n"
	"       int index = int(v.x); // index should be zero\n"
	"       unsized_array[index + gl_VertexID] = unsized_array.length();\n"
        "}\n";

static const char fs_source[] =
	"#version 150\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
        "#extension GL_ARB_gpu_shader_fp64 : require\n"
	"\n"
	"struct A {\n"
	"       double a1;\n"
	"       dvec2 a2[2];\n"
	"       dmat2 a4;\n"
	"};\n"
	"layout(std140, row_major, binding=2) buffer ssbo {\n"
	"       dvec4 v;\n"
	"       double d;\n"
	"       A s;\n"
	"       double unsized_array[];\n"
	"};\n"
	"out vec4 color;\n"
	"\n"
	"void main() {\n"
	"       color = vec4(0,1,0,1);\n"
	"       v = dvec4(0.0, 1.0, 2.0, 3.0);\n"
	"       s.a1 = 5.0lf;\n"
	"       s.a2[1] = dvec2(8.0, 9.0);\n"
	"       s.a4 = dmat2(10.0, 11.0, 12.0, 13.0);\n"
	"       int index = int(v.z + gl_FragCoord.x);\n"
	"       unsized_array[index] = unsized_array.length() * 2.0;\n"
	"}\n";

GLuint prog;

double expected[SSBO_SIZE] = { 0.0,  1.0,  2.0,  3.0, // dvec4 v
                               4.0,  0.0,             // double d
                               5.0,  0.0,             // double s.a1
                               6.0,  7.0,             // dvec2 s.a2[0]
                               8.0,  9.0,             // dvec2 s.a2[1]
                               10.0, 12.0,            // dmat2 a4
                               11.0, 13.0,            // dmat2 a4
                               14.0, 0.0,             // double unsized_array[0]
                               14.0, 0.0,             // double unsized_array[1]
                               28.0, 0.0,             // double unsized_array[2]
                               28.0, 0.0,             // double unsized_array[3]
                               28.0, 0.0,             // double unsized_array[4]
                               28.0, 0.0,             // double unsized_array[5]
                               28.0, 0.0,             // double unsized_array[6]
                               28.0, 0.0,             // double unsized_array[7]
                               28.0, 0.0,             // double unsized_array[8]
                               28.0, 0.0,             // double unsized_array[9]
                               28.0, 0.0,             // double unsized_array[10]
                               28.0, 0.0,             // double unsized_array[11]
                               28.0, 0.0,             // double unsized_array[12]
                               28.0, 0.0,             // double unsized_array[13]
};

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint buffer;
	unsigned int i;
	double ssbo_values[SSBO_SIZE] = {0};
	double *map;

	piglit_require_extension("GL_ARB_shader_storage_buffer_object");
	piglit_require_extension("GL_ARB_gpu_shader_fp64");
        piglit_require_GLSL_version(150);

	prog = piglit_build_simple_program(vs_pass_thru_text, fs_source);

	glUseProgram(prog);

	glClearColor(0, 0, 0, 0);

	glGenBuffers(1, &buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, SSBO_SIZE*sizeof(GLdouble),
				&ssbo_values[0], GL_DYNAMIC_DRAW);

	glViewport(0, 0, piglit_width, piglit_height);

	piglit_draw_rect(-1, -1, 2, 2);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
	map = (double *) glMapBuffer(GL_SHADER_STORAGE_BUFFER,  GL_READ_ONLY);

	for (i = 0; i < SSBO_SIZE; i++) {
		if (map[i] != expected[i]) {
			printf("expected[%d] = %.2f. Read value: %.2f\n",
			       i, expected[i], map[i]);
			pass = false;
		}
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	if (!piglit_check_gl_error(GL_NO_ERROR))
	   pass = false;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
