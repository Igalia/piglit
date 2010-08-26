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

/** @file spec/arb_color_buffer_float/probepixel.c
 *
 * Tests that read color clamping affects 1x1 glReadPixels as specified by
 * ARB_color_buffer_float
 *
 * This is a separate test from the 2x2 glReadPixels because some
 * implementations (e.g. ATI's) are broken, since they presumably
 * special-case it incorrectly.
 */

#include "common.h"

GLboolean test()
{
	GLboolean pass = GL_TRUE;

	for(read_clamp = 0; read_clamp < 3; ++read_clamp)
	{
		unsigned clamped = clamp_enums[read_clamp] == GL_TRUE || (clamp_enums[read_clamp] == GL_FIXED_ONLY_ARB && fixed);
		printf("probe_pixel of fbo for float texture with read clamp %s (expecting %sclamping)\n", clamp_strings[read_clamp], clamped ? "" : "no ");
		glClampColorARB(GL_CLAMP_READ_COLOR_ARB, clamp_enums[read_clamp]);

		expected = (fixed || clamped) ? clamped_pixels : pixels;

		unsigned x, y;
		for(y = 0; y < 2; ++y)
			for(x = 0; x < 2; ++x)
			{
				GLboolean cpass = piglit_probe_pixel_rgba(x, y, expected + 8 * y + 4 * x);
				GLboolean opass = cpass;
				if(!cpass && clamped && ati_driver)
				{
					printf("ATI driver known bug: 1x1 glReadPixels ignores the read clamp!\n");
					opass = GL_TRUE;
				}
				if(!cpass && nvidia_driver && clamped)
				{
					printf("nVidia driver known *** MAJOR BUG ***: they ignore the read clamp!\n");
					opass = GL_TRUE;
				}
				pass = opass && pass;
			}
	}

	glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
	return pass;
}

unsigned
init()
{
	return TEST_SRT;
}
