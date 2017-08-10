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

/** @file layout-std140-fp64-shader.c
 *
 * Tests that shader storage block reads/writes in GLSL works correctly (offsets
 * and values) when interface packing qualifier is std140 and row_major, using
 * doubles.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.window_width = 100;
	config.window_height = 100;
	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

#define SSBO_SIZE 44
#define TOLERANCE 1e-5
#define DIFFER(a,b) ((a > b ? a - b : b - a) > TOLERANCE)

static const char vs_code[] =
	"#version 150\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"#extension GL_ARB_gpu_shader_fp64 : require\n"
	"\n"
	"struct A {\n"
	"	double a1;\n"
	"	dvec2 a2[2];\n"
	"	dmat2 a4;\n"
	"	double a5;\n"
	"};\n"
	"\n"
	"layout(std140, row_major, binding=2) buffer ssbo {\n"
	"	dvec4 u;\n"
	"	dvec4 v;\n"
	"	double d;\n"
	"	A s;\n"
	"	double unsized_array[];\n"
	"};\n"
	"\n"
	"in vec4 piglit_vertex;\n"
	"out vec4 vertex_to_gs;\n"
	"\n"
	"void main() {\n"
	"	vertex_to_gs = piglit_vertex;\n"
	"	d = 4.333333333333333259lf;\n"
	"	s.a2[0] = dvec2(6.0, 7.0) * s.a5;\n"
	"	int index = int(v.x); // index should be zero\n"
	"	unsized_array[index + gl_VertexID] = unsized_array.length();\n"
	"}\n";

static const char gs_source[] =
	"#version 150\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"#extension GL_ARB_gpu_shader_fp64 : require\n"
	"\n"
	"struct A {\n"
	"	double a1;\n"
	"	dvec2 a2[2];\n"
	"	dmat2 a4;\n"
	"	double a5;\n"
	"};\n"
	"\n"
	"layout(std140, row_major, binding=2) buffer ssbo {\n"
	"	dvec4 u;\n"
	"	dvec4 v;\n"
	"	double d;\n"
	"	A s;\n"
	"	double unsized_array[];\n"
	"};\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 3) out;\n"
	"\n"
	"in vec4 vertex_to_gs[3];\n"
	"\n"
	"void main() {\n"
	"	for (int i = 0; i < 3; i++) {\n"
	"		gl_Position = vertex_to_gs[i] + vec4(s.a1);\n"
	"		EmitVertex();\n"
	"	}\n"
	"	s.a4 = dmat2(-1.333333333333333259lf, 11.0, 12.0, 13.0);\n"
	"}\n";

static const char fs_source[] =
	"#version 150\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"#extension GL_ARB_gpu_shader_fp64 : require\n"
	"\n"
	"struct A {\n"
	"	double a1;\n"
	"	dvec2 a2[2];\n"
	"	dmat2 a4;\n"
	"	double a5;\n"
	"};\n"
	"\n"
	"layout(std140, row_major, binding=2) buffer ssbo {\n"
	"	dvec4 u;\n"
	"	dvec4 v;\n"
	"	double d;\n"
	"	A s;\n"
	"	double unsized_array[];\n"
	"};\n"
	"\n"
	"out vec4 color;\n"
	"\n"
	"void main() {\n"
	"	color = vec4(0,1,0,1);\n"
	"	v = u + dvec4(0.333333333333333259lf, 1.0, 2.0, 3.0);\n"
	"	s.a2[1] = dvec2(8.0, 9.0);\n"
	"	int index = int(v.z + gl_FragCoord.x);\n"
	"	unsized_array[index] = unsized_array.length() * 2.0;\n"
	"}\n";

GLuint prog;

double ssbo_values[SSBO_SIZE] = { 6.0,  7.0, 8.0, 0.0,  // dvec4 u
				  0.0,  0.0, 0.0, 0.0,  // dvec4 v
				  0.0,  0.0,		// double d
				  1.0,  0.0,		// double s.a1
				  0.0,  0.0,		// dvec2 s.a2[0]
				  0.0,  0.0,		// dvec2 s.a2[1]
				  0.0,  0.0,		// dmat2 s.a4
				  0.0,  0.0,		// dmat2 s.a4
				  2.0,  0.0,		// double s.a5
				  0.0,  0.0,		// double unsized_array[0]
				  0.0,  0.0,		// double unsized_array[1]
				  0.0,  0.0,		// double unsized_array[2]
				  0.0,  0.0,		// double unsized_array[3]
				  0.0,  0.0,		// double unsized_array[4]
				  0.0,  0.0,		// double unsized_array[5]
				  0.0,  0.0,		// double unsized_array[6]
				  0.0,  0.0,		// double unsized_array[7]
				  0.0,  0.0,		// double unsized_array[8]
				  0.0,  0.0,		// double unsized_array[9]
				  0.0,  0.0,		// double unsized_array[10]
};

double expected[SSBO_SIZE] = {  6.0,  7.0, 8.0, 0.0,		      // dvec4 u		   expected[0]
				6.333333333333333259, 8.0, 10.0, 3.0, // dvec4 v		   expected[4]
				4.333333333333333259, 0.0,	      // double d		   expected[8]
				1.0,  0.0,			      // double s.a1		   expected[10]
			       12.0, 14.0,			      // dvec2 s.a2[0]		   expected[12]
				8.0,  9.0,			      // dvec2 s.a2[1]		   expected[14]
			       -1.333333333333333259, 12.0,	      // dmat2 s.a4		   expected[16]
			       11.0, 13.0,			      // dmat2 s.a4		   expected[18]
				2.0,  0.0,			      // double s.a5		   expected[20]
			       11.0,  0.0,			      // double unsized_array[0]   expected[22]
			       11.0,  0.0,			      // double unsized_array[1]   expected[24]
			       11.0,  0.0,			      // double unsized_array[2]   expected[26]
			       11.0,  0.0,			      // double unsized_array[3]   expected[28]
				0.0,  0.0,			      // double unsized_array[4]   expected[30]
				0.0,  0.0,			      // double unsized_array[5]   expected[32]
				0.0,  0.0,			      // double unsized_array[6]   expected[34]
				0.0,  0.0,			      // double unsized_array[7]   expected[36]
				0.0,  0.0,			      // double unsized_array[8]   expected[38]
				0.0,  0.0,			      // double unsized_array[9]   expected[40]
				0.0,  0.0,			      // double unsized_array[10]  expected[42]
};

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint buffer;
	unsigned int i;
	double *map;

	piglit_require_extension("GL_ARB_shader_storage_buffer_object");
	piglit_require_extension("GL_ARB_gpu_shader_fp64");
	piglit_require_GLSL_version(150);

	prog = piglit_build_simple_program_multiple_shaders(
		GL_VERTEX_SHADER, vs_code,
		GL_GEOMETRY_SHADER, gs_source,
		GL_FRAGMENT_SHADER, fs_source,
		NULL);

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
		if (DIFFER(map[i], expected[i])) {
			printf("expected[%d] = %.14g. Read value: %.14g\n",
			       i, expected[i], map[i]);
			pass = false;
		}
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	glDeleteProgram(prog);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
