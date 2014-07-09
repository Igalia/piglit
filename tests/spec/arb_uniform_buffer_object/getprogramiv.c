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

/** @file getprogramiv.c
 *
 * From the GL_ARB_uniform_buffer_object spec:
 *
 *     "If <pname> is ACTIVE_UNIFORM_BLOCKS the number of uniform
 *      blocks for <program> containing active uniforms is returned.
 *
 *      If <pname> is ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, the length
 *      of the longest active uniform block name, including the null
 *      terminator, is returned."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

struct {
	const char *source;
	int blocks;
	int namelen;
} tests[] = {
	{
		"#extension GL_ARB_uniform_buffer_object : enable\n"
		"uniform a { float u1; };\n"
		"void main() {\n"
		"	gl_FragColor = vec4(u1);\n"
		"}\n",
		1, 2,
	},
	{
		"#extension GL_ARB_uniform_buffer_object : enable\n"
		"uniform a { float u1; };\n"
		"uniform b { float u2; };\n"
		"void main() {\n"
		"	gl_FragColor = vec4(u1 + u2);\n"
		"}\n",
		2, 2,
	},
	{
		"#extension GL_ARB_uniform_buffer_object : enable\n"
		"uniform a { float u1; };\n"
		"uniform bb { float u2; };\n"
		"void main() {\n"
		"	gl_FragColor = vec4(u1 + u2);\n"
		"}\n",
		2, 3,
	},
	{
		"#extension GL_ARB_uniform_buffer_object : enable\n"
		"uniform aa { float u1; };\n"
		"uniform b { float u2; };\n"
		"void main() {\n"
		"	gl_FragColor = vec4(u1 + u2);\n"
		"}\n",
		2, 3,
	},
};

static bool
test_shader(int test)
{
	GLuint prog;
	const char *source = tests[test].source;
	int namelen = 9999, blocks = 9999;

	prog = piglit_build_simple_program(NULL, source);

	glGetProgramiv(prog, GL_ACTIVE_UNIFORM_BLOCKS, &blocks);
	if (blocks != tests[test].blocks) {
		fprintf(stderr,
			"%d: Bad GL_ACTIVE_UNIFORM_BLOCKS "
			"%d, expected %d.  Source:\n%s",
			test, blocks, tests[test].blocks, source);
		return false;
	}

	glGetProgramiv(prog, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &namelen);
	if (namelen != tests[test].namelen) {
		fprintf(stderr,
			"%d: Bad GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH "
			"%d, expected %d.  Source:\n%s",
			test, namelen, tests[test].namelen, source);
		return false;
	}

	glDeleteProgram(prog);

	return true;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	int i;

	piglit_require_extension("GL_ARB_uniform_buffer_object");

	for (i = 0; i < ARRAY_SIZE(tests); i++) {
		pass = test_shader(i) && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

