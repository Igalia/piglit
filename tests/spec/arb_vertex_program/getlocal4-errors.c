/*
 * Copyright © 2013 Intel Corporation
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

/** @file getlocal4-errors.c
 *
 * Tests that the specced errors are generated for
 * glGetProgramLocalParameter*ARB().
 *
 * From the GL_ARB_vertex_program spec:
 *
 *     "The error INVALID_ENUM is generated if <target> specifies a
 *      nonexistent program target or a program target that does not
 *      support program local parameters.  The error INVALID_VALUE is
 *      generated if <index> is greater than or equal to the
 *      implementation-dependent number of supported program local
 *      parameters for the program target."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	const char *source =
		"!!ARBvp1.0\n"
		"OPTION ARB_position_invariant;\n"
		"MOV result.color, program.local[3];\n"
		"END\n";
	GLuint prog;
	GLint max_local;
	float fvalues[4];
	double dvalues[4];

	piglit_require_extension("GL_ARB_vertex_program");

	prog = piglit_compile_program(GL_VERTEX_PROGRAM_ARB, source);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, prog);

	glGetProgramiv(GL_VERTEX_PROGRAM_ARB,
		       GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, &max_local);

	/* Check that an error is generated for going beyond max. */
	glGetProgramLocalParameterfvARB(GL_VERTEX_PROGRAM_ARB, max_local,
					fvalues);
	piglit_check_gl_error(GL_INVALID_VALUE);

	glGetProgramLocalParameterdvARB(GL_VERTEX_PROGRAM_ARB, max_local,
					dvalues);
	piglit_check_gl_error(GL_INVALID_VALUE);

	/* Test bad program target */
	glGetProgramLocalParameterfvARB(0xd0d0d0d0, max_local, fvalues);
	piglit_check_gl_error(GL_INVALID_ENUM);

	glGetProgramLocalParameterdvARB(0xd0d0d0d0, max_local, dvalues);
	piglit_check_gl_error(GL_INVALID_ENUM);

	piglit_report_result(PIGLIT_PASS);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
