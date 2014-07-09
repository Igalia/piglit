/*
 * Copyright © 2010 Intel Corporation
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
 * \file rg-teximage-01.c
 * Verify setting and getting image data for RED or RG formats
 *
 * Specify an RGBA image with a RED internal format.  Read the image back as
 * RGBA.  Verify the red components read back match the source image and the
 * green, blue, and alpha components are 0, 0, and 1, repsectively.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl.h"
#include "rg-teximage-common.h"

/**
 * Map an integer from [0, maximum] to a float on [0, 2*PI].
 */
#define NORMALIZE_TO_RADIANS(x, maximum)		\
	(((float) x / (float) maximum) * 2.0 * M_PI)

#define EPSILON (1.0 / 255.0)
/* give a large value for compression */
#define EPSILON_COMP (20.0 / 255.0)

enum piglit_result
piglit_display(void)
{
	unsigned i;

	for (i = 0; i < num_tex; i++) {
		glBindTexture(GL_TEXTURE_2D, tex[i]);

		piglit_draw_rect_tex(-1.0 + ((float) (2 * i) / num_tex), -1.0,
				     2.0 / num_tex, 2.0,
				     0.0, 0.0, 1.0, 1.0);
	}

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


GLboolean
compare_texture(const GLfloat *orig, const GLfloat *copy,
		GLenum orig_fmt, GLenum copy_fmt, unsigned num_pix,
		GLboolean has_green)
{
	GLboolean logged = GL_FALSE;
	GLboolean pass = GL_TRUE;
	unsigned i;
	float e;

	if (orig_fmt == GL_COMPRESSED_RED_RGTC1 || orig_fmt == GL_COMPRESSED_RG_RGTC2 ||
	    orig_fmt == GL_COMPRESSED_SIGNED_RED_RGTC1 || orig_fmt == GL_COMPRESSED_SIGNED_RG_RGTC2)
		e = EPSILON_COMP;
	else
		e = EPSILON;

	for (i = 0; i < num_pix; i++) {
		if (fabs(orig[0] - copy[0]) > e) {
			if (!logged) {
				fprintf(stderr,
					"Got bad R channel reading back "
					"0x%04x as 0x%04x\n",
					orig_fmt, copy_fmt);
				logged = GL_TRUE;
			}

			pass = GL_FALSE;
		}

		if (has_green && fabs(orig[0] - copy[0]) > e) {
			if (!logged) {
				fprintf(stderr,
					"Got bad G channel reading back "
					"0x%04x as 0x%04x\n",
					orig_fmt, copy_fmt);
				logged = GL_TRUE;
			}

			pass = GL_FALSE;
		}

		if ((!has_green && copy[1] != 0.0)
		    || copy[2] != 0.0
		    || copy[3] != 1.0) {
			if (!logged) {
				fprintf(stderr,
					"Got bad %s channel reading back "
					"0x%04x as 0x%04x\n",
					has_green ? "B/A" : "G/B/A",
					orig_fmt, copy_fmt);
				logged = GL_TRUE;
			}

			pass = GL_FALSE;
		}

		orig += 4;
		copy += 4;
	}

	return pass;
}


void
generate_rainbow_texture_data(unsigned width, unsigned height, float *img)
{
	unsigned i;
	unsigned j;

	for (i = 0; i < height; i++) {
		const float bias = NORMALIZE_TO_RADIANS(i, height);

		for (j = 0; j < width; j++) {
			const float angle = NORMALIZE_TO_RADIANS(j, width);

			img[0] = (cos(angle + bias) + 1.0) * 0.5;
			img[1] = (sin(angle - bias) + 1.0) * 0.5;
			img[2] = (cos(bias) + 1.0) * 0.5;
			img[3] = (sin(bias) + 1.0) * 0.5;
			img += 4;
		}
	}
}
