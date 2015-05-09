/*
 * Copyright © 2009 Intel Corporation
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
 * \file fp-set-02.c
 * Validate all of the set-on instructions in GL_NV_fragment_program_option
 *
 * Each set-on instruction is validated by comparing the value 0.5 with all
 * combinations 0.0, 0.5, and 1.0 on the four channels.  Reference squares are
 * even rows, and testing squares are on odd rows.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl.h"

typedef int (cmp_func)(float a, float b);

static int eq_func(float a, float b);
static int fl_func(float a, float b);
static int ge_func(float a, float b);
static int gt_func(float a, float b);
static int le_func(float a, float b);
static int lt_func(float a, float b);
static int ne_func(float a, float b);
static int tr_func(float a, float b);

struct {
	char *opcode;
	cmp_func *func;
} tests[] = {
	{ "STR", tr_func },
	{ "SFL", fl_func },
	{ "SEQ", eq_func },
	{ "SNE", ne_func },
	{ "SGE", ge_func },
	{ "SLT", lt_func },
	{ "SGT", gt_func },
	{ "SLE", le_func },
};

/* One column for each possible combination of set-on results
 */
#define TEST_COLS  (3 * 3 * 3 * 3)

/* One for each set-on opcode and its reference square.
 */
#define TEST_ROWS  (ARRAY_SIZE(tests) * 2)

#define BOX_SIZE   8

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = (((BOX_SIZE+1)*TEST_COLS)+1);
	config.window_height = (((BOX_SIZE+1)*TEST_ROWS)+1);
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Source for the fragment program to render the reference box.
 */
static const char reference_shader_source[] =
	"!!ARBfp1.0\n"
	"MOV	result.color, program.env[0];\n"
	"END"
	;

static const char shader_template[] =
	"!!ARBfp1.0\n"
	"OPTION	NV_fragment_program;\n"
	"%s	result.color, program.env[1], fragment.color;\n"
	"END"
	;


/**
 * \name Handles to fragment programs.
 */
/*@{*/
static GLint reference_prog;
static GLint progs[ARRAY_SIZE(tests)];
/*@}*/

int
eq_func(float a, float b)
{
	return a == b;
}

int
fl_func(float a, float b)
{
	return 0;
}

int
ge_func(float a, float b)
{
	return a >= b;
}

int
gt_func(float a, float b)
{
	return a > b;
}

int
le_func(float a, float b)
{
	return a <= b;
}


int
lt_func(float a, float b)
{
	return a < b;
}

int
ne_func(float a, float b)
{
	return a != b;
}

int
tr_func(float a, float b)
{
	return 1;
}


enum piglit_result
piglit_display(void)
{
	const GLfloat comparitor[4] = { 0.5, 0.5, 0.5, 0.5 };
	static const float values[3] = { 0.0, 0.5, 1.0 };
	unsigned i;
	unsigned j;
	enum piglit_result result = PIGLIT_PASS;
	GLfloat color[4];
	GLfloat ref[4];

	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, comparitor);

	for (i = 0; i < (2 * ARRAY_SIZE(progs)); i++) {
		const int y = (i * (BOX_SIZE + 1)) + 1;
		const unsigned idx = i >> 1;

		if ((i & 1) == 0) {
			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,
					 reference_prog);
		} else {
			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,
					 progs[idx]);
		}

		for (j = 0; j < (3 * 3 * 3 * 3); j++) {
			const int x = (j * (BOX_SIZE + 1)) + 1;

			/* Determine the color of the reference square.
			 * This depends on both the set-on function and
			 * the mask.
			 */
			ref[0] = values[j        % 3];
			ref[1] = values[(j /  3) % 3];
			ref[2] = values[(j /  9) % 3];
			ref[3] = values[(j / 27) % 3];

			color[0] = tests[idx].func(comparitor[0], ref[0])
				? 1.0 : 0.0;
			color[1] = tests[idx].func(comparitor[1], ref[1])
				? 1.0 : 0.0;
			color[2] = tests[idx].func(comparitor[2], ref[2])
				? 1.0 : 0.0;
			color[3] = tests[idx].func(comparitor[3], ref[3])
				? 1.0 : 0.0;

			glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB,
						    0, color);
			glColor4fv(ref);

			piglit_draw_rect(x, y, BOX_SIZE, BOX_SIZE);
			if ((i & 1) == 0)
				continue;

			if (!piglit_probe_pixel_rgb(x + (BOX_SIZE / 2),
						    y + (BOX_SIZE / 2),
						    color)) {
				if (!piglit_automatic)
					printf("%s failed on ref = "
					       "{ %.01f %.01f %.01f %.01f }\n",
					       tests[idx].opcode,
					       ref[0], ref[1], ref[2], ref[2]);

				result = PIGLIT_FAIL;
			}
		}
	}


	piglit_present_results();
	return result;
}


void
piglit_init(int argc, char **argv)
{
	unsigned i;

	(void) argc;
	(void) argv;

	piglit_require_fragment_program();
	piglit_require_extension("GL_NV_fragment_program_option");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	reference_prog = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB,
						reference_shader_source);

	for (i = 0; i < ARRAY_SIZE(tests); i++) {
		char shader_source[512];

		sprintf(shader_source, shader_template, tests[i].opcode);
		progs[i] = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB,
						  shader_source);
	}


	glClearColor(0.5, 0.5, 0.5, 1.0);
}
