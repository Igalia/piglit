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
 * \file fp-condition_codes-01.c
 * Validate that the correct components of the condition code register are
 * set to the desired value.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl.h"

/* One for the reference square and one for each possible condition code.
 */
#define TEST_ROWS  (1 + 6)

/* One for each possible non-empty write mask.
 */
#define TEST_COLS  (15)

#define BOX_SIZE   16

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = (((BOX_SIZE+1)*TEST_ROWS)+1);
	config.window_height = (((BOX_SIZE+1)*TEST_COLS)+1);
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define INVERT_MASK(x) (~(x) & 0x0f)

static char shader_source[64 * 1024];


/**
 * Strings for binary write-masks.
 *
 * Bit 0 corresponds to X, bit 1 to y, etc.
 */
static const char *const mask_strings[16] = {
	"<empty>",
	"x",
	"y",
	"xy",
	"z",
	"xz",
	"yz",
	"xyz",
	"w",
	"xw",
	"yw",
	"xyw",
	"zw",
	"xzw",
	"yzw",
	"xyzw",
};


/**
 * Strings for condition codes.
 */
static const char *const cc_strings[6] = {
	"EQ",
	"GE",
	"GT",
	"LE",
	"LT",
	"NE",
};


/**
 * Constant values that will set required condition codes
 *
 * The even values set the parallel condition code in \c cc_strings, and the
 * odd values set something else.  For example, element 4 sets GT, and element
 * 5 does not (it sets LT).
 */
static const GLfloat cc_values[12] = {
	0.5, 1.0,
	1.0, 0.0,
	1.0, 0.0,
	0.0, 1.0,
	0.0, 1.0,
	0.0, 0.5
};


/**
 * Source for the fragment program to render the reference box.
 */
static const char reference_shader_source[] =
	"!!ARBfp1.0\n"
	"MOV	result.color, program.env[0];\n"
	"END"
	;

/**
 * \name Handles to fragment programs.
 */
/*@{*/
static GLint reference_prog;
static GLint progs[TEST_ROWS * TEST_COLS];
/*@}*/


void
generate_shader(unsigned cc, unsigned good_mask)
{
	unsigned len;
	static const char *const swiz[16] = {
		"yyyy", "xyyy", "yxyy", "xxyy",
		"yyxy", "xyxy", "yxxy", "xxxy",
		"yyyx", "xyyx", "yxyx", "xxyx",
		"yyxx", "xyxx", "yxxx", "xxxx",
	};


	len = sprintf(& shader_source[0],
		       "!!ARBfp1.0\n"
		       "OPTION	NV_fragment_program;\n"
		       "PARAM	good = program.env[0];\n"
		       "PARAM	junk = program.env[1];\n"
		       "TEMP	R0, R1, R2;\n"
		       "\n"
		       "# Create a combination of good and bad data in R0.\n"
		      "MOV	R0, junk;\n");


	if (good_mask != 0) {
		len += sprintf(& shader_source[len],
			       "MOV	R0.%s, good;\n",
			       mask_strings[good_mask]);
	}

	len += sprintf(& shader_source[len],
		       "\n"
		       "# Set the condition codes.  Inputs are on the range\n"
		       "# [0, 1], so the range must be expanded to [-1, 1].\n"
		       "MADC	R2, fragment.color.%s, 2.0, -1.0;\n"
		       "\n"
		       "# Create a combination of good and bad data in R1.\n"
		       "# The components in R0 that already have good data\n"
		       "# should have bad data in R1.\n"
		       "MOV	R1, good;\n",
		       swiz[INVERT_MASK(good_mask)]);

	if (good_mask != 0) {
		len += sprintf(& shader_source[len],
			       "MOV	R1.%s, junk;\n",
			       mask_strings[good_mask]);
	}

	len += sprintf(& shader_source[len],
		       "\n"
		       "# Fill remaining bits of R0 with good data from R1.\n"
		       "# Write that data to the shader output.\n"
		       "MOV	R0 (%s.xyzw), R1;\n"
		       "MOV	result.color, R0;\n"
		       "END\n",
		       cc_strings[cc]);
}


enum piglit_result
piglit_display(void)
{
	static const GLfloat good_color[4] = { 0.9, 0.5, 0.7, 1.0 };
	static const GLfloat junk_color[4] = { 0.2, 0.2, 0.2, 1.0 };
	unsigned i = 0;
	unsigned cc;
	unsigned mask;
	enum piglit_result result = PIGLIT_PASS;


	glClear(GL_COLOR_BUFFER_BIT);

	glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB,
				    0, good_color);
	glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB,
				    1, junk_color);

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, reference_prog);
	piglit_draw_rect(1, 1, BOX_SIZE, BOX_SIZE);

	for (cc = 0; cc < 6; cc++) {
		glColor4f(cc_values[(cc * 2) + 0], cc_values[(cc * 2) + 1],
			  0.0, 1.0);

		for (mask = 0; mask < 0x0f; mask++) {
			const int x = ((cc + 1) * (BOX_SIZE + 1)) + 1;
			const int y = (mask * (BOX_SIZE + 1)) + 1;
			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, progs[i]);
			piglit_draw_rect(x, y, BOX_SIZE, BOX_SIZE);
			i++;

			if (!piglit_probe_pixel_rgb(x + (BOX_SIZE / 2),
						    y + (BOX_SIZE / 2),
						    good_color)) {
				if (!piglit_automatic)
					printf("CC %s with mask %s failed.\n",
					       cc_strings[cc],
					       mask_strings[mask]);

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
	unsigned cc;
	unsigned mask;
	unsigned i;

	(void) argc;
	(void) argv;

	piglit_require_fragment_program();
	piglit_require_extension("GL_NV_fragment_program_option");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	reference_prog = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB,
						reference_shader_source);

	i = 0;
	for (cc = 0; cc < 6; cc++) {
		for (mask = 0; mask < 0x0f; mask++) {
			generate_shader(cc, mask);
			progs[i] =
				piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB,
						       shader_source);
			i++;
		}
	}
}
