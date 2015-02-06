/*  BEGIN_COPYRIGHT -*- glean -*-
 * 
 *  Copyright (C) 1999  Allen Akin   All Rights Reserved.
 *  Copyright (C) 2014  Intel Corporation.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 *  END_COPYRIGHT
 */

/** @file blend.c  
 * 
 *  Test blending functions.
 *  
 *	This test checks all combinations of source and destination
 *	blend factors for the GL_FUNC_ADD blend equation.  It operates
 *	on all RGB or RGBA drawing surface configurations that support
 *	the creation of windows.
 *
 *	Note that a common cause of failures for this test is small errors
 *	introduced when an implementation scales color values incorrectly;
 *	for example, converting an 8-bit color value to float by
 *	dividing by 256 rather than 255, or computing a blending result
 *	by shifting a double-width intermediate value rather than scaling
 *	it.  Also, please note that the OpenGL spec requires that when
 *	converting from floating-point colors to integer form, the result
 *	must be rounded to the nearest integer, not truncated.
 *	[1.2.1, 2.13.9]
 *	
 *	The test reports two error measurements.  The first (readback) is
 *	the error detected when reading back raw values that were written
 *	to the framebuffer.  The error in this case should be very close
 *	to zero, since the values are carefully constructed so that they
 *	can be represented accurately in the framebuffer.  The second
 *	(blending) is the error detected in the result of the blending
 *	computation.  For the test to pass, these errors must both be
 *	no greater than one least-significant bit in the framebuffer
 *	representation of a color.
 */

#include "piglit-util-gl.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define HUGE_STEP 1000

/*
 * We will check each pair of blend factors
 * for each pixel in a square image of this
 * dimension, so if you make it too large,
 * the tests may take quite a while to run.
 */
#define drawing_size 32
#define img_width drawing_size
#define img_height drawing_size

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | 
		PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const GLenum src_factors[] = {
	GL_ZERO,
	GL_ONE,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,
	GL_SRC_ALPHA_SATURATE,
	GL_CONSTANT_COLOR,
	GL_ONE_MINUS_CONSTANT_COLOR,
	GL_CONSTANT_ALPHA,
	GL_ONE_MINUS_CONSTANT_ALPHA
};
static const GLenum dst_factors[] = {
	GL_ZERO,
	GL_ONE,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,
	GL_CONSTANT_COLOR,
	GL_ONE_MINUS_CONSTANT_COLOR,
	GL_CONSTANT_ALPHA,
	GL_ONE_MINUS_CONSTANT_ALPHA
};
static const GLenum operators[] = {
	GL_FUNC_ADD,
	GL_FUNC_SUBTRACT,
	GL_FUNC_REVERSE_SUBTRACT,
	GL_MIN,
	GL_MAX
};

struct image {
	GLuint name;
	GLfloat *data;
};

static struct image dst_img;      /* Image #1 */
static struct image src_img;      /* Image #2 */
static struct image exp_img;      /* The expected blending result */

static bool have_sep_func, have_blend_equation;
static bool have_blend_equation_sep, have_blend_color;

/* A bright, semi-transparent blue */
static const GLfloat constant_color[4] = {0.25, 0.0, 1.0, 0.75};

/* Our random image data factory. */
GLfloat* 
random_image_data(void) 
{
	int i;
	GLfloat *img = malloc(4*img_width*img_height*sizeof(GLfloat));
	for (i = 0; i < 4*img_width*img_height; ++i) {
		img[i] = (float) rand() / RAND_MAX;
	}
	return img;
} /* random_image_data */

/* Our solid color fill data factory. */
GLfloat* 
color_fill_data(GLfloat r, GLfloat g, GLfloat b, GLfloat a) 
{
	int i, j;
	GLfloat *img = malloc(4*img_width*img_height*sizeof(GLfloat));
	for (j = 0; j < img_height; ++j) { /* j = vertical, i = horizontal */
		for (i = 0; i < img_width; ++i) {
			img[4*(img_width*j + i) + 0] = r; 
			img[4*(img_width*j + i) + 1] = g;
			img[4*(img_width*j + i) + 2] = b;
			img[4*(img_width*j + i) + 3] = a;
		}
	}
	return img;
} /* color_fill_data */

static void
image_init(struct image *image) 
{
	glGenTextures(1, &image->name);
	glBindTexture(GL_TEXTURE_2D, image->name);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		     img_width, img_height, 0, 
		     GL_RGBA, GL_FLOAT, image->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
} /* image_init */


