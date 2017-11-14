/*
 * Copyright (C) 1999  Allen Akin   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

/**
 * @file combine.c:  Test the GL_EXT_texture_env_combine extension
 * Author: Brian Paul (brianp@valinux.com)  September 2000
 *
 * GL_EXT_texture_env_dot3 extension test
 * Author: Gareth Hughes (gareth@valinux.com)  January 2001
 *
 * GL_ARB_texture_env_crossbar extension test
 * Author: Brian Paul (brian@tungstengraphics.com)  December 2002
 *
 * The challenge with testing this extension is dealing with combinatorial
 * explosion.  There are 16 state variables in this extension:
 *
 * GL_COMBINE_RGB_EXT which has 5 possible values
 * GL_COMBINE_ALPHA_EXT which has 5 possible values
 * GL_SOURCE0_RGB_EXT which has 4 possible values
 * GL_SOURCE1_RGB_EXT which has 4 possible values
 * GL_SOURCE2_RGB_EXT which has 4 possible values
 * GL_SOURCE0_ALPHA_EXT which has 4 possible values
 * GL_SOURCE1_ALPHA_EXT which has 4 possible values
 * GL_SOURCE2_ALPHA_EXT which has 4 possible values
 * GL_OPERAND0_RGB_EXT which has 4 possible values
 * GL_OPERAND1_RGB_EXT which has 4 possible values
 * GL_OPERAND2_RGB_EXT which has 2 possible values
 * GL_OPERAND0_ALPHA_EXT which has 2 possible values
 * GL_OPERAND1_ALPHA_EXT which has 2 possible values
 * GL_OPERAND2_ALPHA_EXT which has 1 possible value
 * GL_RGB_SCALE_EXT which has 3 possible values
 * GL_ALPHA_SCALE which has 3 possible values
 *
 * The product of those values is 117,964,800.  And that's just for one
 * texture unit!  If we wanted to fully exercise N texture units we'd
 * need to run 117,964,800 ^ N tests!  Ideally we'd also like to test
 * with a number of different fragment, texenv and texture colors.
 * Clearly we can't test everything.
 *
 * So, we've partitioned the combination space into subsets defined
 * by the replace_params[], add_params[], interpolate_params[], etc arrays.
 * For multitexture, we do an even more limited set of tests:  testing
 * all permutations of the 5 combine modes on all texture units.
 *
 * In the future we might look at programs that use the combine
 * extension to see which mode combination are important to them and
 * put them into this test.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
	config.window_width = 2;
	config.window_height = 2;

PIGLIT_GL_TEST_CONFIG_END

#define MAX_TEX_UNITS 8

#define COPY4(DST, SRC)                                                      \
{                                                                            \
	(DST)[0] = (SRC)[0];                                                 \
	(DST)[1] = (SRC)[1];                                                 \
	(DST)[2] = (SRC)[2];                                                 \
	(DST)[3] = (SRC)[3];                                                 \
}

static bool have_dot3;
static bool have_crossbar;
static bool have_combine3;
static GLuint textures[MAX_TEX_UNITS];
static int test_stride = 1;
static int num_tex_units;

/* Our model of GL machine state */
static struct {
	GLenum COMBINE_RGB[MAX_TEX_UNITS];
	GLenum COMBINE_ALPHA[MAX_TEX_UNITS];
	GLenum SOURCE0_RGB[MAX_TEX_UNITS];
	GLenum SOURCE1_RGB[MAX_TEX_UNITS];
	GLenum SOURCE2_RGB[MAX_TEX_UNITS];
	GLenum SOURCE0_ALPHA[MAX_TEX_UNITS];
	GLenum SOURCE1_ALPHA[MAX_TEX_UNITS];
	GLenum SOURCE2_ALPHA[MAX_TEX_UNITS];
	GLenum OPERAND0_RGB[MAX_TEX_UNITS];
	GLenum OPERAND1_RGB[MAX_TEX_UNITS];
	GLenum OPERAND2_RGB[MAX_TEX_UNITS];
	GLenum OPERAND0_ALPHA[MAX_TEX_UNITS];
	GLenum OPERAND1_ALPHA[MAX_TEX_UNITS];
	GLenum OPERAND2_ALPHA[MAX_TEX_UNITS];
	float RGB_SCALE[MAX_TEX_UNITS];
	float ALPHA_SCALE[MAX_TEX_UNITS];
	float frag_color[4];		   /* fragment color */
	float env_color[MAX_TEX_UNITS][4]; /* texture env color */
	float tex_color[MAX_TEX_UNITS][4]; /* texture image color */
	GLenum tex_format[MAX_TEX_UNITS];  /* texture base format */
} machine;

/* describes possible state combinations */
struct test_param {
	GLenum target;
	GLenum valid_values[6];
};

/* These objects define the space of tex-env combinations that we exercise.
 * Each array element is { state-var, { list of possible values, 0 } }.
 */

static const struct test_param replace_params[] = {
	{GL_COMBINE_RGB_EXT, {GL_REPLACE, 0}},
	{GL_COMBINE_ALPHA_EXT, {GL_REPLACE, 0}},
	{GL_SOURCE0_RGB_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_SOURCE0_ALPHA_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_OPERAND0_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND0_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_RGB_SCALE_EXT, {1, 2, 4, 0}},
	{GL_ALPHA_SCALE, {1, 2, 4, 0}},
	{0, {0, 0, 0, 0, 0}}
};

static const struct test_param add_params[] = {
	{GL_COMBINE_RGB_EXT, {GL_ADD, 0}},
	{GL_COMBINE_ALPHA_EXT, {GL_ADD, 0}},
	{GL_SOURCE0_RGB_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_SOURCE1_RGB_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_SOURCE0_ALPHA_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_SOURCE1_ALPHA_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_OPERAND0_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND0_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_RGB_SCALE_EXT, {1, 2, 4, 0}},
	{GL_ALPHA_SCALE, {1, 2, 4, 0}},
	{0, {0, 0, 0, 0, 0}}
};

