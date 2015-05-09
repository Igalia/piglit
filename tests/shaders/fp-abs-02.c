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
 * \file fp-abs-02.c
 * Validate the absolute value operand modifier in GL_NV_fragment_program_option
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl.h"

#define TEST_ROWS  1
#define TEST_COLS  3
#define BOX_SIZE   32

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = (((BOX_SIZE+1)*TEST_COLS)+1);
	config.window_height = (((BOX_SIZE+1)*TEST_ROWS)+1);
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char cos_shader_source[] =
	"!!ARBfp1.0\n"
	"OPTION	NV_fragment_program;\n"
	"ATTRIB	input0 = fragment.texcoord[0];\n"
	"ATTRIB	input1 = fragment.texcoord[1];\n"
	"TEMP	R0, R1, R2;\n"
	"\n"
	"MOV	R2, {0.0, 0.0, 0.0, 1.0};\n"
	"\n"
	"# Assume that input1.x is 1.0.  COS(PI) is -1.  This means\n"
	"# that R2.y should end up with -1.0.\n"
	"MUL	R0, input1.x, 3.14159265358979323846;\n"
	"COS	R0, R0.x;\n"
	"DP4	R2.y, R0, 0.25;\n"
	"\n"
	"MOV	result.color, |R2|;\n"
	"END\n"
	;

static const char sne_shader_source[] =
	"!!ARBfp1.0\n"
	"OPTION	NV_fragment_program;\n"
	"ATTRIB	input0 = fragment.texcoord[0];\n"
	"ATTRIB	input1 = fragment.texcoord[1];\n"
	"TEMP	R0, R1, R2;\n"
	"\n"
	"MOV	R2, {0.0, 1.0, 0.0, 1.0};\n"
	"\n"
	"# By convention, all components of input0 are < 0.0, and\n"
	"# input0 = -input1.\n"
	"# The dot-product compacts the four components into a single\n"
	"# component.  R2.x should be 0.0.\n"
	"ADD	R0, -input1, |input0|;\n"
	"SNE	R1, R0, 0.0;\n"
	"DP4	R2.x, R1, 1.0;\n"
	"\n"
	"# If R2.x is not 0.0 as it should be, set R2.y != 1.0\n"
	"DP3	R1, R2.xxxx, 1.0;\n"
	"SUB	R2.y, R2, R1;\n"
	"\n"
	"MOV	result.color, R2;\n"
	"END\n"
	;

static const char addc_shader_source[] =
	"!!ARBfp1.0\n"
	"OPTION	NV_fragment_program;\n"
	"ATTRIB	input0 = fragment.texcoord[0];\n"
	"ATTRIB	input1 = fragment.texcoord[1];\n"
	"TEMP	R0, R1, R2;\n"
	"\n"
	"MOV	R2, {0.0, 1.0, 0.0, 1.0};\n"
	"\n"
	"# By convention, all components of input0 are < 0.0, and\n"
	"# input0 = -input1.\n"
	"# The dot-product compacts the four components into a single\n"
	"# component.  R2.x should be 0.0.\n"
	"ADDC	R0, -input1, |input0|;\n"
	"MOV	R1 (NE), 1.0;\n"
	"DP4	R2.x, R1, 1.0;\n"
	"\n"
	"# If R2.x is not 0.0 as it should be, set R2.y != 1.0\n"
	"DP3	R1, R2.xxxx, 1.0;\n"
	"SUB	R2.y, R2, R1;\n"
	"\n"
	"MOV	result.color, R2;\n"
	"END\n"
	;

static const char vert_shader_source[] =
	"!!ARBvp1.0\n"
	"ATTRIB	iPos = vertex.position;\n"
	"OUTPUT	oPos = result.position;\n"
	"PARAM	mvp[4] = { state.matrix.mvp };\n"
	"DP4	oPos.x, mvp[0], iPos;\n"
	"DP4	oPos.y, mvp[1], iPos;\n"
	"DP4	oPos.z, mvp[2], iPos;\n"
	"DP4	oPos.w, mvp[3], iPos;\n"
	"MOV	result.texcoord[0], -vertex.color;\n"
	"MOV	result.texcoord[1], vertex.color;\n"
	"END"
	;

/**
 * \name Handles to fragment programs.
 */
/*@{*/
static GLint progs[TEST_COLS];
static GLint vert_prog;
/*@}*/


enum piglit_result
piglit_display(void)
{
	static const GLfloat color[4] = { 0.0, 1.0, 0.0, 0.0 };
	enum piglit_result result = PIGLIT_PASS;
	unsigned i;

	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glEnable(GL_VERTEX_PROGRAM_ARB);

	glColor4f(1.0, 0.6, 0.3, 0.1);

	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vert_prog);

	for (i = 0; i < ARRAY_SIZE(progs); i++) {
		const int x = 1 + (i * (BOX_SIZE + 1));

		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, progs[i]);

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
	(void) argc;
	(void) argv;

	piglit_require_vertex_program();
	piglit_require_fragment_program();
	piglit_require_extension("GL_NV_fragment_program_option");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	progs[0] = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB,
					  cos_shader_source);
	progs[1] = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB,
					  sne_shader_source);
	progs[2] = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB,
					  addc_shader_source);

	vert_prog = piglit_compile_program(GL_VERTEX_PROGRAM_ARB,
					   vert_shader_source);

	glClearColor(0.5, 0.5, 0.5, 1.0);
}
