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
 * \file get-tes-params.c
 *
 * Test tessellation evaluation shader layout getters.
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


static const struct {
	GLenum prim_mode;
	GLenum vertex_spacing;
	GLenum ordering;
	GLenum point_mode;
	const char *source;
} tes_params[] = {
	{GL_QUADS, GL_EQUAL, GL_CCW, GL_FALSE,
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(quads) in;\n"
"void main() { gl_Position = vec4(0.0); }\n"},

	{GL_TRIANGLES, GL_EQUAL, GL_CCW, GL_FALSE,
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(triangles) in;\n"
"void main() { gl_Position = vec4(0.0); }\n"},

	{GL_ISOLINES, GL_EQUAL, GL_CCW, GL_FALSE,
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(isolines) in;\n"
"void main() { gl_Position = vec4(0.0); }\n"},

	{GL_QUADS, GL_FRACTIONAL_ODD, GL_CCW, GL_FALSE,
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(quads, fractional_odd_spacing) in;\n"
"void main() { gl_Position = vec4(0.0); }\n"},

	{GL_QUADS, GL_FRACTIONAL_EVEN, GL_CCW, GL_FALSE,
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(quads, fractional_even_spacing) in;\n"
"void main() { gl_Position = vec4(0.0); }\n"},

	{GL_QUADS, GL_EQUAL, GL_CW, GL_FALSE,
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(quads, cw) in;\n"
"void main() { gl_Position = vec4(0.0); }\n"},

	{GL_QUADS, GL_EQUAL, GL_CCW, GL_TRUE,
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(quads, point_mode) in;\n"
"void main() { gl_Position = vec4(0.0); }\n"},
};


static bool
test_param(GLenum pname, GLenum expected_value, const char *const source)
{
	int v;

	glGetProgramiv(prog, pname, &v);
	if (v == expected_value)
		return true;

	fprintf(stderr, "%s is %s, expected %s for program \n%s\n",
		piglit_get_gl_enum_name(pname),
		piglit_get_gl_enum_name(v),
		piglit_get_gl_enum_name(expected_value), source);
	return false;
}


static bool
test_tes_params(void)
{
	bool pass = true;
	int i;

	for (i = 0; i < ARRAY_SIZE(tes_params); ++i) {
		prog = piglit_build_simple_program_multiple_shaders(
				GL_VERTEX_SHADER, vs_source,
				GL_TESS_EVALUATION_SHADER, tes_params[i].source,
				0);

		pass = test_param(GL_TESS_GEN_MODE,
				  tes_params[i].prim_mode,
				  tes_params[i].source) && pass;
		pass = test_param(GL_TESS_GEN_SPACING,
				  tes_params[i].vertex_spacing,
				  tes_params[i].source) && pass;
		pass = test_param(GL_TESS_GEN_VERTEX_ORDER,
				  tes_params[i].ordering,
				  tes_params[i].source) && pass;
		pass = test_param(GL_TESS_GEN_POINT_MODE,
				  tes_params[i].point_mode,
				  tes_params[i].source) && pass;

		glDeleteProgram(prog);
	}
	return pass;
}


void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_ARB_tessellation_shader");

	pass = test_tes_params() && pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

