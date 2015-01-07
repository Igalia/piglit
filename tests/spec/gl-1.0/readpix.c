/*
 * BEGIN_COPYRIGHT -*- glean -*-
 *
 * Copyright (C) 2001  Allen Akin   All Rights Reserved.
 * Copyright (C) 2014  Intel Corporation  All Rights Reserved.
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
 * END_COPYRIGHT
 */

/** @file readpix.c  
 *
 * Checks to make sure glReadPixels is functioning properly.
 *
 * This test performs a sanity check of glReadPixels, using as
 * few other portions of the GL as possible.  If this test fails,
 * it may be pointless to run other tests, since so many of them
 * depend on reading the contents of the framebuffer to determine
 * if they pass.
 * 
 * The test works by using glClear to fill the framebuffer with a
 * randomly-chosen value, reading the contents of the
 * framebuffer, and comparing the actual contents with the
 * expected contents.  RGB, RGBA, color index, stencil, and depth
 * buffers (whichever are applicable to the current rendering
 * context) are checked.  The test passes if the actual contents
 * are within 1 LSB of the expected contents.
 */

#include "piglit-util-gl.h"

#include <math.h>
#include <stdlib.h>
#include <assert.h>


PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | 
		PIGLIT_GL_VISUAL_DOUBLE | 
		PIGLIT_GL_VISUAL_STENCIL | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	srand(0); 
} /* piglit_init */

/** Returns a float in the range [0.0, 1.0]. */
float
random_float(void)
{
	return (float) rand() / RAND_MAX;
}

/**
 * (Copied from Glean's misc.cpp.)
 * Utility routine to compute error, expressed in bits
 *	Typically used to convert a floating-point error (in the range [0,1])
 *	to the number of bits in the representation of a color.
 */
double
error_bits(double abs_error, int rep_bits) {
	double log2_error;
	if (abs_error <= 0.0)
		return 0.0;
	log2_error = log2(abs_error) + rep_bits;
	return (log2_error <= 0.0)? 0.0: log2_error;
} /* error_bits */

bool
check_rgba(void) 
{
	int i, x, y, j;
	int thresh = 1;
	bool pass = true;
	double current_error = 0.0;
	double err;
	int xerr = 0, yerr = 0;
	float expected[4], expected_rgba[4] = { 0 }, actual_rgba[4] = { 0 };
	const int w = piglit_width;
	const int h = piglit_height;
	GLfloat *buf = malloc(h * w * 4 * sizeof *buf);
	GLfloat dr, dg, db, da;
	GLint rbits, gbits, bbits, abits;
	glGetIntegerv(GL_RED_BITS, &rbits);
	glGetIntegerv(GL_GREEN_BITS, &gbits);
	glGetIntegerv(GL_BLUE_BITS, &bbits);
	glGetIntegerv(GL_ALPHA_BITS, &abits);

	for (i = 0; i < 100 && pass; ++i) {
		/* Generate a random color and clear the color buffer: */
		expected[0] = random_float();
		expected[1] = random_float();
		expected[2] = random_float();
		expected[3] = random_float();
		glClearColor(expected[0], expected[1], 
			     expected[2], expected[3]);
		glClear(GL_COLOR_BUFFER_BIT);

		/* Read the buffer: */
		glReadPixels(0, 0, w, h, GL_RGBA, GL_FLOAT, buf);

		/*
		 * Now compute the error for each pixel, and record the
		 * worst one we find:
		 */
		for (y = 0; y < h; ++y) {
			for (x = 0; x < w; ++x) {
				dr = fabs(buf[y*w*4 + x*4 + 0] - expected[0]);
				dg = fabs(buf[y*w*4 + x*4 + 1] - expected[1]);
				db = fabs(buf[y*w*4 + x*4 + 2] - expected[2]);
				da = fabs(buf[y*w*4 + x*4 + 3] - expected[3]);
				err =
				    fmax(error_bits(dr, rbits),
				    fmax(error_bits(dg, gbits),
				    fmax(error_bits(db, bbits),
					error_bits(da,
					   abits? abits: thresh+1))));
					   /*
					    * The "thresh+1" fudge above is
					    * needed to force the error to
					    * be greater than the threshold
					    * in the case where there is no
					    * alpha channel.  Without it the
					    * error would be just equal to
					    * the threshold, and the test
					    * would spuriously pass.
					    */
				if (err > current_error) {
					current_error = err;
					xerr = x;
					yerr = y;
					for (j = 0; j < 4; ++j) {
						expected_rgba[j] = 
							expected[j];
						actual_rgba[j] = 
							buf[y*w*4 + x*4 + j];
					}
				}
			}
		}

		if (current_error > thresh)
			pass = false;

		/* Show the image */
		if (!piglit_automatic) {
			piglit_present_results();
		}
	}

	if (!pass) {
		printf("\tRGB(A) worst-case error was %f bits at (%i, %i)\n",
			current_error, xerr, yerr);
		printf("\t\texpected (%f, %f, %f, %f)\n",
			expected_rgba[0],
			expected_rgba[1],
			expected_rgba[2],
			expected_rgba[3]);
		printf("\t\tgot (%f, %f, %f, %f)\n",
			actual_rgba[0],
			actual_rgba[1],
			actual_rgba[2],
			actual_rgba[3]);
			
	}

	free(buf);

	return pass;
} /* check_rgba */

