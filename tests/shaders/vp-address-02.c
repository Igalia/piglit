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
 * \file vp-address-02.c
 * Validate address registers with various constant offsets.
 *
 * Much like vp-address-01, but this test utilizes multiple address registers.
 * GL_NV_vertex_program2_option requires at least two address registers.  Base
 * GL_ARB_vertex_program implementations can also support more than one, but
 * only one is required.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl.h"

static const GLfloat attrib[] = {
	1.0, 1.0,
	2.0, 2.0,
	0.0, 0.0,
	1.0, 0.0,
	-1.0, 0.0,
	0.0, 1.0,
	0.0, -1.0,
	2.0, -1.0,
	-2.0, -1.0,
	2.0, -2.0,
};

#define TEST_ROWS  1
#define TEST_COLS  (ARRAY_SIZE(attrib) / 2)
#define BOX_SIZE   32

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = (((BOX_SIZE+1)*TEST_COLS)+1);
	config.window_height = (((BOX_SIZE+1)*TEST_ROWS)+1);
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vertex_source_template[] =
	"!!ARBvp1.0\n"
	"PARAM	colors[] = { program.env[0..3] };\n"
	"ADDRESS	A0, A1;\n"
	"\n"
	"ARL	A0.x, vertex.attrib[1].x;\n"
	"ARL	A1.x, vertex.attrib[1].y;\n"
	"ADD	result.color, colors[A0.x %c %u], colors[A1.x %c %u];\n"
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
	static const GLfloat color[4] = { 0.0, 0.5, 0.0, 0.5 };
	static const GLfloat good_color[4] = { 0.0, 1.0, 0.0, 1.0 };
	static const GLfloat bad_color[4] = { 1.0, 0.0, 0.0, 1.0 };
	enum piglit_result result = PIGLIT_PASS;
	unsigned i;

	glClear(GL_COLOR_BUFFER_BIT);

	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 0, bad_color);
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 1, color);
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 2, bad_color);
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 3, bad_color);

	for (i = 0; i < ARRAY_SIZE(progs); i++) {
		const int x = 1 + (i * (BOX_SIZE + 1));

		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, progs[i]);

		glVertexAttrib2fvARB(1, & attrib[i * 2]);

		piglit_draw_rect(x, 1, BOX_SIZE, BOX_SIZE);

		if (!piglit_probe_pixel_rgb(x + (BOX_SIZE / 2),
					    1 + (BOX_SIZE / 2),
					    good_color)) {
			if (! piglit_automatic)
				printf("shader %u failed with attributes "
				       "%.1f, %.1f\n",
				       i,
				       attrib[(i * 2) + 0],
				       attrib[(i * 2) + 1]);

			result = PIGLIT_FAIL;
		}
	}

	piglit_present_results();
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
			  GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB,
			  & max_address_registers);
	if (max_address_registers == 0) {
		/* we have to have at least one address register */
		if (! piglit_automatic)
			printf("GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB == 0\n");

		piglit_report_result(PIGLIT_FAIL);
	} else 	if (max_address_registers == 1) {
		if (piglit_is_extension_supported("GL_NV_vertex_program2_option")) {
			/* this extension requires two address regs */
			if (! piglit_automatic)
				printf("GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB "
				       "== 1\n");

			piglit_report_result(PIGLIT_FAIL);
		} else {
			piglit_report_result(PIGLIT_SKIP);
		}
	}

	for (i = 0; i < ARRAY_SIZE(progs); i++) {
		char shader_source[1024];
		int offset[2];
		char direction[2];

		/* We want the constant offset in the instruction plus the
		 * value read from the attribute to be 1.
		 */
		offset[0] = 1 - (int) attrib[(2 * i) + 0];
		offset[1] = 1 - (int) attrib[(2 * i) + 1];

		if (offset[0] < 0) {
			direction[0] = '-';
			offset[0] = -offset[0];
		} else {
			direction[0] = '+';
		}

		if (offset[1] < 0) {
			direction[1] = '-';
			offset[1] = -offset[1];
		} else {
			direction[1] = '+';
		}

		snprintf(shader_source, sizeof(shader_source),
			 vertex_source_template,
			 direction[0], offset[0],
			 direction[1], offset[1]);

		progs[i] = piglit_compile_program(GL_VERTEX_PROGRAM_ARB,
						  shader_source);
	}

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glEnable(GL_VERTEX_PROGRAM_ARB);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, piglit_ARBfp_pass_through);

	glClearColor(0.5, 0.5, 0.5, 1.0);
}
