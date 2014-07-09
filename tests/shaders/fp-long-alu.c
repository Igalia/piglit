/*
 * Copyright (c) 2009 Nicolai Hähnle
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
 *    Nicolai Hähnle <nhaehnle@gmail.com>
 *
 */

/**
 * \file
 * Test certain type of very long fragment programs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

int max_alu_instructions;

static const char program_Head[] =
	"!!ARBfp1.0\n"
	"TEMP r;\n"
	"MOV r, 0;\n"
	;

static const char program_Step[] = "ADD %s, r.wxyz, { %f, %f, %f, %f };\n";

static const char program_Tail[] =
	"END\n";

static const char program_Output[] = "result.color";

static void step_add(unsigned int i, float * add)
{
	unsigned int rotate = i % 4;
	add[(0 + rotate) % 4] = ((i+1) % 16) ? 0.0625 : -1.0+0.0625;
	add[(1 + rotate) % 4] = ((i+1) % 16) ? 0.0 : (((i+1) % 256) ? 0.0625 : -1.0+0.0625);
	add[(2 + rotate) % 4] = ((i+1) % 256) ? 0.0 : (((i+1) % 4096) ? 0.0625 : -1.0+0.0625);
	add[(3 + rotate) % 4] = ((i+1) % 4096) ? 0.0 : 0.0625;
}

static enum piglit_result test(unsigned int alu_depth)
{
	char * program_text = malloc(sizeof(program_Head) +
	                             2*alu_depth*sizeof(program_Step) +
	                             sizeof(program_Tail) +
	                             sizeof(program_Output));
	char * buildp;
	char buf[128];
	GLuint program_object;
	unsigned int i;
	float expected[4] = { 0.0, 0.0, 0.0, 0.0 };

	/* Note: This test makes sense up to alu_depth of 65536,
	 * but current drivers are not exactly efficient with such
	 * long programs, and if 16k works, then 64k will probably
	 * work, too ;-)
	 */
	if (!alu_depth || alu_depth > 16384 || alu_depth + 1 > max_alu_instructions) {
		free(program_text);
		return PIGLIT_SKIP;
	}

	printf("Testing: alu_depth = %u\n", alu_depth);

	memcpy(program_text, program_Head, sizeof(program_Head)-1);
	buildp = program_text + sizeof(program_Head)-1;

	for(i = 0; i < alu_depth; ++i) {
		float add[4];
		int length;
		step_add(i, add);
		length = snprintf(buf, sizeof(buf),
			program_Step, i == (alu_depth-1) ? program_Output : "r",
			add[0], add[1], add[2], add[3]);
		memcpy(buildp, buf, length);
		buildp += length;
	}
	memcpy(buildp, program_Tail, sizeof(program_Tail));

//	printf(program_text);

	program_object = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, program_text);

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, program_object);

	piglit_draw_rect(0, 0, 32, 32);

	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	glDeleteProgramsARB(1, &program_object);

	expected[0] = (alu_depth % 16) * 0.0625;
	expected[1] = ((alu_depth/16) % 16) * 0.0625;
	expected[2] = ((alu_depth/256) % 16) * 0.0625;
	expected[3] = (alu_depth/4096) * 0.0625;
	for(i = 0; i < ((alu_depth+3) % 4); ++i) {
		float tmp = expected[3];
		expected[3] = expected[2];
		expected[2] = expected[1];
		expected[1] = expected[0];
		expected[0] = tmp;
	}
	if (expected[0] > 1.0)
		expected[0] = 1.0;
	if (expected[1] > 1.0)
		expected[1] = 1.0;
	if (expected[2] > 1.0)
		expected[2] = 1.0;
	if (expected[3] > 1.0)
		expected[3] = 1.0;

	if (!piglit_probe_pixel_rgba(16, 16, expected)) {
		fprintf(stderr, "Failure in alu_depth = %i\n", alu_depth);
		return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

enum piglit_result piglit_display(void)
{
	enum piglit_result result;
	unsigned int alu_depth;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	glClear(GL_COLOR_BUFFER_BIT);

	alu_depth = 1;
	for(;;) {
		result = test(alu_depth);
		if (result == PIGLIT_SKIP)
			break;
		if (result != PIGLIT_PASS)
			return result;

		if (alu_depth < 8) {
			alu_depth++;
		} else {
			/* Not quite powers of two to avoid aliasing */
			alu_depth = (alu_depth * 2) - 5;
		}
	}

	return PIGLIT_PASS;
}


void piglit_init(int argc, char ** argv)
{
	piglit_require_fragment_program();

	glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB,
			  GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB,
			  &max_alu_instructions);

	printf("Max (native) ALU instructions: %i\n",
	       max_alu_instructions);
}
