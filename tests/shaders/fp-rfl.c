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
 * \file fp-rfl.c
 * Validate the RFL instruction in GL_NV_fragment_program_option.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include <time.h>

#include "piglit-util-gl.h"

#define BOX_SIZE   8

/* As many columns as will fit in 640.
 */
#define TEST_COLS  ((640 - 1) / (BOX_SIZE + 1))

/* As many rows as will fit in 400.
 */
#define TEST_ROWS  ((400 - 1) / (BOX_SIZE + 1))

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = (((BOX_SIZE+1)*TEST_COLS)+1);
	config.window_height = (((BOX_SIZE+1)*TEST_ROWS)+1);
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vert_shader_source[] =
	"!!ARBvp1.0\n"
	"ATTRIB	iPos = vertex.position;\n"
	"OUTPUT	oPos = result.position;\n"
	"PARAM	mvp[4] = { state.matrix.mvp };\n"
	"DP4	oPos.x, mvp[0], iPos;\n"
	"DP4	oPos.y, mvp[1], iPos;\n"
	"DP4	oPos.z, mvp[2], iPos;\n"
	"DP4	oPos.w, mvp[3], iPos;\n"
	"MOV	result.texcoord[0], vertex.attrib[1];\n"
	"MOV	result.texcoord[1], vertex.attrib[2];\n"
	"END"
	;

static const char frag_shader_source[] =
	"!!ARBfp1.0\n"
#if 1
	"OPTION	NV_fragment_program;\n"
	"TEMP	tmp, axis, direction;\n"
	"\n"
	"# Since we're calcuating per-fragment and the parameters are\n"
	"# interpolated, the parameters must be normalized.\n"
	"DP3	tmp.x, fragment.texcoord[0], fragment.texcoord[0];\n"
	"DP3	tmp.y, fragment.texcoord[1], fragment.texcoord[1];\n"
	"RCP	tmp.x, tmp.x;\n"
	"RCP	tmp.y, tmp.y;\n"
	"MUL	axis, tmp.x, fragment.texcoord[0];\n"
	"MUL	direction, tmp.y, fragment.texcoord[1];\n"
	"RFL	result.color, fragment.texcoord[0], fragment.texcoord[1];\n"
#else
	"TEMP	tmp, axis, direction;\n"
	"DP3	tmp.x, fragment.texcoord[0], fragment.texcoord[0];\n"
	"DP3	tmp.y, fragment.texcoord[1], fragment.texcoord[1];\n"
	"RCP	tmp.x, tmp.x;\n"
	"RCP	tmp.y, tmp.y;\n"
	"MUL	axis, tmp.x, fragment.texcoord[0];\n"
	"MUL	direction, tmp.y, fragment.texcoord[1];\n"

	/* This is the open-coded version of the RFL instruction.  It was used
	 * during the development of the test.  Since it maybe useful in future
	 * debugging, I am leaving it in.
	 */
	"DP3	tmp.w, axis, axis;\n"
	"DP3	tmp.x, axis, direction;\n"
	"MUL	tmp.x, 2.0.x, tmp.x;\n"

	"RCP	tmp.w, tmp.w;\n"
	"MUL	tmp.x, tmp.x, tmp.w;\n"

	"MAD	result.color.xyz, tmp.xxxx, axis, -direction;\n"
	"MOV	result.color.w, {0.0};\n"
#endif
	"END"
	;

/**
 * \name Handles to programs.
 */
/*@{*/
static GLint vert_prog;
static GLint frag_prog;
/*@}*/


static GLfloat direction[4 * TEST_ROWS * TEST_COLS];
static GLfloat axis[4 * TEST_ROWS * TEST_COLS];
static GLfloat position[4 * TEST_ROWS * TEST_COLS];

static const float green[4] = { 0.0, 1.0, 0.0, 0.0 };

enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_PASS;
	unsigned r;
	unsigned c;


	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glEnable(GL_VERTEX_PROGRAM_ARB);

	glVertexAttribPointerARB(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
				 position);
	glVertexAttribPointerARB(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
				 axis);
	glVertexAttribPointerARB(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
				 direction);
	glEnableVertexAttribArrayARB(0);
	glEnableVertexAttribArrayARB(1);
	glEnableVertexAttribArrayARB(2);

	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vert_prog);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, frag_prog);

	glPointSize((float) BOX_SIZE);
	glDrawArrays(GL_POINTS, 0, TEST_ROWS * TEST_COLS);

	for (r = 0; r < TEST_ROWS; r++) {
		for (c = 0; c < TEST_COLS; c++) {
			const int idx = 4 * ((r * TEST_COLS) + c);

			if (!piglit_probe_pixel_rgb(position[idx + 0],
						    position[idx + 1],
						    green)) {
				printf("direction = { %.2f %.2f %.2f }\n"
				       "axis      = { %.2f %.2f %.2f }\n",
				       direction[idx + 0],
				       direction[idx + 1],
				       direction[idx + 2],
				       axis[idx + 0],
				       axis[idx + 1],
				       axis[idx + 2]);
			}
		}
	}

	piglit_present_results();
	return result;
}


static double
random_float(void)
{
	return (double) rand() / (double) rand();
}


void
piglit_init(int argc, char **argv)
{
	unsigned r;
	unsigned c;
	unsigned i;


	(void) argc;
	(void) argv;

	piglit_require_vertex_program();
	piglit_require_fragment_program();
	piglit_require_extension("GL_NV_fragment_program_option");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	vert_prog = piglit_compile_program(GL_VERTEX_PROGRAM_ARB,
					   vert_shader_source);
	frag_prog = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB,
					   frag_shader_source);

	glClearColor(0.5, 0.5, 0.5, 1.0);

	i = 0;
	for (r = 0; r < TEST_ROWS; r++) {
		for (c = 0; c < TEST_COLS; c++) {
			position[i + 0] = (float)((BOX_SIZE / 2) + c *
						  (BOX_SIZE + 1) + 1);
			position[i + 1] = (float)((BOX_SIZE / 2) + r *
						  (BOX_SIZE + 1) + 1);
			position[i + 2] = 0.0f;
			position[i + 3] = 1.0f;
			i += 4;
		}
	}


	/* Generate a bunch of random direction vectors.  Based on the random
	 * direction vector, generate an axis such that the reflection of the
	 * random vector across the axis is { 0, 1, 0 }.
	 */
	srand(time(NULL));
	for (i = 0; i < (ARRAY_SIZE(direction) / 4); i++) {
		const double d[3] = {
			random_float(),
			random_float(),
			random_float()
		};
		const double inv_mag_d = 1.0 /
			sqrt((d[0] * d[0]) + (d[1] * d[1]) + (d[2] * d[2]));
		double a[3];
		double mag_a;


		direction[(i * 4) + 0] = d[0] * inv_mag_d;
		direction[(i * 4) + 1] = d[1] * inv_mag_d;
		direction[(i * 4) + 2] = d[2] * inv_mag_d;
		direction[(i * 4) + 3] = 0.0;

		a[0] = direction[(i * 4) + 0] + 0.0;
		a[1] = direction[(i * 4) + 1] + 1.0;
		a[2] = direction[(i * 4) + 2] + 0.0;
		mag_a = sqrt((a[0] * a[0]) + (a[1] * a[1]) + (a[2] * a[2]));

		axis[(i * 4) + 0] = a[0] / mag_a;
		axis[(i * 4) + 1] = a[1] / mag_a;
		axis[(i * 4) + 2] = a[2] / mag_a;
		axis[(i * 4) + 3] = 0.0;
	}
}
