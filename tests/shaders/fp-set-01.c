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
 * \file fp-set-01.c
 * Validate the two set-on instructions in GL_ARB_fragment_program
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl.h"

/* One column for each possible combination of set-on results
 */
#define TEST_COLS  16

/* One for the reference square and each of the 2 set-on instructions
 */
#define TEST_ROWS  3

#define BOX_SIZE   16

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = (((BOX_SIZE+1)*TEST_COLS)+1);
	config.window_height = (((BOX_SIZE+1)*TEST_ROWS)+1);
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Source for the fragment program to render the reference box.
 */
static const char reference_shader_source[] =
	"!!ARBfp1.0\n"
	"MOV	result.color, program.env[0];\n"
	"END"
	;

static const char slt_shader_source[] =
	"!!ARBfp1.0\n"
	"SLT	result.color, program.env[1], fragment.color;\n"
	"END"
	;

static const char sge_shader_source[] =
	"!!ARBfp1.0\n"
	"SGE	result.color, fragment.color, program.env[1];\n"
	"END"
	;

/**
 * \name Handles to fragment programs.
 */
/*@{*/
static GLint reference_prog;
static GLint progs[2];
/*@}*/


enum piglit_result
piglit_display(void)
{
	const GLfloat comparitor[4] = { 0.5, 0.5, 0.5, 0.5 };
	unsigned i;
	unsigned j;
	enum piglit_result result = PIGLIT_PASS;


	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	for (i = 0; i < TEST_COLS; i++) {
		const int x = (i * (BOX_SIZE + 1)) + 1;
		GLfloat color[4];

		color[0] = (i & 0x01) ? 1.0 : 0.0;
		color[1] = (i & 0x02) ? 1.0 : 0.0;
		color[2] = (i & 0x04) ? 1.0 : 0.0;
		color[3] = (i & 0x08) ? 1.0 : 0.0;

		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, reference_prog);

		glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0,
					    color);
		glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1,
					    comparitor);

		piglit_draw_rect(x, 1, BOX_SIZE, BOX_SIZE);

		glColor4fv(color);

		for (j = 0; j < ARRAY_SIZE(progs); j++) {
			const int y = ((j + 1) * (BOX_SIZE + 1)) + 1;

			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, progs[j]);
			piglit_draw_rect(x, y, BOX_SIZE, BOX_SIZE);

			if (!piglit_probe_pixel_rgb(x + (BOX_SIZE / 2),
						    y + (BOX_SIZE / 2),
						    color)) {
				if (!piglit_automatic)
                                        printf("shader %u failed on index %u\n",
					       j, i);

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
	(void) argc;
	(void) argv;

	piglit_require_fragment_program();
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	reference_prog = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB,
						reference_shader_source);

	progs[0] = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB,
					  slt_shader_source);
	progs[1] = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB,
					  sge_shader_source);

	glClearColor(1.0, 1.0, 1.0, 1.0);
}