void
piglit_init(int argc, char **argv)
{

	int rgb_tol = 0;
	int alpha_tol = 0;
	const char* blend_rgb_tol = getenv("PIGLIT_BLEND_RGB_TOLERANCE");
	const char* blend_alpha_tol = getenv("PIGLIT_BLEND_ALPHA_TOLERANCE");

	/* 
	 * Hack: Make driver tests on incorrect hardware feasible
	 * We want to be able to perform meaningful tests
	 * even when the blend unit of a GPU simply doesn't have
	 * sufficient precision.
	 */
	if (blend_rgb_tol)
	{
		rgb_tol = atoi(blend_rgb_tol);
		printf("Note: RGB tolerance changed to %i %s.\n", rgb_tol,
			rgb_tol == 1 ? "bit" : "bits");
	}
	if (blend_alpha_tol)
	{
		alpha_tol = atoi(blend_alpha_tol);
		printf("Note: Alpha tolerance changed to %i %s.\n", alpha_tol,
			alpha_tol == 1 ? "bit" : "bits");
	}
	/* A 0 passed in yields the default tolerance of 3.0/(1 << 8) ~= 0.01. */
	piglit_set_tolerance_for_bits(rgb_tol, rgb_tol, rgb_tol, alpha_tol);

	/* Initialize random images. */
	srand(0); 

	/* Fill src and dst with randomness. */
	dst_img.data = random_image_data();
	src_img.data = random_image_data();

	/* Fill exp_img with constant_color. */
	/* 
	 * You can use this as a check to make sure the test is working
	 * properly.
	 */
	exp_img.data = color_fill_data(constant_color[0],
				       constant_color[1],
				       constant_color[2],
				       constant_color[3]);

	image_init(&dst_img);
	image_init(&src_img);
	image_init(&exp_img);

} /* piglit_init */

bool
needs_blend_color(const GLenum func) 
{
	switch (func) {
	case GL_CONSTANT_COLOR:
	case GL_ONE_MINUS_CONSTANT_COLOR:
	case GL_CONSTANT_ALPHA:
	case GL_ONE_MINUS_CONSTANT_ALPHA:
		return true;
	default:
		return false;
	}
}