static const struct test_param modulate_params[] = {
	{GL_COMBINE_RGB_EXT, {GL_MODULATE, 0}},
	{GL_COMBINE_ALPHA_EXT, {GL_MODULATE, 0}},
	{GL_SOURCE0_RGB_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE1_RGB_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_SOURCE0_ALPHA_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE1_ALPHA_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_OPERAND0_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND0_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_RGB_SCALE_EXT, {1, 2, 4, 0}},
	{GL_ALPHA_SCALE, {1, 2, 4, 0}},
	{0, {0, 0, 0, 0, 0}}
};

static const struct test_param add_signed_params[] = {
	{GL_COMBINE_RGB_EXT, {GL_ADD_SIGNED_EXT, 0}},
	{GL_COMBINE_ALPHA_EXT, {GL_ADD_SIGNED_EXT, 0}},
	{GL_SOURCE0_RGB_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE1_RGB_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_SOURCE0_ALPHA_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE1_ALPHA_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_OPERAND0_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND0_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_RGB_SCALE_EXT, {1, 2, 4, 0}},
	{GL_ALPHA_SCALE, {1, 2, 4, 0}},
	{0, {0, 0, 0, 0, 0}}
};

static const struct test_param interpolate_params[] = {
	{GL_COMBINE_RGB_EXT, {GL_INTERPOLATE_EXT, 0}},
	{GL_COMBINE_ALPHA_EXT, {GL_INTERPOLATE_EXT, 0}},
	{GL_SOURCE0_RGB_EXT, {GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE1_RGB_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_SOURCE2_RGB_EXT, {GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE0_ALPHA_EXT, {GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE1_ALPHA_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_SOURCE2_ALPHA_EXT, {GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_OPERAND0_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND2_RGB_EXT, {GL_SRC_ALPHA, 0}},
	{GL_OPERAND0_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND2_ALPHA_EXT, {GL_SRC_ALPHA, 0}},
	{GL_RGB_SCALE_EXT, {1, 4, 0}},
	{GL_ALPHA_SCALE, {1, 2, 0}},
	{0, {0, 0, 0, 0, 0}}
};

static const struct test_param dot3_rgb_params[] = {
	{GL_COMBINE_RGB_EXT, {GL_DOT3_RGB_EXT, 0}},
	{GL_COMBINE_ALPHA_EXT, {GL_MODULATE, 0}},
	{GL_SOURCE0_RGB_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE1_RGB_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_SOURCE0_ALPHA_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE1_ALPHA_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_OPERAND0_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND0_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_RGB_SCALE_EXT, {1, 2, 4, 0}},
	{GL_ALPHA_SCALE, {1, 2, 4, 0}},
	{0, {0, 0, 0, 0, 0}}
};

static const struct test_param dot3_rgba_params[] = {
	{GL_COMBINE_RGB_EXT, {GL_DOT3_RGBA_EXT, 0}},
	{GL_COMBINE_ALPHA_EXT, {GL_MODULATE, 0}},
	{GL_SOURCE0_RGB_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE1_RGB_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_SOURCE0_ALPHA_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE1_ALPHA_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_OPERAND0_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND0_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_RGB_SCALE_EXT, {1, 2, 4, 0}},
	{GL_ALPHA_SCALE, {1, 2, 4, 0}},
	{0, {0, 0, 0, 0, 0}}
};

static const struct test_param modulate_add_params[] = {
	{GL_COMBINE_RGB_EXT, {GL_MODULATE_ADD_ATI, 0}},
	{GL_COMBINE_ALPHA_EXT, {GL_MODULATE_ADD_ATI, 0}},
	{GL_SOURCE0_RGB_EXT, {GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE1_RGB_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_SOURCE2_RGB_EXT, {GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE0_ALPHA_EXT, {GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE1_ALPHA_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_SOURCE2_ALPHA_EXT, {GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_OPERAND0_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND2_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND0_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND2_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_RGB_SCALE_EXT, {1, 4, 0}},
	{GL_ALPHA_SCALE, {1, 2, 0}},
	{0, {0, 0, 0, 0, 0}}
};

static const struct test_param modulate_signed_add_params[] = {
	{GL_COMBINE_RGB_EXT, {GL_MODULATE_SIGNED_ADD_ATI, 0}},
	{GL_COMBINE_ALPHA_EXT, {GL_MODULATE_SIGNED_ADD_ATI, 0}},
	{GL_SOURCE0_RGB_EXT, {GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE1_RGB_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_SOURCE2_RGB_EXT, {GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE0_ALPHA_EXT, {GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE1_ALPHA_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_SOURCE2_ALPHA_EXT, {GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_OPERAND0_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND2_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND0_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND2_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_RGB_SCALE_EXT, {1, 4, 0}},
	{GL_ALPHA_SCALE, {1, 2, 0}},
	{0, {0, 0, 0, 0, 0}}
};

static const struct test_param modulate_subtract_params[] = {
	{GL_COMBINE_RGB_EXT, {GL_MODULATE_SUBTRACT_ATI, 0}},
	{GL_COMBINE_ALPHA_EXT, {GL_MODULATE_SUBTRACT_ATI, 0}},
	{GL_SOURCE0_RGB_EXT, {GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE1_RGB_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_SOURCE2_RGB_EXT, {GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE0_ALPHA_EXT, {GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_SOURCE1_ALPHA_EXT, {GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0}},
	{GL_SOURCE2_ALPHA_EXT, {GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0}},
	{GL_OPERAND0_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND2_RGB_EXT, {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND0_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND1_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_OPERAND2_ALPHA_EXT, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0}},
	{GL_RGB_SCALE_EXT, {1, 4, 0}},
	{GL_ALPHA_SCALE, {1, 2, 0}},
	{0, {0, 0, 0, 0, 0}}
};

static void
problem(const char *s)
{
	fprintf(stderr, "Problem in combine(): %s\n", s);
	assert(0);
}

/* Set machine parameters to default values. */
static void
reset_machine(void)
{
	for (int u = 0; u < MAX_TEX_UNITS; u++) {
		machine.COMBINE_RGB[u] = GL_MODULATE;
		machine.COMBINE_ALPHA[u] = GL_MODULATE;
		machine.SOURCE0_RGB[u] = GL_TEXTURE;
		machine.SOURCE1_RGB[u] = GL_PREVIOUS_EXT;
		machine.SOURCE2_RGB[u] = GL_CONSTANT_EXT;
		machine.SOURCE0_ALPHA[u] = GL_TEXTURE;
		machine.SOURCE1_ALPHA[u] = GL_PREVIOUS_EXT;
		machine.SOURCE2_ALPHA[u] = GL_CONSTANT_EXT;
		machine.OPERAND0_RGB[u] = GL_SRC_COLOR;
		machine.OPERAND1_RGB[u] = GL_SRC_COLOR;
		machine.OPERAND2_RGB[u] = GL_SRC_ALPHA;
		machine.OPERAND0_ALPHA[u] = GL_SRC_ALPHA;
		machine.OPERAND1_ALPHA[u] = GL_SRC_ALPHA;
		machine.OPERAND2_ALPHA[u] = GL_SRC_ALPHA;
		machine.RGB_SCALE[u] = 1.0;
		machine.ALPHA_SCALE[u] = 1.0;
		machine.tex_format[u] = GL_RGBA;
	}
}

/* This computes the expected texcombine result for one texture unit. */
static void
compute_tex_combine(int tex_unit, const float prev_color[4], float result[4])
{
	float term0[4], term1[4], term2[4], dot;
	const float *color_src0, *color_src1, *color_src2;
	const float *alpha_src0, *alpha_src1 = NULL, *alpha_src2 = NULL;
	const float *frag_color = machine.frag_color;
	const float *const_color = machine.env_color[tex_unit];
	const float *tex_color = machine.tex_color[tex_unit];
	int src_unit;

	switch (machine.SOURCE0_RGB[tex_unit]) {
	case GL_PRIMARY_COLOR_EXT:
		color_src0 = frag_color;
		break;
	case GL_TEXTURE:
		color_src0 = tex_color;
		break;
	case GL_CONSTANT_EXT:
		color_src0 = const_color;
		break;
	case GL_PREVIOUS_EXT:
		color_src0 = prev_color;
		break;
	case GL_TEXTURE0:
	case GL_TEXTURE1:
	case GL_TEXTURE2:
	case GL_TEXTURE3:
	case GL_TEXTURE4:
	case GL_TEXTURE5:
	case GL_TEXTURE6:
	case GL_TEXTURE7:
		/* GL_ARB_texture_env_crossbar */
		src_unit = machine.SOURCE0_RGB[tex_unit] - GL_TEXTURE0;
		color_src0 = machine.tex_color[src_unit];
		break;
	default:
		problem("bad rgbSource0");
		return;
	}

	switch (machine.SOURCE0_ALPHA[tex_unit]) {
	case GL_PRIMARY_COLOR_EXT:
		alpha_src0 = frag_color;
		break;
	case GL_TEXTURE:
		alpha_src0 = tex_color;
		break;
	case GL_CONSTANT_EXT:
		alpha_src0 = const_color;
		break;
	case GL_PREVIOUS_EXT:
		alpha_src0 = prev_color;
		break;
	case GL_TEXTURE0:
	case GL_TEXTURE1:
	case GL_TEXTURE2:
	case GL_TEXTURE3:
	case GL_TEXTURE4:
	case GL_TEXTURE5:
	case GL_TEXTURE6:
	case GL_TEXTURE7:
		/* GL_ARB_texture_env_crossbar */
		src_unit = machine.SOURCE0_ALPHA[tex_unit] - GL_TEXTURE0;
		alpha_src0 = machine.tex_color[src_unit];
		break;
	default:
		problem("bad alphaSource0");
		return;
	}

	switch (machine.SOURCE1_RGB[tex_unit]) {
	case GL_PRIMARY_COLOR_EXT:
		color_src1 = frag_color;
		break;
	case GL_TEXTURE:
		color_src1 = tex_color;
		break;
	case GL_CONSTANT_EXT:
		color_src1 = const_color;
		break;
	case GL_PREVIOUS_EXT:
		color_src1 = prev_color;
		break;
	case GL_TEXTURE0:
	case GL_TEXTURE1:
	case GL_TEXTURE2:
	case GL_TEXTURE3:
	case GL_TEXTURE4:
	case GL_TEXTURE5:
	case GL_TEXTURE6:
	case GL_TEXTURE7:
		/* GL_ARB_texture_env_crossbar */
		src_unit = machine.SOURCE1_RGB[tex_unit] - GL_TEXTURE0;
		color_src1 = machine.tex_color[src_unit];
		break;
	default:
		problem("bad rgbSource1");
		return;
	}

	switch (machine.SOURCE1_ALPHA[tex_unit]) {
	case GL_PRIMARY_COLOR_EXT:
		alpha_src1 = frag_color;
		break;
	case GL_TEXTURE:
		alpha_src1 = tex_color;
		break;
	case GL_CONSTANT_EXT:
		alpha_src1 = const_color;
		break;
	case GL_PREVIOUS_EXT:
		alpha_src1 = prev_color;
		break;
	case GL_TEXTURE0:
	case GL_TEXTURE1:
	case GL_TEXTURE2:
	case GL_TEXTURE3:
	case GL_TEXTURE4:
	case GL_TEXTURE5:
	case GL_TEXTURE6:
	case GL_TEXTURE7:
		/* GL_ARB_texture_env_crossbar */
		src_unit = machine.SOURCE1_ALPHA[tex_unit] - GL_TEXTURE0;
		alpha_src1 = machine.tex_color[src_unit];
		break;
	default:
		problem("bad alphaSource1");
		return;
	}

	switch (machine.SOURCE2_RGB[tex_unit]) {
	case GL_PRIMARY_COLOR_EXT:
		color_src2 = frag_color;
		break;
	case GL_TEXTURE:
		color_src2 = tex_color;
		break;
	case GL_CONSTANT_EXT:
		color_src2 = const_color;
		break;
	case GL_PREVIOUS_EXT:
		color_src2 = prev_color;
		break;
	case GL_TEXTURE0:
	case GL_TEXTURE1:
	case GL_TEXTURE2:
	case GL_TEXTURE3:
	case GL_TEXTURE4:
	case GL_TEXTURE5:
	case GL_TEXTURE6:
	case GL_TEXTURE7:
		/* GL_ARB_texture_env_crossbar */
		src_unit = machine.SOURCE2_RGB[tex_unit] - GL_TEXTURE0;
		color_src2 = machine.tex_color[src_unit];
		break;
	default:
		problem("bad rgbSource2");
		return;
	}

	switch (machine.SOURCE2_ALPHA[tex_unit]) {
	case GL_PRIMARY_COLOR_EXT:
		alpha_src2 = frag_color;
		break;
	case GL_TEXTURE:
		alpha_src2 = tex_color;
		break;
	case GL_CONSTANT_EXT:
		alpha_src2 = const_color;
		break;
	case GL_PREVIOUS_EXT:
		alpha_src2 = prev_color;
		break;
	case GL_TEXTURE0:
	case GL_TEXTURE1:
	case GL_TEXTURE2:
	case GL_TEXTURE3:
	case GL_TEXTURE4:
	case GL_TEXTURE5:
	case GL_TEXTURE6:
	case GL_TEXTURE7:
		/* GL_ARB_texture_env_crossbar */
		src_unit = machine.SOURCE2_ALPHA[tex_unit] - GL_TEXTURE0;
		alpha_src2 = machine.tex_color[src_unit];
		break;
	default:
		problem("bad alphaSource2");
		return;
	}

	switch (machine.OPERAND0_RGB[tex_unit]) {
	case GL_SRC_COLOR:
		term0[0] = color_src0[0];
		term0[1] = color_src0[1];
		term0[2] = color_src0[2];
		break;
	case GL_ONE_MINUS_SRC_COLOR:
		term0[0] = 1.0 - color_src0[0];
		term0[1] = 1.0 - color_src0[1];
		term0[2] = 1.0 - color_src0[2];
		break;
	case GL_SRC_ALPHA:
		term0[0] = color_src0[3];
		term0[1] = color_src0[3];
		term0[2] = color_src0[3];
		break;
	case GL_ONE_MINUS_SRC_ALPHA:
		term0[0] = 1.0 - color_src0[3];
		term0[1] = 1.0 - color_src0[3];
		term0[2] = 1.0 - color_src0[3];
		break;
	default:
		problem("bad rgbOperand0");
		return;
	}

	switch (machine.OPERAND0_ALPHA[tex_unit]) {
	case GL_SRC_ALPHA:
		term0[3] = alpha_src0[3];
		break;
	case GL_ONE_MINUS_SRC_ALPHA:
		term0[3] = 1.0 - alpha_src0[3];
		break;
	default:
		problem("bad alphaOperand0");
		return;
	}

	switch (machine.OPERAND1_RGB[tex_unit]) {
	case GL_SRC_COLOR:
		term1[0] = color_src1[0];
		term1[1] = color_src1[1];
		term1[2] = color_src1[2];
		break;
	case GL_ONE_MINUS_SRC_COLOR:
		term1[0] = 1.0 - color_src1[0];
		term1[1] = 1.0 - color_src1[1];
		term1[2] = 1.0 - color_src1[2];
		break;
	case GL_SRC_ALPHA:
		term1[0] = color_src1[3];
		term1[1] = color_src1[3];
		term1[2] = color_src1[3];
		break;
	case GL_ONE_MINUS_SRC_ALPHA:
		term1[0] = 1.0 - color_src1[3];
		term1[1] = 1.0 - color_src1[3];
		term1[2] = 1.0 - color_src1[3];
		break;
	default:
		problem("bad rgbOperand1");
		return;
	}

	switch (machine.OPERAND1_ALPHA[tex_unit]) {
	case GL_SRC_ALPHA:
		term1[3] = alpha_src1[3];
		break;
	case GL_ONE_MINUS_SRC_ALPHA:
		term1[3] = 1.0 - alpha_src1[3];
		break;
	default:
		problem("bad alphaOperand1");
		return;
	}

	switch (machine.OPERAND2_RGB[tex_unit]) {
	case GL_SRC_COLOR:
		term2[0] = color_src2[0];
		term2[1] = color_src2[1];
		term2[2] = color_src2[2];
		break;
	case GL_ONE_MINUS_SRC_COLOR:
		term2[0] = 1.0 - color_src2[0];
		term2[1] = 1.0 - color_src2[1];
		term2[2] = 1.0 - color_src2[2];
		break;
	case GL_SRC_ALPHA:
		term2[0] = color_src2[3];
		term2[1] = color_src2[3];
		term2[2] = color_src2[3];
		break;
	case GL_ONE_MINUS_SRC_ALPHA:
		term2[0] = 1.0 - color_src2[3];
		term2[1] = 1.0 - color_src2[3];
		term2[2] = 1.0 - color_src2[3];
		break;
	default:
		problem("bad rgbOperand2");
		return;
	}

	switch (machine.OPERAND2_ALPHA[tex_unit]) {
	case GL_SRC_ALPHA:
		term2[3] = alpha_src2[3];
		break;
	default:
		problem("bad alphaOperand2");
		return;
	}

	/* Final combine */
	switch (machine.COMBINE_RGB[tex_unit]) {
	case GL_REPLACE:
		result[0] = term0[0];
		result[1] = term0[1];
		result[2] = term0[2];
		break;
	case GL_MODULATE:
		result[0] = term0[0] * term1[0];
		result[1] = term0[1] * term1[1];
		result[2] = term0[2] * term1[2];
		break;
	case GL_ADD:
		result[0] = term0[0] + term1[0];
		result[1] = term0[1] + term1[1];
		result[2] = term0[2] + term1[2];
		break;
	case GL_ADD_SIGNED_EXT:
		result[0] = term0[0] + term1[0] - 0.5;
		result[1] = term0[1] + term1[1] - 0.5;
		result[2] = term0[2] + term1[2] - 0.5;
		break;
	case GL_INTERPOLATE_EXT:
		result[0] = term0[0] * term2[0] + term1[0] * (1.0 - term2[0]);
		result[1] = term0[1] * term2[1] + term1[1] * (1.0 - term2[1]);
		result[2] = term0[2] * term2[2] + term1[2] * (1.0 - term2[2]);
		break;
	case GL_DOT3_RGB_EXT:
	case GL_DOT3_RGBA_EXT:
		dot = ((term0[0] - 0.5) * (term1[0] - 0.5) +
		       (term0[1] - 0.5) * (term1[1] - 0.5) +
		       (term0[2] - 0.5) * (term1[2] - 0.5));
		result[0] = dot;
		result[1] = dot;
		result[2] = dot;
		if (machine.COMBINE_RGB[tex_unit] == GL_DOT3_RGBA_EXT)
			result[3] = dot;
		break;
	case GL_MODULATE_ADD_ATI:
		result[0] = term0[0] * term2[0] + term1[0];
		result[1] = term0[1] * term2[1] + term1[1];
		result[2] = term0[2] * term2[2] + term1[2];
		break;
	case GL_MODULATE_SIGNED_ADD_ATI:
		result[0] = term0[0] * term2[0] + term1[0] - 0.5;
		result[1] = term0[1] * term2[1] + term1[1] - 0.5;
		result[2] = term0[2] * term2[2] + term1[2] - 0.5;
		break;
	case GL_MODULATE_SUBTRACT_ATI:
		result[0] = term0[0] * term2[0] - term1[0];
		result[1] = term0[1] * term2[1] - term1[1];
		result[2] = term0[2] * term2[2] - term1[2];
		break;
	default:
		problem("bad rgbCombine");
		return;
	}

	switch (machine.COMBINE_ALPHA[tex_unit]) {
	case GL_REPLACE:
		result[3] = term0[3];
		break;
	case GL_MODULATE:
		result[3] = term0[3] * term1[3];
		break;
	case GL_ADD:
		result[3] = term0[3] + term1[3];
		break;
	case GL_ADD_SIGNED_EXT:
		result[3] = term0[3] + term1[3] - 0.5;
		break;
	case GL_INTERPOLATE_EXT:
		result[3] = term0[3] * term2[3] + term1[3] * (1.0 - term2[3]);
		break;
	case GL_MODULATE_ADD_ATI:
		result[3] = term0[3] * term2[3] + term1[3];
		break;
	case GL_MODULATE_SIGNED_ADD_ATI:
		result[3] = term0[3] * term2[3] + term1[3] - 0.5;
		break;
	case GL_MODULATE_SUBTRACT_ATI:
		result[3] = term0[3] * term2[3] - term1[3];
		break;
	default:
		problem("bad alphaCombine");
		return;
	}

	if (machine.COMBINE_RGB[tex_unit] == GL_DOT3_RGBA_EXT) {
		result[3] = result[0];
	}

	/* scaling
	 * GH: Remove this crud when the ARB extension is done.  It
	 * most likely won't have this scale factor restriction. */
	switch (machine.COMBINE_RGB[tex_unit]) {
	case GL_DOT3_RGB_EXT:
	case GL_DOT3_RGBA_EXT:
		result[0] *= 4.0;
		result[1] *= 4.0;
		result[2] *= 4.0;
		break;
	default:
		result[0] *= machine.RGB_SCALE[tex_unit];
		result[1] *= machine.RGB_SCALE[tex_unit];
		result[2] *= machine.RGB_SCALE[tex_unit];
		break;
	}
	switch (machine.COMBINE_RGB[tex_unit]) {
	case GL_DOT3_RGBA_EXT:
		result[3] *= 4.0;
		break;
	default:
		result[3] *= machine.ALPHA_SCALE[tex_unit];
		break;
	}

	/* final clamping */
	result[0] = CLAMP(result[0], 0.0, 1.0);
	result[1] = CLAMP(result[1], 0.0, 1.0);
	result[2] = CLAMP(result[2], 0.0, 1.0);
	result[3] = CLAMP(result[3], 0.0, 1.0);
}

/* Set the fragment, texenv (constant), and texture colors for all the
 * machine's texture units. */
static void
setup_colors(void)
{
	static const float frag_color[4] = {0.00, 0.25, 0.50, 0.75};
	static const float env_colors[][4] = {{0.25, 0.50, 0.75, 1.00},
					      {0.50, 0.75, 1.00, 0.00},
					      {0.75, 1.00, 0.00, 0.25},
					      {1.00, 0.00, 0.25, 0.50}};
	static const float tex_colors[][8] = {
		{1.00, 0.00, 0.25, 0.50},
		{0.75, 1.00, 0.00, 0.25},
		{0.50, 0.75, 1.00, 0.00},
		{0.25, 0.50, 0.75, 1.00},
		/* extra colors that'll only be used for crossbar test */
		{0.00, 0.00, 0.00, 0.00},
		{0.25, 0.50, 0.50, 0.00},
		{0.50, 0.25, 0.75, 0.25},
		{0.75, 1.00, 0.25, 0.00}};

	COPY4(machine.frag_color, frag_color);
	glColor4fv(frag_color);

	for (int u = 0; u < num_tex_units; u++) {
		if (num_tex_units > 1)
			glActiveTexture(GL_TEXTURE0 + u);
		glBindTexture(GL_TEXTURE_2D, textures[u]);
		glEnable(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
			  GL_COMBINE_EXT);
		machine.env_color[u][0] = env_colors[u % 4][0];
		machine.env_color[u][1] = env_colors[u % 4][1];
		machine.env_color[u][2] = env_colors[u % 4][2];
		machine.env_color[u][3] = env_colors[u % 4][3];
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,
			   env_colors[u % 4]);

		const float *tex_col = tex_colors[u % 8];

		/* Setup texture color, according to texture format */
		switch (machine.tex_format[u]) {
		case GL_RGBA:
			machine.tex_color[u][0] = tex_col[0];
			machine.tex_color[u][1] = tex_col[1];
			machine.tex_color[u][2] = tex_col[2];
			machine.tex_color[u][3] = tex_col[3];
			break;
		case GL_RGB:
			machine.tex_color[u][0] = tex_col[0];
			machine.tex_color[u][1] = tex_col[1];
			machine.tex_color[u][2] = tex_col[2];
			machine.tex_color[u][3] = 1.0;
			break;
		case GL_ALPHA:
			machine.tex_color[u][0] = 0.0;
			machine.tex_color[u][1] = 0.0;
			machine.tex_color[u][2] = 0.0;
			machine.tex_color[u][3] = tex_col[3];
			break;
		case GL_LUMINANCE:
			machine.tex_color[u][0] = tex_col[0];
			machine.tex_color[u][1] = tex_col[0];
			machine.tex_color[u][2] = tex_col[0];
			machine.tex_color[u][3] = 1.0;
			break;
		case GL_LUMINANCE_ALPHA:
			machine.tex_color[u][0] = tex_col[0];
			machine.tex_color[u][1] = tex_col[0];
			machine.tex_color[u][2] = tex_col[0];
			machine.tex_color[u][3] = tex_col[3];
			break;
		case GL_INTENSITY:
			machine.tex_color[u][0] = tex_col[0];
			machine.tex_color[u][1] = tex_col[0];
			machine.tex_color[u][2] = tex_col[0];
			machine.tex_color[u][3] = tex_col[0];
			break;
		default:
			problem("bad texture format");
			return;
		}

		/* Make a 4x4 solid color texture */
		float image[16][4];
		for (int i = 0; i < 16; i++) {
			image[i][0] = tex_colors[u % 8][0];
			image[i][1] = tex_colors[u % 8][1];
			image[i][2] = tex_colors[u % 8][2];
			image[i][3] = tex_colors[u % 8][3];
		}
		glTexImage2D(GL_TEXTURE_2D, 0, machine.tex_format[u], 4, 4, 0,
			     GL_RGBA, GL_FLOAT, image);
	}
}

/* Examine a set of test params and compute the number of possible
 * state combinations. */
static int
count_test_combinations(const struct test_param test_params[])
{
	int num_tests = 1;
	for (int t = 0; test_params[t].target; t++) {
		int values = 0;
		for (int val = 0; test_params[t].valid_values[val]; val++) {
			values++;
		}
		num_tests *= values;
	}
	return num_tests / test_stride;
}

/* Setup the actual GL state and our internal simulated GL state. */
static void
tex_env(int tex_unit, GLenum target, GLenum value)
{
	if (num_tex_units > 1)
		glActiveTexture(GL_TEXTURE0 + tex_unit);

	glTexEnvi(GL_TEXTURE_ENV, target, value);
	piglit_check_gl_error(GL_NO_ERROR);

	switch (target) {
	case GL_COMBINE_RGB_EXT:
		machine.COMBINE_RGB[tex_unit] = value;
		break;
	case GL_COMBINE_ALPHA_EXT:
		machine.COMBINE_ALPHA[tex_unit] = value;
		break;
	case GL_SOURCE0_RGB_EXT:
		machine.SOURCE0_RGB[tex_unit] = value;
		break;
	case GL_SOURCE1_RGB_EXT:
		machine.SOURCE1_RGB[tex_unit] = value;
		break;
	case GL_SOURCE2_RGB_EXT:
		machine.SOURCE2_RGB[tex_unit] = value;
		break;
	case GL_SOURCE0_ALPHA_EXT:
		machine.SOURCE0_ALPHA[tex_unit] = value;
		break;
	case GL_SOURCE1_ALPHA_EXT:
		machine.SOURCE1_ALPHA[tex_unit] = value;
		break;
	case GL_SOURCE2_ALPHA_EXT:
		machine.SOURCE2_ALPHA[tex_unit] = value;
		break;
	case GL_OPERAND0_RGB_EXT:
		machine.OPERAND0_RGB[tex_unit] = value;
		break;
	case GL_OPERAND1_RGB_EXT:
		machine.OPERAND1_RGB[tex_unit] = value;
		break;
	case GL_OPERAND2_RGB_EXT:
		machine.OPERAND2_RGB[tex_unit] = value;
		break;
	case GL_OPERAND0_ALPHA_EXT:
		machine.OPERAND0_ALPHA[tex_unit] = value;
		break;
	case GL_OPERAND1_ALPHA_EXT:
		machine.OPERAND1_ALPHA[tex_unit] = value;
		break;
	case GL_OPERAND2_ALPHA_EXT:
		machine.OPERAND2_ALPHA[tex_unit] = value;
		break;
	case GL_RGB_SCALE_EXT:
		machine.RGB_SCALE[tex_unit] = value;
		break;
	case GL_ALPHA_SCALE:
		machine.ALPHA_SCALE[tex_unit] = value;
		break;
	}
}

/* Make the glTexEnv calls to setup one particular set of test parameters
 * from <test_params>.
 * <test_num> must be between 0 and count_test_combinations(test_params)-1. */
static void
setup_test_env(int test_num, const struct test_param test_params[])
{
	int divisor = 1;
	for (int t = 0; test_params[t].target; t++) {
		int num_values = 0;
		for (int val = 0; test_params[t].valid_values[val]; val++) {
			num_values++;
		}
		assert(num_values > 0);
		int v = (test_num / divisor) % num_values;
		GLenum target = test_params[t].target;
		GLenum value = test_params[t].valid_values[v];
		tex_env(0, target, value);
		divisor *= num_values;
	}
}

static void
print_test_env(int test_num, const struct test_param test_params[])
{
	int divisor = 1;
	for (int t = 0; test_params[t].target; t++) {
		int num_values = 0;
		for (int val = 0; test_params[t].valid_values[val]; val++) {
			num_values++;
		}
		assert(num_values > 0);
		int v = (test_num / divisor) % num_values;
		GLenum target = test_params[t].target;
		GLenum value = test_params[t].valid_values[v];
		printf("%s %s\n", piglit_get_gl_enum_name(target),
		       piglit_get_gl_enum_name(value));
		divisor *= num_values;
	}
	printf("\n");
}

#define RUN_SINGLE_TEXTURE_TEST(test) run_single_texture_test(test, #test)

/* Test texenv-combine with a single texture unit. */
static bool
run_single_texture_test(const struct test_param test_params[],
			const char *test_name)
{
	assert(num_tex_units == 1);
	setup_colors();

	const int num_tests = count_test_combinations(test_params);

	for (int test = 0; test < num_tests; test += test_stride) {
		/* 0. Setup state */
		reset_machine();
		setup_test_env(test, test_params);

		/* 1. Render with OpenGL */
		glTexCoord2f(0, 0); /* use texcoord (0,0) for all vertices */
		piglit_draw_rect(-1, -1, 2, 2);

		/* 2. Compute expected result */
		float expected[4];
		expected[3] = -1.0f;
		compute_tex_combine(0, machine.frag_color, expected);

		/* 3. Compare rendered result to expected result */
		if (!piglit_probe_pixel_rgba(0, 0, expected)) {
			printf("Single Texture Test %s %d\n", test_name,
			       test);
			print_test_env(test, test_params);
			return false;
		}
	}
	return true;
}

/* For each texture unit, test each texenv-combine mode.
 * That's 5 ^ num_tex_units combinations.
 * Or 7 ^ num_tex_units if DOT3 combine mode is supported */
static int
count_multi_texture_test_combinations()
{
	int num_tests = 1;
	for (int i = 0; i < num_tex_units; i++)
		num_tests *= (have_dot3 ? 7 : 5);

	return num_tests / test_stride;
}

/* Test texenv-combine with multiple texture units. */
static bool
run_multi_texture_test(void)
{
	static const GLenum combine_modes[10] = {
		GL_REPLACE,
		GL_ADD,
		GL_ADD_SIGNED_EXT,
		GL_MODULATE,
		GL_INTERPOLATE_EXT,
		GL_DOT3_RGB_EXT,
		GL_DOT3_RGBA_EXT,
		GL_MODULATE_ADD_ATI,
		GL_MODULATE_SIGNED_ADD_ATI,
		GL_MODULATE_SUBTRACT_ATI
	};
	const int num_modes = have_dot3 ? (have_combine3 ? 10 : 7) : 5;

	/* four texture units is enough to test */
	if (num_tex_units > 4)
		num_tex_units = 4;

	const int num_tests = count_multi_texture_test_combinations();

	setup_colors();
	for (int test_num = 0; test_num < num_tests;
	     test_num += test_stride) {
		/* 0. Set up texture units */
		reset_machine();
		int divisor = 1;
		for (int u = 0; u < num_tex_units; u++) {
			const int m = (test_num / divisor) % num_modes;
			const GLenum mode = combine_modes[m];

			/* Set GL_COMBINE_RGB_EXT and GL_COMBINE_ALPHA_EXT */
			tex_env(u, GL_COMBINE_RGB_EXT, mode);
			tex_env(u, GL_COMBINE_ALPHA_EXT,
				(mode == GL_DOT3_RGB_EXT ||
				 mode == GL_DOT3_RGBA_EXT)
					? GL_REPLACE
					: mode);
			tex_env(u, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
			tex_env(u, GL_SOURCE1_RGB_EXT, GL_PREVIOUS_EXT);
			tex_env(u, GL_SOURCE2_RGB_EXT, GL_TEXTURE);
			tex_env(u, GL_SOURCE0_ALPHA_EXT, GL_PREVIOUS_EXT);
			tex_env(u, GL_SOURCE1_ALPHA_EXT, GL_PREVIOUS_EXT);
			tex_env(u, GL_SOURCE2_ALPHA_EXT, GL_TEXTURE);
			tex_env(u, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
			tex_env(u, GL_OPERAND1_RGB_EXT,
				GL_ONE_MINUS_SRC_COLOR);
			tex_env(u, GL_OPERAND2_RGB_EXT, GL_SRC_ALPHA);
			tex_env(u, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
			tex_env(u, GL_OPERAND1_ALPHA_EXT,
				GL_ONE_MINUS_SRC_ALPHA);
			tex_env(u, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA);
			tex_env(u, GL_RGB_SCALE_EXT, 1);
			tex_env(u, GL_ALPHA_SCALE, 1);

			divisor *= num_modes;
		}

		/* 1. Render with OpenGL */
		/* use texcoord (0,0) for all vertices */
		for (int u = 0; u < num_tex_units; u++)
			glMultiTexCoord2f(GL_TEXTURE0 + u, 0, 0);
		piglit_draw_rect(-1, -1, 2, 2);

		/* 2. Compute expected result */
		float prev_color[4];
		float expected[4] = {0};
		COPY4(prev_color, machine.frag_color);
		for (int u = 0; u < num_tex_units; u++) {
			compute_tex_combine(u, prev_color, expected);
			COPY4(prev_color, expected);
		}

		/* 3. Compare rendered result to expected result */
		if (!piglit_probe_pixel_rgba(0, 0, expected)) {
			printf("Multi-texture test %d\n", test_num);
			return false;
		}
	}
	return true;
}

/* We do a really short, simple test for GL_ARB_texture_env_crossbar since the
 * preceeding tests are pretty comprehensive and the crossbar feature is just
 * an incremental addition.  Basically, if we have N texture units we run N
 * tests.  For test [i] we set texture unit [i] to fetch the texture color
 * from unit [num_units - i - 1].  For units != i we use the constant color
 * (0,0,0,0).  We use GL_ADD mode to compute the sum over all units.  So
 * effectively, the result of texture combine is simply the incoming fragment
 * color plus unit [num_units - test - 1]'s texture color. */
static bool
run_crossbar_test()
{
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &num_tex_units);

	/* Set up constant texture state for all tests */
	setup_colors();
	reset_machine();
	for (int unit = 0; unit < num_tex_units; unit++) {
		tex_env(unit, GL_COMBINE_RGB_EXT, GL_ADD);
		tex_env(unit, GL_COMBINE_ALPHA_EXT, GL_ADD);
		tex_env(unit, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
		tex_env(unit, GL_SOURCE0_ALPHA_EXT, GL_PREVIOUS_EXT);
		/* SOURCE1_RGB/ALPHA is set below, per test */
		tex_env(unit, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
		tex_env(unit, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
		tex_env(unit, GL_OPERAND2_RGB_EXT, GL_SRC_ALPHA);
		tex_env(unit, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
		tex_env(unit, GL_OPERAND1_ALPHA_EXT, GL_SRC_ALPHA);
		tex_env(unit, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA);
		tex_env(unit, GL_RGB_SCALE_EXT, 1);
		tex_env(unit, GL_ALPHA_SCALE, 1);

		machine.env_color[unit][0] = 0.0;
		machine.env_color[unit][1] = 0.0;
		machine.env_color[unit][2] = 0.0;
		machine.env_color[unit][3] = 0.0;
		glActiveTexture(GL_TEXTURE0 + unit);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,
			   machine.env_color[unit]);
	}

	for (int test = 0; test < num_tex_units; test++) {
		/* 1. Set up texture state */
		for (int unit = 0; unit < num_tex_units; unit++) {
			if (unit == test) {
				const int revUnit = num_tex_units - unit - 1;
				tex_env(unit, GL_SOURCE1_RGB_EXT,
					GL_TEXTURE0 + revUnit);
				tex_env(unit, GL_SOURCE1_ALPHA_EXT,
					GL_TEXTURE0 + revUnit);
			} else {
				tex_env(unit, GL_SOURCE1_RGB_EXT,
					GL_CONSTANT_EXT);
				tex_env(unit, GL_SOURCE1_ALPHA_EXT,
					GL_CONSTANT_EXT);
			}
		}

		/* 2. Render with OpenGL */
		/* texcoord (0,) for all vertices is OK */
		for (int unit = 0; unit < num_tex_units; unit++)
			glMultiTexCoord2f(GL_TEXTURE0 + unit, 0, 0);
		piglit_draw_rect(-1, -1, 2, 2);

		/* 3. Compute expected result */
		float prev_color[4];
		float expected[4];
		COPY4(prev_color, machine.frag_color);
		for (int unit = 0; unit < num_tex_units; unit++) {
			compute_tex_combine(unit, prev_color, expected);
			COPY4(prev_color, expected);
		}

		/* 4. Compare rendered result to expected result */
		if (!piglit_probe_pixel_rgba(0, 0, expected)) {
			printf("Texture crossbar test %d\n", test);
			return false;
		}
	}
	return true;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	float old_tolerance[4];

	/* Do single texture unit tests first. */
	if (pass)
		pass = RUN_SINGLE_TEXTURE_TEST(replace_params);
	if (pass)
		pass = RUN_SINGLE_TEXTURE_TEST(add_params);
	if (pass)
		pass = RUN_SINGLE_TEXTURE_TEST(add_signed_params);
	if (pass)
		pass = RUN_SINGLE_TEXTURE_TEST(modulate_params);
	if (pass)
		pass = RUN_SINGLE_TEXTURE_TEST(interpolate_params);
	/* Some implementations have precision problems with the dot3
	 * instruction. */
	for (int i = 0; i < 4; ++i) {
		old_tolerance[i] = piglit_tolerance[i];
		piglit_tolerance[i] = MAX2(0.02, piglit_tolerance[i]);
	}
	if (pass && have_dot3)
		pass = RUN_SINGLE_TEXTURE_TEST(dot3_rgb_params);
	if (pass && have_dot3)
		pass = RUN_SINGLE_TEXTURE_TEST(dot3_rgba_params);
	for (int i = 0; i < 4; ++i)
		piglit_tolerance[i] = old_tolerance[i];
	if (pass && have_combine3)
		pass = RUN_SINGLE_TEXTURE_TEST(modulate_add_params);
	if (pass && have_combine3)
		pass = RUN_SINGLE_TEXTURE_TEST(modulate_signed_add_params);
	if (pass && have_combine3)
		pass = RUN_SINGLE_TEXTURE_TEST(modulate_subtract_params);

	/* Now do some multi-texture tests */
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &num_tex_units);
	if (pass && num_tex_units > 1) {
		pass = run_multi_texture_test();
	}

	/* Do crossbar tests */
	if (pass && have_crossbar) {
		pass = run_crossbar_test();
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_texture_env_combine");

	/* Test the availability of the DOT3 extenstion */
	have_dot3 = piglit_is_extension_supported("GL_EXT_texture_env_dot3");

	have_crossbar =
		piglit_is_extension_supported("GL_ARB_texture_env_crossbar");

	have_combine3 =
		piglit_is_extension_supported("GL_ATI_texture_env_combine3");

	/* Allocate our textures */
	glGenTextures(MAX_TEX_UNITS, textures);

	reset_machine();
	num_tex_units = 1;

	for (int i = 1; i < argc; i++)
		if (!strcmp(argv[i], "--quick"))
			test_stride = 67; /* a prime number */
}
