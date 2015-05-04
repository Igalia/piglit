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

/** @file layout-std430-write-shader.c
 *
 * Tests that shader storage block writes in GLSL works correctly (offsets and
 * values) when interface packing qualifier is std430.
 * 
 * From GL_ARB_shader_storage_buffer_object:
 * 
 *  "When using the "std430" storage layout, shader storage
 *   blocks will be laid out in buffer storage identically to uniform and
 *   shader storage blocks using the "std140" layout, except that the base
 *   alignment of arrays of scalars and vectors in rule (4) and of structures
 *   in rule (9) are not rounded up a multiple of the base alignment of a vec4."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.window_width = 100;
	config.window_height = 100;
	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

#define SSBO_SIZE 48

static const char vs_pass_thru_text[] =
	"#version 130\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
        "#extension GL_ARB_uniform_buffer_object : require\n"
	"\n"
	"struct B { float b1[3]; };\n"
	"struct A {\n"
	"       float a1;\n"
	"       vec3 a2;\n"
	"       mat2 a4[2];\n"
	"       B sb[2];\n"
	"};\n"
	"layout(std430, binding=2) buffer ssbo {\n"
	"       vec4 v;\n"
	"       float f;\n"
	"       A s;\n"
	"       mat3x4 m;\n"
	"       float unsized_array[];\n"
	"};\n"
	"in vec4 piglit_vertex;\n"
	"void main() {\n"
	"	gl_Position = piglit_vertex;\n"
	"       v.yz = vec2(1.0, 2.0);\n"
	"       f = 4.0;\n"
	"       s.a2.x = 6.0; \n"
	"       s.a2.y = 7.0; \n"
	"       s.a4[0] = mat2(10.0, 11.0, 12.0, 13.0);\n"
	"       s.sb[0].b1[0] = 18.0;\n"
	"       s.sb[0].b1[1] = 19.0;\n"
	"       m[1] = vec4(25.0, 26.0, 27.0, 28.0);\n"
	"       int index = int(v.x); // index should be zero\n"
	"       unsized_array[index + gl_VertexID] = unsized_array.length();\n"
	"}\n";

static const char fs_source[] =
	"#version 130\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
        "#extension GL_ARB_uniform_buffer_object : require\n"
	"\n"
        "struct B { float b1[3]; };\n"
	"struct A {\n"
	"       float a1;\n"
	"       vec3 a2;\n"
	"       mat2 a4[2];\n"
        "       B sb[2];\n"
	"};\n"
	"layout(std430, binding=2) buffer ssbo {\n"
	"       vec4 v;\n"
	"       float f;\n"
	"       A s;\n"
	"       mat3x4 m;\n"
	"       float unsized_array[];\n"
	"};\n"
	"out vec4 color;\n"
	"\n"
	"void main() {\n"
	"       color = vec4(0,1,0,1);\n"
	"       v.xw = vec2(0.0, 3.0);\n"
	"       s.a1 = 5.0;\n"
	"       s.a2.z = 8.0;\n"
	"       s.a4[1] = mat2(14.0, 15.0, 16.0, 17.0);\n"
	"       s.sb[1].b1[2] = 20.0;\n"
	"       m[0] = vec4(21.0, 22.0, 23.0, 24.0);\n"
	"       m[2] = vec4(29.0, 30.0, 31.0, 32.0);\n"
	"       int index = int(v.z + gl_FragCoord.x);\n"
	"       unsized_array[index] = unsized_array.length() * 2.0;\n"
	"}\n";

GLuint prog;

float expected[SSBO_SIZE] = { 0.0,  1.0,  2.0,  3.0, // vec4 v
			      4.0,  0.0,  0.0,  0.0, // float f
			      5.0,  0.0,  0.0,  0.0, // float s.a1
			      6.0,  7.0,  8.0,  0.0, // vec3 s.a2
			     10.0, 11.0, 12.0, 13.0, // mat2 s.a4[0]
			     14.0, 15.0, 16.0, 17.0, // mat2 s.a4[1]
			     18.0, 19.0,  0.0,  0.0, // float s.sb[0].b1
			      0.0, 20.0,  0.0,  0.0, // float s.sb[1].b1
			     21.0, 22.0, 23.0, 24.0, // mat3x4 m[0]
			     25.0, 26.0, 27.0, 28.0, // mat3x4 m[1]
			     29.0, 30.0, 31.0, 32.0, // mat3x4 m[2]
			      4.0,  4.0,  8.0,  8.0  // float unsized_array[0]
};

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint buffer;
	unsigned int i;
	float ssbo_values[SSBO_SIZE] = {0};
	float *map;

	piglit_require_extension("GL_ARB_shader_storage_buffer_object");
        piglit_require_GLSL_version(130);

	prog = piglit_build_simple_program(vs_pass_thru_text, fs_source);

	glUseProgram(prog);

	glClearColor(0, 0, 0, 0);

	glGenBuffers(1, &buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, SSBO_SIZE*sizeof(GLfloat),
				&ssbo_values[0], GL_DYNAMIC_DRAW);

	glViewport(0, 0, piglit_width, piglit_height);

	piglit_draw_rect(-1, -1, 2, 2);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
	map = (float *) glMapBuffer(GL_SHADER_STORAGE_BUFFER,  GL_READ_ONLY);

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
