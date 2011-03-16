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

/** @file spec/arb_color_buffer_float/readpixels.c
 *
 * Tests that read color clamping affects 2x2 glReadPixels as specified by
 * ARB_color_buffer_float
 */

/*
 * Modify Section 4.3.2 (Reading Pixels), p. 219
 * [...]
 * (modify second paragraph of "Final Conversion", p. 222) For an
 * RGBA color, if <type> is not FLOAT, or if the CLAMP_READ_COLOR_ARB
 * is TRUE, or CLAMP_READ_COLOR_ARB is FIXED_ONLY_ARB and the selected
 * color buffer is a fixed-point buffer, each component is first
 * clamped to [0,1].  Then the appropriate conversion...
 */

#include "common.h"

GLboolean test()
{
	GLboolean pass = GL_TRUE;
	unsigned read_clamp;

	for(read_clamp = 0; read_clamp < 3; ++read_clamp)
	{
		float observed[16];
		GLboolean cpass = GL_TRUE;
		GLboolean opass;
		unsigned clamped = clamp_enums[read_clamp] == GL_TRUE || (clamp_enums[read_clamp] == GL_FIXED_ONLY_ARB && fixed);
		float* expected;

		printf("glReadPixels of fbo with read clamp %s (expecting %sclamping)\n", clamp_strings[read_clamp], clamped ? "" : "no ");
		if (!sanity)
			glClampColorARB(GL_CLAMP_READ_COLOR_ARB, clamp_enums[read_clamp]);

		memset(observed, 0, sizeof(observed));
		glReadPixels(0, 0, 2, 2, GL_RGBA, GL_FLOAT, observed);

		expected = clamped ? clamped_pixels :
			   fixed_snorm ? signed_clamped_pixels :
			   fixed ? clamped_pixels :
			   pixels;
		cpass = compare_arrays(expected, observed, 4, 4);

		opass = cpass;
		if(!cpass && nvidia_driver && clamped)
		{
			printf("nVidia driver known *** MAJOR BUG ***: they ignore the read clamp!\n");
			opass = GL_TRUE;
		}
		pass = opass && pass;
	}

	if (!sanity)
		glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
	return pass;
}

unsigned init()
{
	return TEST_SRT;
}