/* Function that verifies GL's blending behavior. */
static void
apply_blend(GLenum src_factor_rgb, GLenum src_factor_a,
	    GLenum dst_factor_rgb, GLenum dst_factor_a,
	    GLenum op_rgb, GLenum op_a,
	    float* dst, const float* src,
	    const GLfloat constant_color[4])
{
	float sf[4] = { 0 };
	float df[4] = { 0 };

	if (op_rgb != GL_MIN && op_rgb != GL_MAX) {
		/* Src RGB term */
		switch (src_factor_rgb) {
		case GL_ZERO:
			sf[0] = sf[1] = sf[2] = 0.0;
			break;
		case GL_ONE:
			sf[0] = sf[1] = sf[2] = 1.0;
			break;
		case GL_DST_COLOR:
			sf[0] = dst[0];
			sf[1] = dst[1];
			sf[2] = dst[2];
			break;
		case GL_ONE_MINUS_DST_COLOR:
			sf[0] = 1.0 - dst[0];
			sf[1] = 1.0 - dst[1];
			sf[2] = 1.0 - dst[2];
			break;
		case GL_SRC_ALPHA:
			sf[0] = sf[1] = sf[2] = src[3];
			break;
		case GL_ONE_MINUS_SRC_ALPHA:
			sf[0] = sf[1] = sf[2] = 1.0 - src[3];
			break;
		case GL_DST_ALPHA:
			sf[0] = sf[1] = sf[2] = dst[3];
			break;
		case GL_ONE_MINUS_DST_ALPHA:
			sf[0] = sf[1] = sf[2] = 1.0 - dst[3];
			break;
		case GL_SRC_ALPHA_SATURATE: {
			float f = 1.0 - dst[3];
			if (src[3] < f)
				f = src[3];
			sf[0] = sf[1] = sf[2] = f;
			}
			break;
		case GL_CONSTANT_COLOR:
			sf[0] = constant_color[0];
			sf[1] = constant_color[1];
			sf[2] = constant_color[2];
			break;
		case GL_ONE_MINUS_CONSTANT_COLOR:
			sf[0] = 1.0 - constant_color[0];
			sf[1] = 1.0 - constant_color[1];
			sf[2] = 1.0 - constant_color[2];
			break;
		case GL_CONSTANT_ALPHA:
			sf[0] =
			sf[1] =
			sf[2] = constant_color[3];
			break;
		case GL_ONE_MINUS_CONSTANT_ALPHA:
			sf[0] =
			sf[1] =
			sf[2] = 1.0 - constant_color[3];
			break;
		default:
			sf[0] = sf[1] = sf[2] = 0.0;
			abort();
			break;
		}

		/* Dest RGB term */
		switch (dst_factor_rgb) {
		case GL_ZERO:
			df[0] = df[1] = df[2] = 0.0;
			break;
		case GL_ONE:
			df[0] = df[1] = df[2] = 1.0;
			break;
		case GL_SRC_COLOR:
			df[0] = src[0];
			df[1] = src[1];
			df[2] = src[2];
			break;
		case GL_ONE_MINUS_SRC_COLOR:
			df[0] = 1.0 - src[0];
			df[1] = 1.0 - src[1];
			df[2] = 1.0 - src[2];
			break;
		case GL_SRC_ALPHA:
			df[0] = df[1] = df[2] = src[3];
			break;
		case GL_ONE_MINUS_SRC_ALPHA:
			df[0] = df[1] = df[2] = 1.0 - src[3];
			break;
		case GL_DST_ALPHA:
			df[0] = df[1] = df[2] = dst[3];
			break;
		case GL_ONE_MINUS_DST_ALPHA:
			df[0] = df[1] = df[2] = 1.0 - dst[3];
			break;
		case GL_CONSTANT_COLOR:
			df[0] = constant_color[0];
			df[1] = constant_color[1];
			df[2] = constant_color[2];
			break;
		case GL_ONE_MINUS_CONSTANT_COLOR:
			df[0] = 1.0 - constant_color[0];
			df[1] = 1.0 - constant_color[1];
			df[2] = 1.0 - constant_color[2];
			break;
		case GL_CONSTANT_ALPHA:
			df[0] =
			df[1] =
			df[2] = constant_color[3];
			break;
		case GL_ONE_MINUS_CONSTANT_ALPHA:
			df[0] =
			df[1] =
			df[2] = 1.0 - constant_color[3];
			break;
		default:
			df[0] = df[1] = df[2] = 0.0;
			abort();
			break;
		}
	}

	if (op_a != GL_MIN && op_a != GL_MAX) {
		/* Src Alpha term */
		switch (src_factor_a) {
		case GL_ZERO:
			sf[3] = 0.0;
			break;
		case GL_ONE:
			sf[3] = 1.0;
			break;
		case GL_DST_COLOR:
			sf[3] = dst[3];
			break;
		case GL_ONE_MINUS_DST_COLOR:
			sf[3] = 1.0 - dst[3];
			break;
		case GL_SRC_ALPHA:
			sf[3] = src[3];
			break;
		case GL_ONE_MINUS_SRC_ALPHA:
			sf[3] = 1.0 - src[3];
			break;
		case GL_DST_ALPHA:
			sf[3] = dst[3];
			break;
		case GL_ONE_MINUS_DST_ALPHA:
			sf[3] = 1.0 - dst[3];
			break;
		case GL_SRC_ALPHA_SATURATE:
			sf[3] = 1.0;
			break;
		case GL_CONSTANT_COLOR:
			sf[3] = constant_color[3];
			break;
		case GL_ONE_MINUS_CONSTANT_COLOR:
			sf[3] = 1.0 - constant_color[3];
			break;
		case GL_CONSTANT_ALPHA:
			sf[3] = constant_color[3];
			break;
		case GL_ONE_MINUS_CONSTANT_ALPHA:
			sf[3] = 1.0 - constant_color[3];
			break;
		default:
			sf[3] = 0.0;
			abort();
			break;
		}

		/* Dst Alpha term */
		switch (dst_factor_a) {
		case GL_ZERO:
			df[3] = 0.0;
			break;
		case GL_ONE:
			df[3] = 1.0;
			break;
		case GL_SRC_COLOR:
			df[3] = src[3];
			break;
		case GL_ONE_MINUS_SRC_COLOR:
			df[3] = 1.0 - src[3];
			break;
		case GL_SRC_ALPHA:
			df[3] = src[3];
			break;
		case GL_ONE_MINUS_SRC_ALPHA:
			df[3] = 1.0 - src[3];
			break;
		case GL_DST_ALPHA:
			df[3] = dst[3];
			break;
		case GL_ONE_MINUS_DST_ALPHA:
			df[3] = 1.0 - dst[3];
			break;
		case GL_CONSTANT_COLOR:
			df[3] = constant_color[3];
			break;
		case GL_ONE_MINUS_CONSTANT_COLOR:
			df[3] = 1.0 - constant_color[3];
			break;
		case GL_CONSTANT_ALPHA:
			df[3] = constant_color[3];
			break;
		case GL_ONE_MINUS_CONSTANT_ALPHA:
			df[3] = 1.0 - constant_color[3];
			break;
		default:
			df[3] = 0.0;
			abort();
			break;
		}
	}

	switch (op_rgb) {
	case GL_FUNC_ADD:
		dst[0] = CLAMP(src[0] * sf[0] + dst[0] * df[0], 0.0f, 1.0f);
		dst[1] = CLAMP(src[1] * sf[1] + dst[1] * df[1], 0.0f, 1.0f);
		dst[2] = CLAMP(src[2] * sf[2] + dst[2] * df[2], 0.0f, 1.0f);
		break;
	case GL_FUNC_SUBTRACT:
		dst[0] = CLAMP(src[0] * sf[0] - dst[0] * df[0], 0.0f, 1.0f);
		dst[1] = CLAMP(src[1] * sf[1] - dst[1] * df[1], 0.0f, 1.0f);
		dst[2] = CLAMP(src[2] * sf[2] - dst[2] * df[2], 0.0f, 1.0f);
		break;
	case GL_FUNC_REVERSE_SUBTRACT:
		dst[0] = CLAMP(dst[0] * df[0] - src[0] * sf[0], 0.0f, 1.0f);
		dst[1] = CLAMP(dst[1] * df[1] - src[1] * sf[1], 0.0f, 1.0f);
		dst[2] = CLAMP(dst[2] * df[2] - src[2] * sf[2], 0.0f, 1.0f);
		break;
	case GL_MIN:
		dst[0] = MIN2(src[0], dst[0]);
		dst[1] = MIN2(src[1], dst[1]);
		dst[2] = MIN2(src[2], dst[2]);
		break;
	case GL_MAX:
		dst[0] = MAX2(src[0], dst[0]);
		dst[1] = MAX2(src[1], dst[1]);
		dst[2] = MAX2(src[2], dst[2]);
		break;
        default:
		abort();
        }

	switch (op_a) {
	case GL_FUNC_ADD:
		dst[3] = CLAMP(src[3] * sf[3] + dst[3] * df[3], 0.0f, 1.0f);
		break;
	case GL_FUNC_SUBTRACT:
		dst[3] = CLAMP(src[3] * sf[3] - dst[3] * df[3], 0.0f, 1.0f);
		break;
	case GL_FUNC_REVERSE_SUBTRACT:
		dst[3] = CLAMP(dst[3] * df[3] - src[3] * sf[3], 0.0f, 1.0f);
		break;
	case GL_MIN:
		dst[3] = MIN2(src[3], dst[3]);
		break;
	case GL_MAX:
		dst[3] = MAX2(src[3], dst[3]);
		break;
        default:
		abort();
        }

} /* apply_blend */


