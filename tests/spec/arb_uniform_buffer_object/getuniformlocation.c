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

/** @file getuniformlocation.c
 *
 * Tests that glGetUniformLocation() returns no location for UBO variables.
 *
 * From the GL_ARB_uniform_buffer_object spec:
 *
 *      To find the location within a program object of an active uniform
 *      variable associated with the default uniform block, use the command
 *
 *          int GetUniformLocation(uint program, const char *name);
 *
 *      ... The value -1 will be returned if <name> does not
 *      correspond to an active uniform variable name in <program>, if
 *      <name> is associated with a named uniform block..."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static GLuint prog;

static const char fs_source[] =
	"#extension GL_ARB_uniform_buffer_object : require\n"
	"\n"
	"uniform ub_a { vec4 a; };\n"
	"uniform vec4 b;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = a + b;\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	int location;

	piglit_require_extension("GL_ARB_uniform_buffer_object");

	prog = piglit_build_simple_program(NULL, fs_source);

	location = glGetUniformLocation(prog, "a");
	if (location != -1) {
		printf("Uniform \"a\" had location %d, expected -1\n", location);
		pass = false;
	}

	location = glGetUniformLocation(prog, "b");
	if (location == -1) {
		printf("Uniform \"b\" had location %d\n", location);
		pass = false;
	}

	if (!pass)
		printf("Source:\n%s", fs_source);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