bool
check_depth(void) 
{
	int i, x, y;
	int thresh = 1;
	bool pass = true;
	GLdouble expected, expected_depth = 0.0, actual, actual_depth = 0.0;
	const int w = piglit_width;
	const int h = piglit_height;
	GLuint *buf = malloc(h * w * sizeof *buf);
	double current_error = 0.0;
	GLfloat dd;
	double err;
	int xerr = 0, yerr = 0;
	GLint dbits;
	glGetIntegerv(GL_DEPTH_BITS, &dbits);

	for (i = 0; i < 100 && pass; ++i) {
		/* Generate a random depth and clear the depth buffer */
		expected = random_float();
		glClearDepth(expected);
		glClear(GL_DEPTH_BUFFER_BIT);

		/*
		 * Because glReadPixels won't return data of type GLdouble,
		 * there's no straightforward portable way to deal with
		 * integer depth buffers that are deeper than 32 bits or
		 * floating-point depth buffers that have higher precision
		 * than a GLfloat.  Since this is just a sanity check, we'll
		 * use integer readback and settle for 32 bits at best.
		 */
		glReadPixels(0, 0, w, h,
			GL_DEPTH_COMPONENT,
			GL_UNSIGNED_INT, buf);

		/*
		 * Now compute the error for each pixel, and record the
		 * worst one we find:
		 */
		for (y = 0; y < h; ++y) {
			for (x = 0; x < w; ++x) {
				actual = buf[y*w + x]/(double)0xffffffffU;
				dd = abs(actual - expected);
				err = error_bits(dd, dbits);
				if (err > current_error) {
					current_error = err;
					xerr = x;
					yerr = y;
					expected_depth = expected;
					actual_depth = actual;
				}
			}
		}

		if (current_error > thresh)
			pass = false;

		/* Show the image */
		if (!piglit_automatic) {
			piglit_present_results();
		}
	}

	if (!pass) {
		printf("\tDepth worst-case error was %f bits at (%i, %i)\n",
			current_error, xerr, yerr);
		printf("\t\texpected %f; got %f.\n", expected_depth, 
			actual_depth);
	}

	free(buf);

	return pass;
} /* check_depth */

/** Generate a random number with the number of bits specified */
unsigned int
random_bits(unsigned int bits)
{
	assert(bits <= 32);
	assert(bits > 0);
	if (bits == 32)
		return rand();
	else
		return rand() % (1 << bits);
}

bool
check_stencil(void) 
{
	int i, x = 0, y = 0;
	bool pass = true;
	const int w = piglit_width;
	const int h = piglit_height;
	GLuint *buf = malloc(h * w * sizeof *buf);
	GLuint expected = 0;
	GLint sbits;
	glGetIntegerv(GL_STENCIL_BITS, &sbits);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	for (i = 0; i < 100 && pass; ++i) {
		expected = random_bits(sbits);
		glClearStencil(expected);
		pass &= piglit_check_gl_error(GL_NO_ERROR);
		glClear(GL_STENCIL_BUFFER_BIT);
		pass &= piglit_check_gl_error(GL_NO_ERROR);

		glReadPixels(0, 0, w, h,
			GL_STENCIL_INDEX, GL_UNSIGNED_INT, buf);
		pass &= piglit_check_gl_error(GL_NO_ERROR);

		for (y = 0; y < h && pass; ++y) {
			for (x = 0; x < w; ++x) {
				if (buf[y*w + x] != expected) {
					pass = false;
					break;
				}
			}
		}

		/* Show the image */
		if (!piglit_automatic) {
			piglit_present_results();
		}
	}

	if (!pass) {
		printf("\tStencil failed at (%i, %i).\n",
			x, y);
		printf("\t\tExpected %i; got %i.\n",
			expected, buf[y*w + x]);
	}
	
	free(buf);

	return pass;
} /* check_stencil */

enum piglit_result
piglit_display(void)
{
	/*
	 * Many (if not most) other tests need to read the contents of
	 * the framebuffer to determine if the correct image has been
	 * drawn.  Obviously this is a waste of time if the basic
	 * functionality of glReadPixels isn't working.
	 *
	 * This test does a "sanity" check of glReadPixels.  Using as
	 * little of the GL as practicable, it writes a random value
	 * in the framebuffer, reads it, and compares the value read
	 * with the value written.
	 */
	bool pass = true;

	glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_PACK_LSB_FIRST, GL_FALSE);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
	glPixelTransferi(GL_MAP_STENCIL, GL_FALSE);
	glPixelTransferi(GL_INDEX_SHIFT, 0);
	glPixelTransferi(GL_INDEX_OFFSET, 0);
	glPixelTransferf(GL_RED_SCALE, 1.0);
	glPixelTransferf(GL_GREEN_SCALE, 1.0);
	glPixelTransferf(GL_BLUE_SCALE, 1.0);
	glPixelTransferf(GL_ALPHA_SCALE, 1.0);
	glPixelTransferf(GL_DEPTH_SCALE, 1.0);
	glPixelTransferf(GL_RED_BIAS, 0.0);
	glPixelTransferf(GL_GREEN_BIAS, 0.0);
	glPixelTransferf(GL_BLUE_BIAS, 0.0);
	glPixelTransferf(GL_ALPHA_BIAS, 0.0);
	glPixelTransferf(GL_DEPTH_BIAS, 0.0);

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_DITHER);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(~0);

	pass &= check_rgba();
	pass &= check_depth();
	pass &= check_stencil();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
} /* piglit_display */
