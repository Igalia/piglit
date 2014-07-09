/*
 * Copyright (c) 2014 Intel Corporation
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

/** @file invocation-id.c
 *
 * Verifies reading GL_GEOMETRY_SHADER_INVOCATIONS
 */

 #include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version   = 32;

PIGLIT_GL_TEST_CONFIG_END

const char *vs_source = {
	"#version 150\n"
	"void main() {}\n"
};

const char *gs_source1 = {
	"#version 150\n"
	"#extension GL_ARB_gpu_shader5 : enable\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 3) out;\n"
	"\n"
	"void main() {}\n"
};

const char *gs_source4 = {
	"#version 150\n"
	"#extension GL_ARB_gpu_shader5 : enable\n"
	"layout(triangles, invocations = 4) in;\n"
	"layout(triangle_strip, max_vertices = 3) out;\n"
	"\n"
	"void main() {}\n"
};

const char *fs_source = {
	"#version 150\n"
	"void main() {\n"
	"	gl_FragColor = vec4(0, 1, 0, 1);\n"
	"}\n"
};

static bool
test_gs_invocations(const char *gs_src, GLint expected)
{
	GLint program;
	GLint invocations;
	bool pass = true;

	program = piglit_build_simple_program_multiple_shaders(
						GL_VERTEX_SHADER,   vs_source,
						GL_GEOMETRY_SHADER, gs_src,
						GL_FRAGMENT_SHADER, fs_source,
						0);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	if (!pass)
		return pass;

	glGetProgramiv(program, GL_GEOMETRY_SHADER_INVOCATIONS, &invocations);
	pass = piglit_check_gl_error(GL_NO_ERROR);

	if (pass) {
		pass = pass && (invocations == expected);
		if (!pass) {
			printf("GEOMETRY_SHADER_INVOCATIONS: "
			       "Expected=%d, Got=%d\n",
			       expected,
			       invocations);
		}
	}

	glDeleteProgram(program);

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_ARB_gpu_shader5");

	pass = test_gs_invocations(gs_source1, 1) && pass;
	pass = test_gs_invocations(gs_source4, 4) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}

