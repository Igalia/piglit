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

/** @file spec/arb_color_buffer_float/drawpixels.c
 *
 * Tests that fragment color clamping affects glDrawPixels as specified by
 * ARB_color_buffer_float
 */

/* 6. What control should apply to DrawPixels RGBA components?
 * RESOLVED:  The fragment color clamp control.
 */

#include "common.h"

GLboolean test()
{
	GLboolean pass = GL_TRUE;

	for(frag_clamp = 0; frag_clamp < 3; ++frag_clamp)
	{
		GLboolean cpass = GL_TRUE;
		unsigned clamped = clamp_enums[frag_clamp] == GL_TRUE || (clamp_enums[frag_clamp] == GL_FIXED_ONLY_ARB && fixed);
		printf("glDrawPixels of fbo for float texture with fragment clamp %s (expecting %sclamping)\n", clamp_strings[frag_clamp], clamped ? "" : "no ");
		glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, clamp_enums[frag_clamp]);

		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		glWindowPos2fARB(0, 0);
		glDrawPixels(2, 2, GL_RGBA, GL_FLOAT, pixels);

		expected = ((clamped || fixed) ? clamped_pixels : pixels);
		unsigned x, y;
		for(y = 0; y < 2; ++y)
			for(x = 0; x < 2; ++x)
				cpass = piglit_probe_pixel_rgba(x, y, expected + 8 * y + 4 * x) && cpass;

		GLboolean opass = cpass;
		if(!cpass && nvidia_driver && clamped)
		{
			printf("nVidia driver known *** MAJOR BUG ***: they don't clamp glDrawPixels!\n");
			opass = GL_TRUE;
		}
		pass = opass && pass;
	}
	return pass;
}

unsigned init()
{
	piglit_require_extension("GL_ARB_window_pos");
	return TEST_SRT;
}
