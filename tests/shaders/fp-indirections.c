/*
 * Copyright © 2008 Intel Corporation
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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static int get_program_i(GLenum pname)
{
	GLint val;

	glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, pname, &val);

	return val;
}

/**
 * Generate a program that samples the texture into the same temporary over
 * and over..
 *
 * This should excersize case (2) of question (24) of the ARB_fragment_program
 * spec.
 *
 * Note that the compiler could optimize out our inner TEX instructions
 * since they've got the same coordinates.  Assume the compiler isn't
 * for now.
 */
static char *gen_temporary_dest_indirections(int sample_count,
					     GLuint *progname)
{
	char *prog;
	char *pre =
		"!!ARBfp1.0\n"
		"TEMP val, sample;\n"
		"MOV val, fragment.color;\n";
	char *sample =
		"TEX sample, fragment.color, texture[0], 2D;\n"
		"MUL val, val, sample;\n";
	char *post =
		"MOV result.color, val;\n"
		"END";
	int i, instr_count;

	prog = malloc(strlen(pre) + strlen(sample) * sample_count +
		      strlen(post) + 1);

	if (prog == 0) {
		printf("malloc failed.\n");
		piglit_report_result(PIGLIT_FAIL);
		exit(1);
	}

	sprintf(prog, "%s", pre);
	for (i = 0; i < sample_count; i++)
		strcat(prog, sample);
	strcat(prog, post);

	instr_count = 2 + 2 * (sample_count - 1) + 1;
	if (get_program_i(GL_MAX_PROGRAM_INSTRUCTIONS_ARB) < instr_count) {
		printf("indirection count %d too low to generate program with "
		       "%d indirections and %d instructions\n",
		       get_program_i(GL_MAX_PROGRAM_INSTRUCTIONS_ARB),
		       sample_count, instr_count);
		free(prog);
		return NULL;
	}

	*progname = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, prog);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, *progname);

	return prog;
}

/**
 * Generate a program that samples two textures into a pair of temporaries
 * over and over.
 *
 * This should excersize case (1) of question (24) of the ARB_fragment_program
 * spec without hitting case (2) at the same time.
 *
 * Note that the compiler could optimize out our inner TEX instructions
 * since they've got the same coordinates.  Assume the compiler isn't
 * for now.
 */
static char *gen_temporary_source_indirections(int sample_count,
					       GLuint *progname)
{
	char *prog;
	char *pre =
		"!!ARBfp1.0\n"
		"TEMP val, val2, sample, sample2;\n"
		"MOV val, fragment.color;\n"
		"MOV val2, fragment.color;\n";
	char *sample =
		"TEX sample, val, texture[0], 2D;\n"
		"TEX sample2, val2, texture[1], 2D;\n"
		"MUL val, sample, sample2;\n"
		"MUL val2, val2, val;\n";
	char *post =
		"MOV result.color, val;\n"
		"END";
	int i, instr_count;

	instr_count = 2 + 4 * (sample_count - 1) + 1;
	if (get_program_i(GL_MAX_PROGRAM_INSTRUCTIONS_ARB) < instr_count) {
		printf("indirection count %d too low to generate program with "
		       "%d indirections and %d instructions\n",
		       get_program_i(GL_MAX_PROGRAM_INSTRUCTIONS_ARB),
		       sample_count, instr_count);
		return NULL;
	}

	prog = malloc(strlen(pre) + strlen(sample) * (sample_count - 1) +
		      strlen(post) + 1);

	if (prog == 0) {
		printf("malloc failed.\n");
		piglit_report_result(PIGLIT_FAIL);
		exit(1);
	}

	sprintf(prog, "%s", pre);
	for (i = 0; i < sample_count - 1; i++)
		strcat(prog, sample);
	strcat(prog, post);

	*progname = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, prog);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, *progname);

	return prog;
}