/* Test for one set of factor levels */
bool
run_factor_set(GLenum src_factor_rgb, GLenum src_factor_a,
	       GLenum dst_factor_rgb, GLenum dst_factor_a,
	       GLenum op_rgb, GLenum op_a,
	       const GLfloat constant_color[4])
{
	int i, j;
	bool pass = true, p;

	glDisable(GL_DITHER);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Send dst image to the framebuffer. */
	glDisable(GL_BLEND);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, dst_img.name);
	piglit_draw_rect_tex(0, 0, img_width, img_height, 0, 0, 1, 1);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/*
	 * Read back the contents of the framebuffer, and measure any
	 * difference from what was actually written.  We can't tell
	 * whether errors occurred when writing or when reading back,
	 * but at least we can report anything unusual.
	 */
	pass &= piglit_probe_image_rgba(0, 0, img_width, 
		img_height, dst_img.data);

	/*
	 * Now apply the blending
	 * operation to both the framebuffer and a copy in the image
	 * ``expected''.  Note that a fresh source alpha must be
	 * generated here, because the range of source alpha values is
	 * not limited by the range of alpha values that can be
	 * represented in the framebuffer.  Save the source pixels in
	 * the image ``src'' so we can diagnose any problems we find
	 * later.
	 */

	/* Configure the appropriate blending settings */
	if (have_sep_func)
		glBlendFuncSeparate(src_factor_rgb, dst_factor_rgb,
                                    src_factor_a, dst_factor_a);
	else
		glBlendFunc(src_factor_rgb, dst_factor_rgb);

	if (have_blend_equation_sep)
		glBlendEquationSeparate(op_rgb, op_a);
	else if (have_blend_equation)
		glBlendEquation(op_rgb);


	/* Send src to the framebuffer and let GL blend it with dst */
	glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, src_img.name);
	piglit_draw_rect_tex(0, 0, img_width, img_height, 0, 0, 1, 1);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* Compute the appropriate expected. */
	for (j = 0; j < img_height; ++j) {
		for (i = 0; i < img_width; ++i) {

			int idx = 4*(img_width*j + i);
			
			/* Initialize expected with dst data. */
			exp_img.data[idx + 0] = dst_img.data[idx + 0]; 
			exp_img.data[idx + 1] = dst_img.data[idx + 1];
			exp_img.data[idx + 2] = dst_img.data[idx + 2];
			exp_img.data[idx + 3] = dst_img.data[idx + 3];

			/* Apply the blending. */
			apply_blend(src_factor_rgb, src_factor_a,
				    dst_factor_rgb, dst_factor_a,
				    op_rgb, op_a,
				    &exp_img.data[idx], &src_img.data[idx], 
				    constant_color);
		}
	}

	/*
	 * Compare the image in the framebuffer to the
	 * computed image (``expected'') to see if any pixels are
	 * outside the expected tolerance range. 
	 */
	p = piglit_probe_image_rgba(0, 0, img_width, img_height,
		exp_img.data);
	if (!p) {
		printf("  Blend src factors: %s, %s\n",
		       piglit_get_gl_enum_name(src_factor_rgb),
		       piglit_get_gl_enum_name(src_factor_a));
		printf("  Blend dst factors: %s, %s\n",
		       piglit_get_gl_enum_name(dst_factor_rgb),
		       piglit_get_gl_enum_name(dst_factor_a));
		printf("  Blend ops: %s, %s\n",
		       piglit_get_gl_enum_name(op_rgb),
		       piglit_get_gl_enum_name(op_a));
		printf("  Blend color: %.3f, %.3f, %.3f, %.3f\n",
		       constant_color[0], constant_color[1],
		       constant_color[2], constant_color[3]);
		fflush(stdout);
	}

	pass &= p;

	return pass;
} /* run_factor_set */

