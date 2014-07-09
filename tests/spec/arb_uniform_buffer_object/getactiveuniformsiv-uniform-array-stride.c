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

/** @file getactiveuniformsiv-uniform-array-stride.c
 *
 * Tests that (std140 layout) uniform array strides are reported
 * correctly through the API.
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
	"	vec4 a;\n"
	"	vec4 b[2];\n"
	"	float c[2];\n"
	"	mat4 d[2];\n"
	"};\n"
	"uniform vec4 e;\n"
	"uniform vec4 f[2];\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = a + b[0] + vec4(c[0]) + d[0][0] + e + f[0];\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint prog;
	const char *uniform_names[] = { "a", "b", "c", "d", "e", "f" };
	int expected_strides[] = { 0, 16, 16, 64, -1, -1 };
	GLint strides[ARRAY_SIZE(uniform_names)];
	GLuint uniform_indices[ARRAY_SIZE(uniform_names)];
	int i;

	piglit_require_extension("GL_ARB_uniform_buffer_object");

	prog = piglit_build_simple_program(NULL, fs_source);

	glGetUniformIndices(prog, ARRAY_SIZE(uniform_names), uniform_names,
			    uniform_indices);
	glGetActiveUniformsiv(prog, ARRAY_SIZE(uniform_names), uniform_indices,
			      GL_UNIFORM_ARRAY_STRIDE, strides);
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