void
print_program_info(char *program)
{
	printf("Program:\n");
	printf("%s", program);
	printf("\n");

	printf("tex instructions: %d\n",
	       get_program_i(GL_PROGRAM_TEX_INSTRUCTIONS_ARB));
	printf("native tex instructions: %d\n",
	       get_program_i(GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB));
	printf("tex indirections: %d\n",
	       get_program_i(GL_PROGRAM_TEX_INDIRECTIONS_ARB));
	printf("native tex indirections: %d\n",
	       get_program_i(GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB));
	printf("\n");
}

/**
 * Test that we can emit a whole load of samples as long as the indirection
 * count is low.
 */
GLboolean test_temporary_dest_indirections(void)
{
	GLboolean pass = GL_TRUE;
	GLuint progname;
	char *prog;
	GLint indirections_limit, use_limit;
	GLint count;

	indirections_limit = get_program_i(GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB);

	use_limit = MIN2(indirections_limit, 1024);

	count = use_limit - 1;
	printf("testing program with %d indirections from temporary dests\n",
	       count);
	prog = gen_temporary_dest_indirections(count, &progname);
	if (prog != NULL) {
		if (!get_program_i(GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB)) {
			printf("Program with %d indirections unexpectedly "
			       "exceeded native limits.\n", count);
			print_program_info(prog);
			pass = GL_FALSE;
		}
		free(prog);
	}

	count = use_limit + 1;
	printf("testing program with %d indirections from temporary dests\n",
	       count);
	prog = gen_temporary_dest_indirections(count, &progname);
	if (prog != NULL && count > indirections_limit) {
		if (get_program_i(GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB)) {
			printf("Program with %d indirections unexpectedly "
			       "met native limits.\n", count);
			print_program_info(prog);
		}
		free(prog);
	}

	return pass;
}

/**
 * Test that we can emit a whole load of samples as long as the indirection
 * count is low.
 */
GLboolean test_temporary_source_indirections(void)
{
	GLboolean pass = GL_TRUE;
	GLuint progname;
	char *prog;
	GLint indirections_limit, use_limit;
	GLint count;

	indirections_limit = get_program_i(GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB);

	use_limit = MIN2(indirections_limit, 1024);

	count = use_limit - 1;
	printf("testing program with %d indirections from temporary sources\n",
	       count);
	prog = gen_temporary_source_indirections(count, &progname);
	if (prog != NULL) {
		if (!get_program_i(GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB)) {
			printf("Program with %d indirections unexpectedly "
			       "exceeded native limits.\n", count);
			print_program_info(prog);
			pass = GL_FALSE;
		}
		free(prog);
	}

	count = use_limit + 1;
	printf("testing program with %d indirections from temporary sources\n",
	       count);
	prog = gen_temporary_source_indirections(count, &progname);
	if (prog != NULL && count > indirections_limit) {
		if (get_program_i(GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB)) {
			printf("Program with %d indirections unexpectedly "
			       "met native limits.\n", count);
			print_program_info(prog);
		}
		free(prog);
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;

	pass = test_temporary_dest_indirections() && pass;
	pass = test_temporary_source_indirections() && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_fragment_program();

	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	printf("Maximum tex instructions: %d\n",
	       get_program_i(GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB));
	printf("Maximum native tex instructions: %d\n",
	       get_program_i(GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB));
	printf("Maximum tex indirections: %d\n",
	       get_program_i(GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB));
	printf("Maximum native tex indirections: %d\n",
	       get_program_i(GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB));

	/* If the GL reports more than 10000 texture indirections, then we're probably
	 * running on hardware with no limitations - the driver just picked some
	 * arbitrary large number to report back.  The test isn't meaningful, and
	 * the run time explodes with huge limits, so just skip it.
	 *
	 * For reference, Mesa and NVIDIA report 16384; AMD reports 2147483647.
	 * Pineview hardware (where this test is relevant) has a limit of 4.
	 */
	if (get_program_i(GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB) > 10000) {
		printf("Skipping; the hardware doesn't appear to have real limits.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}
