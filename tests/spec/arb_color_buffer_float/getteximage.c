/*
 * Copyright © 2010 Luca Barbieri
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
 *
 */

/** @file spec/arb_color_buffer_float/getteximage.c
 *
 * Tests that read color clamping doesn't affect glGetTexImage as
 * specified by ARB_color_buffer_float
 */

#include "common.h"

GLboolean test()
{
	GLboolean pass = GL_TRUE;
	unsigned read_clamp;

	for(read_clamp = 0; read_clamp < 3; ++read_clamp)
	{
		float observed[16];
		float* expected;

		printf("glGetTexImage of %s texture with read clamp %s (expecting %sclamping)\n", format_name, clamp_strings[read_clamp], fixed && !fixed_snorm ? "" : "no ");
		if (!sanity)
			glClampColorARB(GL_CLAMP_READ_COLOR_ARB, clamp_enums[read_clamp]);

		memset(observed, 0, sizeof(observed));
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, observed);

		expected = fixed_snorm ? signed_clamped_pixels :
			   fixed ? clamped_pixels :
			   pixels;
		pass = compare_arrays(expected, observed, 4, 4) && pass;
	}
	return pass;
}

unsigned init()
{
	return TEST_NO_RT;
}
