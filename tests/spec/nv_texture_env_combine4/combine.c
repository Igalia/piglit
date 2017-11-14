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
 * @file combine.c:  Test GL_NV_texture_env_combine4
 * Author: Brian Paul (brianp@valinux.com)  Januar 2009
 *
 * Generate some random combiner state and colors, compute the expected
 * color, then render with the combiner state and compare the results.
 * Only one texture unit is tested and not all possible combiner terms
 * are exercised.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#ifdef _WIN32
#define SRAND(x) srand(x)
#define DRAND() ((float)rand() / RAND_MAX)
#else
#define SRAND(x) srand48(x)
#define DRAND() drand48()
#endif

#define NUM_TESTS 200

struct combine_state {
	GLenum combine_mode;
	GLenum source[4];
	GLenum operand_rgb[4];
	GLenum operand_a[4];
	float primary_color[4];
	float constant_color[4];
	float texture_color[4];
};

/* generate random combiner state */
static void
generate_state(struct combine_state *state)
{
	if (DRAND() > 0.5f)
		state->combine_mode = GL_ADD;
	else
		state->combine_mode = GL_ADD_SIGNED_EXT;

	for (int i = 0; i < 4; i++) {
		int src = (int)(DRAND() * 4.0);
		switch (src) {
		case 0:
			state->source[i] = GL_ZERO;
			break;
		case 1:
			state->source[i] = GL_TEXTURE;
			break;
		case 2:
			state->source[i] = GL_CONSTANT_EXT;
			break;
		default:
			state->source[i] = GL_PRIMARY_COLOR_EXT;
			break;
		}

		if (DRAND() > 0.5f) {
			state->operand_rgb[i] = GL_SRC_COLOR;
			state->operand_a[i] = GL_SRC_ALPHA;
		} else {
			state->operand_rgb[i] = GL_ONE_MINUS_SRC_COLOR;
			state->operand_a[i] = GL_ONE_MINUS_SRC_ALPHA;
		}
	}

	state->primary_color[0] = DRAND();
	state->primary_color[1] = DRAND();
	state->primary_color[2] = DRAND();
	state->primary_color[3] = DRAND();

	state->constant_color[0] = DRAND();
	state->constant_color[1] = DRAND();
	state->constant_color[2] = DRAND();
	state->constant_color[3] = DRAND();

	state->texture_color[0] = DRAND();
	state->texture_color[1] = DRAND();
	state->texture_color[2] = DRAND();
	state->texture_color[3] = DRAND();
}

/* compute expected final color */
static void
evaluate_state(const struct combine_state state, float result[4])
{
	float arg[4][4];

	/* setup terms */
	for (int i = 0; i < 4; i++) {
		switch (state.source[i]) {
		case GL_ZERO:
			arg[i][0] = arg[i][1] = arg[i][2] = arg[i][3] = 0.0f;
			break;
		case GL_PRIMARY_COLOR_EXT:
			arg[i][0] = state.primary_color[0];
			arg[i][1] = state.primary_color[1];
			arg[i][2] = state.primary_color[2];
			arg[i][3] = state.primary_color[3];
			break;
		case GL_CONSTANT_EXT:
			arg[i][0] = state.constant_color[0];
			arg[i][1] = state.constant_color[1];
			arg[i][2] = state.constant_color[2];
			arg[i][3] = state.constant_color[3];
			break;
		case GL_TEXTURE:
			arg[i][0] = state.texture_color[0];
			arg[i][1] = state.texture_color[1];
			arg[i][2] = state.texture_color[2];
			arg[i][3] = state.texture_color[3];
			break;
		default:
			assert(0);
		}

		switch (state.operand_rgb[i]) {
		case GL_SRC_COLOR:
			/* nop */
			break;
		case GL_ONE_MINUS_SRC_COLOR:
			arg[i][0] = 1.0f - arg[i][0];
			arg[i][1] = 1.0f - arg[i][1];
			arg[i][2] = 1.0f - arg[i][2];
			arg[i][3] = 1.0f - arg[i][3];
			break;
		default:
			assert(0);
		}
	}

	/* combine terms, loop over color channels */
	for (int i = 0; i < 4; i++) {
		result[i] = arg[0][i] * arg[1][i] + arg[2][i] * arg[3][i];
		if (state.combine_mode == GL_ADD_SIGNED_EXT)
			result[i] -= 0.5f;
		if (result[i] < 0.0f)
			result[i] = 0.0f;
		else if (result[i] > 1.0f)
			result[i] = 1.0f;
	}
}

/* render quad with given combiner state and compare resulting color
 * return false if GL error is detected or comparison fails, true otherwise.
 */
static bool
render_state(const struct combine_state state, const float expected[4])
{
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);

	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, state.combine_mode);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, state.combine_mode);

	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, state.source[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, state.source[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, state.source[1]);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, state.source[1]);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, state.source[2]);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, state.source[2]);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, state.source[3]);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_NV, state.source[3]);

	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, state.operand_rgb[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, state.operand_a[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, state.operand_rgb[1]);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, state.operand_a[1]);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, state.operand_rgb[2]);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, state.operand_a[2]);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, state.operand_rgb[3]);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_NV, state.operand_a[3]);

	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,
		   state.constant_color);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT,
		     state.texture_color);

	glColor4fv(state.primary_color);

	glClear(GL_COLOR_BUFFER_BIT);
	piglit_draw_rect_tex(-1, -1, 2, 2, 0, 0, 1, 1);

	return piglit_probe_pixel_rgb(piglit_width / 2, piglit_height / 2,
				      expected);
}

enum piglit_result
piglit_display(void)
{
	for (int i = 0; i < NUM_TESTS; i++) {
		struct combine_state state;
		float expected[4];

		generate_state(&state);

		evaluate_state(state, expected);

		if (!render_state(state, expected))
			return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_NV_texture_env_combine4");
	SRAND(42); /* init random number generator */
}
