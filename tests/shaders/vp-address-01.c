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
 * \file vp-address-01.c
 * Validate address registers with various constant offsets.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util.h"
#include "piglit-framework.h"

static const GLfloat attrib[] = {
	1.0,
	2.0,
	0.0,
	-1.0,
	-2.0
};

#define ELEMENTS(x)  (sizeof(x) / sizeof(x[0]))

#define TEST_ROWS  1
#define TEST_COLS  ELEMENTS(attrib)
#define BOX_SIZE   32


int piglit_window_mode = GLUT_DOUBLE;
int piglit_width = (((BOX_SIZE+1)*TEST_COLS)+1);
int piglit_height = (((BOX_SIZE+1)*TEST_ROWS)+1);


static const char vertex_source_template[] =
	"!!ARBvp1.0\n"
	"PARAM	colors[] = { program.env[0..3] };\n"
	"ADDRESS	A0;\n"
	"\n"
	"ARL	A0.x, vertex.attrib[1].x;\n"
	"MOV	result.color, colors[A0.x %c %u];\n"
	PIGLIT_VERTEX_PROGRAM_MVP_TRANSFORM
	"END\n"
	;


/**
 * \name Handles to programs.
 */
/*@{*/
static GLint progs[TEST_COLS];
/*@}*/


enum piglit_result
piglit_display(void)
{
	static const GLfloat color[4] = { 0.0, 1.0, 0.0, 1.0 };
	static const GLfloat good_color[4] = { 0.0, 1.0, 0.0, 1.0 };
	static const GLfloat bad_color[4] = { 1.0, 0.0, 0.0, 1.0 };
	int result = PIGLIT_SUCCESS;
	unsigned i;

	glClear(GL_COLOR_BUFFER_BIT);

	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 0, bad_color);
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 1, color);
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 2, bad_color);
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 3, bad_color);

	for (i = 0; i < ELEMENTS(progs); i++) {
		const int x = 1 + (i * (BOX_SIZE + 1));

		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, progs[i]);

		glVertexAttrib1fARB(1, attrib[i]);

		piglit_draw_rect(x, 1, BOX_SIZE, BOX_SIZE);

		if (!piglit_probe_pixel_rgb(x + (BOX_SIZE / 2),
					    1 + (BOX_SIZE / 2),
					    good_color)) {
			if (! piglit_automatic)
				printf("shader %u failed with attribute "
				       "%.1f\n", i, attrib[i]);

			result = PIGLIT_FAILURE;
		}
	}

	glutSwapBuffers();
	return result;
}


void
piglit_init(int argc, char **argv)
{
	GLint max_address_registers;
	unsigned i;

	(void) argc;
	(void) argv;

	piglit_require_vertex_program();
	piglit_require_fragment_program();
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glGetProgramivARB(GL_VERTEX_PROGRAM_ARB,
			  GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB,
			  & max_address_registers);
	if (max_address_registers == 0) {
		if (! piglit_automatic)
			printf("GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB "
			       "== 0");

		piglit_report_result(PIGLIT_FAILURE);
	}

	for (i = 0; i < ELEMENTS(progs); i++) {
		char shader_source[1024];
		int offset[2];
		char direction[2];

		/* We want the constant offset in the instruction plus the
		 * value read from the attribute to be 1.
		 */
		offset[0] = 1 - (int) attrib[i];

		if (offset[0] < 0) {
			direction[0] = '-';
			offset[0] = -offset[0];
		} else {
			direction[0] = '+';
		}

		snprintf(shader_source, sizeof(shader_source),
			 vertex_source_template,
			 direction[0], offset[0]);

		progs[i] = piglit_compile_program(GL_VERTEX_PROGRAM_ARB,
						  shader_source);
	}

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glEnable(GL_VERTEX_PROGRAM_ARB);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, piglit_ARBfp_pass_through);

	glClearColor(0.5, 0.5, 0.5, 1.0);
}