bool
proc_factors(int sf, int sfa, int df, int dfa, int* counter, 
	int op, int opa)
{
	GLenum src_rgb, src_a, dst_rgb, dst_a;

	if (have_sep_func) {
		src_rgb = src_factors[sf];
		src_a = src_factors[sfa];
		dst_rgb = dst_factors[df];
		dst_a = dst_factors[dfa];
	}
	else {
		src_rgb = src_a = src_factors[sf];
		dst_rgb = dst_a = dst_factors[df];
	}

	/* skip test if blend color used, but not supported. */
	if (!have_blend_color
		&& (needs_blend_color(src_rgb) ||
			needs_blend_color(src_a) ||
			needs_blend_color(dst_rgb) ||
			needs_blend_color(dst_a)))
		return true;

	/* Increment counter so that tests are numbered starting from 1. */
	(*counter)++; 

	/* For verification purposes, this prints every test
	 * configuration as it runs.*/
	/*
	 * printf("%i: %s, %s, %s, %s, %s, %s\n", *counter,
	 * 			piglit_get_gl_enum_name(src_rgb), 
	 * 			piglit_get_gl_enum_name(src_a), 
	 * 			piglit_get_gl_enum_name(dst_rgb), 
	 * 			piglit_get_gl_enum_name(dst_a), 
	 * 			piglit_get_gl_enum_name(operators[op]), 
	 * 			piglit_get_gl_enum_name(operators[opa]));
	 */
	
	return run_factor_set(src_rgb, src_a, dst_rgb, dst_a, 
		operators[op], operators[opa], constant_color);
} /* proc_factors */

/**
 * Run the whole suite of blend tests
 * Not a full factorial test, that would take too long.
 * Tests all add blending permutations.
 * Tests about 1/3 of subtract blending.
 * Skips most max and min tests.
 */
