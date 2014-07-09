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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** \file
 *
 * Test that exceeding the implementation's maximum invocations
 * value (GL_MAX_GEOMETRY_SHADER_INVOCATIONS) results in a compile
 * error.
 *
 * From the ARB_gpu_shader5 specification:
 *
 *     If a shader specifies an invocation count greater than
 *     the implementation-dependent maximum, it will fail to
 *     compile.
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END


enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}


static const char *gs_template =
	"#version 150\n"
	"#extension GL_ARB_gpu_shader5: enable\n"
	"\n"
	"layout(points, invocations = %d) in;\n"
	"\n"
	"void main()\n"
	"{\n"
	"}\n";


static bool
test_invocations_size(GLint size, bool expect_ok)
{
	char *shader_text;
	GLint shader;
	GLint ok;

	printf("Invocation count of %d should %s: ", size,
	       expect_ok ? "compile successfully" : "produce a compile error");

	asprintf(&shader_text, gs_template, size);
	shader = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(shader, 1, (const GLchar **) &shader_text, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		/* Details of the error have already been printed. */
		printf("GL Error occurred.\n");
		return false;
	}
	if (ok)
		printf("Successful compile.\n");
	else
		printf("Compile error.\n");
	return ok == expect_ok;
}


void
piglit_init(int argc, char **argv)
{
	GLint max_invocations;
	int i;
	bool pass = true;

	piglit_require_extension("GL_ARB_gpu_shader5");

	glGetIntegerv(GL_MAX_GEOMETRY_SHADER_INVOCATIONS,
		      &max_invocations);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	for (i = 1; i <= max_invocations; i++) {
		/* Test that this size is allowed. */
		pass = test_invocations_size(i, true) && pass;
	}

	/* Test that a count 1 above the max causes a compilation failure. */
	pass = test_invocations_size(max_invocations + 1, false) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
