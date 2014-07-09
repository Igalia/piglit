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

/** @file getlocal4f-max.c
 *
 * Tests that we can read back all local parameters up to
 * GL_MAX_PROGRAM_LOCAL_PARAMETERS, even if the program reads less
 * than that.
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
	GLint i;

	piglit_require_extension("GL_ARB_vertex_program");

	prog = piglit_compile_program(GL_VERTEX_PROGRAM_ARB, source);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, prog);

	glGetProgramivARB(GL_VERTEX_PROGRAM_ARB,
			  GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, &max_local);

	/* Limit the test to blowing through 256MB of memory. */
	max_local = MIN2(max_local, 1024 * 1024 * 16);

	for (i = 0; i < max_local; i++) {
		float values[4];

		values[0] = i * 4;
		values[1] = i * 4 + 1;
		values[2] = i * 4 + 2;
		values[3] = i * 4 + 3;

		glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, i, values);
	}

	for (i = 0; i < max_local; i++) {
		float values[4], get_values[4];

		glGetProgramLocalParameterfvARB(GL_VERTEX_PROGRAM_ARB, i,
						get_values);

		values[0] = i * 4;
		values[1] = i * 4 + 1;
		values[2] = i * 4 + 2;
		values[3] = i * 4 + 3;

		if (memcmp(values, get_values, sizeof(values)) != 0) {
			fprintf(stderr, "Difference on "
				"glGetProgramLocalParameterfvARB(%d):\n", i);
			fprintf(stderr, "expected: %f %f %f %f\n",
				values[0],
				values[1],
				values[2],
				values[3]);
			fprintf(stderr, "found:    %f %f %f %f\n",
				get_values[0],
				get_values[1],
				get_values[2],
				get_values[3]);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	piglit_report_result(PIGLIT_PASS);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
