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
 * \file vp-address-05.c
 * Simple test of the ARA instruction from GL_NV_vertex_program2_option.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl.h"

#define TEST_ROWS  1
#define TEST_COLS  4
#define BOX_SIZE   32

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = (((BOX_SIZE+1)*TEST_COLS)+1);
	config.window_height = (((BOX_SIZE+1)*TEST_ROWS)+1);
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vertex_source_template[] =
	"!!ARBvp1.0\n"
	"OPTION	NV_vertex_program2;\n"
	"PARAM	colors[] = { program.env[0..3] };\n"
	"ADDRESS	A0;\n"
	"\n"
	"ARL	A0, vertex.attrib[1];\n"
	"ARA	A0.xy, A0;\n"
	"MOV	result.color, colors[A0.%c];\n"
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
	static const GLfloat bad_color[4] = { 1.0, 0.0, 0.0, 1.0 };
	static const GLfloat attrib[4][4] = {
		{ 1.0, -37.0, 1.0, 68.2 },
		{ -37.0, 1.0, 68.2, 1.0 },
		{ 0.0, 3.0, 2.0, 1.0 },
		{ 0.0, 3.0, 1.0, 2.0 },
	};
	enum piglit_result result = PIGLIT_PASS;
	unsigned i;

	glClear(GL_COLOR_BUFFER_BIT);

	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 0, bad_color);
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 1, bad_color);
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 2, color);
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 3, bad_color);

	for (i = 0; i < ARRAY_SIZE(progs); i++) {
		const int x = 1 + (i * (BOX_SIZE + 1));

		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, progs[i]);

		glVertexAttrib4fvARB(1, attrib[i]);

		piglit_draw_rect(x, 1, BOX_SIZE, BOX_SIZE);

		if (!piglit_probe_pixel_rgb(x + (BOX_SIZE / 2),
					    1 + (BOX_SIZE / 2),
					    color)) {
			result = PIGLIT_FAIL;
		}
	}

	piglit_present_results();
	return result;
}


void
piglit_init(int argc, char **argv)
{
	static const char components[] = "xyzw";
	unsigned i;

	(void) argc;
	(void) argv;

	piglit_require_vertex_program();
	piglit_require_fragment_program();
	piglit_require_extension("GL_NV_vertex_program2_option");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	for (i = 0; i < ARRAY_SIZE(progs); i++) {
		char shader_source[1024];

		snprintf(shader_source, sizeof(shader_source),
			 vertex_source_template, components[i]);

		progs[i] = piglit_compile_program(GL_VERTEX_PROGRAM_ARB,
						  shader_source);
	}

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glEnable(GL_VERTEX_PROGRAM_ARB);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, piglit_ARBfp_pass_through);

	glClearColor(0.5, 0.5, 0.5, 1.0);
}
