/*
 * Copyright © 2012 Intel Corporation
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

/** @file getactiveuniformsiv-uniform-matrix-stride.c
 *
 * Tests that (std140 layout) uniform matrix strides are reported
 * correctly through the API.
 *
 * Because std140 lays matrices out like arrays, and array elements
 * get rounded up to the size of a vec4, MATRIX_STRIDE is either 16 or
 * a non-matrix value.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const char fs_source[] =
	"#extension GL_ARB_uniform_buffer_object : require\n"
	"\n"
	"layout(std140) uniform ub {\n"
	"	vec4 v4;\n"
	"	mat4 m4;\n"
	"	mat3 m3;\n"
	"	mat2 m2;\n"
	"	mat4 m4a[2];\n"
	"};\n"
	"uniform vec4 default_v4;\n"
	"uniform mat4 default_m4;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = v4 + default_v4 + default_m4[0] + m4[0] + vec4(m3[0], 0.) + vec4(m2[0], 0., 0.) + m4a[0][0];\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint prog;
	const char *uniform_names[] = {
		"v4",
		"m4",
		"m3",
		"m2",
		"m4a[0]",
		"default_v4",
		"default_m4",
	};
	int expected_strides[] = {
		0,
		16,
		16,
		16,
		16,
		-1,
		-1,
	};
	GLint strides[ARRAY_SIZE(uniform_names)];
	GLuint uniform_indices[ARRAY_SIZE(uniform_names)];
	int i;

	piglit_require_extension("GL_ARB_uniform_buffer_object");

	prog = piglit_build_simple_program(NULL, fs_source);

	glGetUniformIndices(prog, ARRAY_SIZE(uniform_names), uniform_names,
			    uniform_indices);
	glGetActiveUniformsiv(prog, ARRAY_SIZE(uniform_names), uniform_indices,
			      GL_UNIFORM_MATRIX_STRIDE, strides);
	for (i = 0; i < ARRAY_SIZE(uniform_names); i++) {
		printf("Uniform \"%s\": stride %d, expected %d",
		       uniform_names[i],
		       strides[i],
		       expected_strides[i]);

		if (strides[i] != expected_strides[i]) {
			printf(" FAIL");
			pass = false;
		}

		printf("\n");
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