bool
run_all_factor_sets(void)
{
	bool pass = true;
	int gl_version = piglit_get_gl_version();
	int counter = 0; /* Number of tests we have done. */
	int step;
	int op, opa;
	int sf, sfa, df, dfa;

	unsigned num_src_factors_sep, num_dst_factors_sep;
	unsigned num_operators_rgb, num_operators_a;

	/* Find out what kind of GL blending capability we have. */
	have_sep_func = false;
	have_blend_equation = false;
	have_blend_equation_sep = false;
	have_blend_color = false;
	if (gl_version >= 14)
	{
		have_blend_equation = true;
			
		if (piglit_is_extension_supported(
			"GL_EXT_blend_func_separate")) {
			have_sep_func = true;
		}

		if (piglit_is_extension_supported("GL_EXT_blend_color")) {
			have_blend_color = true;
		}
	}
	else if (piglit_is_extension_supported("GL_EXT_blend_subtract") &&
		 piglit_is_extension_supported("GL_EXT_blend_min_max")) {
		have_blend_equation = true;
	}

	if (gl_version >= 20) {
		have_blend_equation_sep = true;
	}
	else if (piglit_is_extension_supported(
		"GL_EXT_blend_equation_separate")) {
		have_blend_equation_sep = true;
	}

	if (have_blend_color) {
		/* Just one blend color setting for all tests */
		/* A bright, semi-transparent blue */
		glBlendColor(constant_color[0], constant_color[1],
			     constant_color[2], 
			     constant_color[3]);
	}

	if (have_sep_func) {
		num_src_factors_sep = ARRAY_SIZE(src_factors);
		num_dst_factors_sep = ARRAY_SIZE(dst_factors);
	}
	else {
		num_src_factors_sep = 1;
		num_dst_factors_sep = 1;
	}

	if (have_blend_equation) {
		num_operators_rgb = ARRAY_SIZE(operators);
		num_operators_a = ARRAY_SIZE(operators);
	}
	else {
		num_operators_rgb = 1; /* just ADD */
		num_operators_a = 1; /* just ADD */
	}

	for (op = 0; op < num_operators_rgb; ++op) {
		for (opa = 0; opa < num_operators_a; ++opa) {
			if (operators[op] == GL_FUNC_ADD && 
			    operators[opa] == GL_FUNC_ADD) {
				/* test _all_ blend term combinations */
				step = 1;
			}
			else if (operators[op] == GL_MIN || 
				 operators[op] == GL_MAX ||
				 operators[opa] == GL_MIN || 
				 operators[opa] == GL_MAX) {
				/* blend terms are N/A so only */
				/* do one iteration of loops */
				step = HUGE_STEP;
			}
			else {
				/* subtract modes: */
				/*do every 3rd blend term for speed */
				step = 3;
			}

			for (sf = 0; 
			     sf < ARRAY_SIZE(src_factors); 
			     sf += step) {
				for (sfa = 0; 
				     sfa < num_src_factors_sep; 
				     sfa += step) {
					for (df = 0; 
					     df < ARRAY_SIZE(dst_factors); 
					     df += step) {
						for (dfa = 0; dfa < 
						     num_dst_factors_sep;
						     dfa += step) {
							pass &= proc_factors(
								sf, sfa, 
								df, dfa,
								&counter,
								op, opa);
						}
					}
				}
			}
		}
	}

	printf("\nRan %i tests.\n", counter);
	return pass;
} /* run_all_factor_sets */

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	pass &= run_all_factor_sets();
	
	if (!piglit_automatic) {
		/* 
		 * Draw our three images, separated by some space. 
		 * This will show only the results of the last test.
		 */

		/* Draw dst */
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, dst_img.name);
		piglit_draw_rect_tex(0, 0, img_width, img_height, 0, 0, 1, 1);

		/* Draw src */
		glBindTexture(GL_TEXTURE_2D, src_img.name);
		piglit_draw_rect_tex(img_width + 2, 0, 
			img_width, img_height, 0, 0, 1, 1);

		/* Draw exp */
		glBindTexture(GL_TEXTURE_2D, exp_img.name);
		/* Have to resend the texture to GL to update GL's copy */
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			     img_width, img_height, 0, 
			     GL_RGBA, GL_FLOAT, exp_img.data);
		piglit_draw_rect_tex(2*(img_width + 2), 0, 
			img_width, img_height, 0, 0, 1, 1);
		glDisable(GL_TEXTURE_2D);
		
		piglit_present_results();
	}


	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
} /* piglit_display */
