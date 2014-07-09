/*
 * Copyright © 2014 Intel Corporation
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
 * \file get-tcs-params.c
 *
 * Test tessellation control shader layout getters.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END


unsigned int prog;


static const char *const vs_source =
"#version 150\n"
"void main() { gl_Position = vec4(0.0); }\n";


static const char *const tcs_source_template =
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(vertices = %d) out;\n"
"void main() {\n"
"	gl_out[gl_InvocationID].gl_Position = vec4(0.0);\n"
"	gl_TessLevelOuter = float[4](1.0, 1.0, 1.0, 1.0);\n"
"	gl_TessLevelInner = float[2](1.0, 1.0);\n"
"}\n";
static char *tcs_source;


static bool
test_tcs_params(const int vertices)
{
	int v;

	sprintf(tcs_source, tcs_source_template, vertices);
	prog = piglit_build_simple_program_multiple_shaders(
			GL_VERTEX_SHADER, vs_source,
			GL_TESS_CONTROL_SHADER, tcs_source,
			0);

	glGetProgramiv(prog, GL_TESS_CONTROL_OUTPUT_VERTICES, &v);
	if (v != vertices) {
		fprintf(stderr, "GL_TESS_CONTROL_OUTPUT_VERTICES "
			"is %d, expected %d for program \n%s\n",
			v, vertices, tcs_source);
		return false;
	}

	return true;
}


void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	int i, max_vertices;
	static const int vertices[] = {1, 2, 3, 4, 8, 16, 32};

	piglit_require_extension("GL_ARB_tessellation_shader");

	tcs_source = malloc(strlen(tcs_source_template) + 16);

	for (i = 0; i < ARRAY_SIZE(vertices); ++i) {
		pass = test_tcs_params(vertices[i]) && pass;
	}

	glGetIntegerv(GL_MAX_PATCH_VERTICES, &max_vertices);
	pass = test_tcs_params(max_vertices) && pass;

	free(tcs_source);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

