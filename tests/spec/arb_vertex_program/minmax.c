/* Copyright © 2011 Intel Corporation
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

/** @file minmax.c
 *
 * Test for the minimum maximum values in GL_ARB_vertex_program.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static bool pass = true;
static GLenum target;

static void
min_test_i(GLenum token, GLint min, const char *name)
{
	GLint val;

	glGetIntegerv(token, &val);

	if (val < min) {
		fprintf(stderr, "%-50s %8d %8d (ERROR)\n", name, min, val);
		pass = false;
	} else {
		printf("%-50s %8d %8d\n", name, min, val);
	}
}

static void
min_test_program(GLenum token, GLint min, const char *name)
{
	GLint val;

	glGetProgramivARB(target, token, &val);

	if (val < min) {
		fprintf(stderr, "%-50s %8d %8d (ERROR)\n", name, min, val);
		pass = false;
	} else {
		printf("%-50s %8d %8d\n", name, min, val);
	}
}
#define MIN_INTEGER_TEST(token, min) min_test_i(token, min, #token)
#define MIN_PROGRAM_TEST(token, min) min_test_program(token, min, #token)

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_vertex_program");

	printf("%-50s %8s %8s\n", "token", "minimum", "value");

	target = GL_VERTEX_PROGRAM_ARB;
	MIN_PROGRAM_TEST(GL_MAX_PROGRAM_ENV_PARAMETERS_ARB, 96);
	MIN_PROGRAM_TEST(GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, 96);
	MIN_INTEGER_TEST(GL_MAX_PROGRAM_MATRICES_ARB, 8);
	MIN_INTEGER_TEST(GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB, 1);
	MIN_PROGRAM_TEST(GL_MAX_PROGRAM_INSTRUCTIONS_ARB, 128);
	MIN_PROGRAM_TEST(GL_MAX_PROGRAM_TEMPORARIES_ARB, 12);
	MIN_PROGRAM_TEST(GL_MAX_PROGRAM_PARAMETERS_ARB, 96);
	MIN_PROGRAM_TEST(GL_MAX_PROGRAM_ATTRIBS_ARB, 16);
	MIN_PROGRAM_TEST(GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB, 1);
	/* No specified minimum, but test that we can query them anyway. */
	MIN_PROGRAM_TEST(GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB, 0);
	MIN_PROGRAM_TEST(GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB, 0);
	MIN_PROGRAM_TEST(GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB, 0);
	MIN_PROGRAM_TEST(GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB, 0);
	MIN_PROGRAM_TEST(GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB, 0);

	/* See the GL_ARB_fragment_program specification for this
	 * consistency requirement.
	 */
	if (piglit_is_extension_supported("GL_ARB_fragment_program")) {
		target = GL_FRAGMENT_PROGRAM_ARB;
		MIN_PROGRAM_TEST(GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB, 0);
		MIN_PROGRAM_TEST(GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB, 0);
		MIN_PROGRAM_TEST(GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB, 0);
		MIN_PROGRAM_TEST(GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB, 0);
		MIN_PROGRAM_TEST(GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB, 0);
		MIN_PROGRAM_TEST(GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB, 0);
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
