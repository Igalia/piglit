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

/** @file spec/arb_color_buffer_float/clear.c
 *
 * Tests that vertex and fragment color clamping do not affect glClear as
 * specified by ARB_color_buffer_float
 */

/*
 * (modify second paragraph, p. 216, removing clamp of clear color)
 * void ClearColor(float r, float g, float b, float a);
 * sets the clear value for the color buffers in RGBA mode.
 *
 * Fixed-point RGBA
 * color buffers are cleared to a color values derived by taking the
 * clear color, clamping to [0,1], and converting to fixed-point
 * according to the rules of section 2.14.9.
 */

#include "common.h"

GLboolean test()
{
	GLboolean pass = GL_TRUE;

	for(vert_clamp = 0; vert_clamp < 3; ++vert_clamp)
	{
		for(frag_clamp = 0; frag_clamp < 3; ++frag_clamp)
		{
			GLboolean cpass;
			GLboolean opass;
			printf("glClear of fbo for float texture with vertex clamp %s and fragment clamp %s (expecting no clamping)\n", clamp_strings[vert_clamp], clamp_strings[frag_clamp]);
			glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, clamp_enums[vert_clamp]);
			glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, clamp_enums[frag_clamp]);

			glClearColor(pixels[0], pixels[1], pixels[2], pixels[3]);
			glClear(GL_COLOR_BUFFER_BIT);

			expected = fixed ? clamped_pixels : pixels;

			cpass = piglit_probe_pixel_rgba(0, 0, expected);
			opass = cpass;
			if(!cpass && ati_driver && format == GL_RGBA16F_ARB)
			{
				printf("ATI driver known *** MAJOR BUG ***: they always clamp clears for fp16 targets!\n");
				opass = GL_TRUE;
			}
			pass = opass && pass;
		}
	}
	return pass;
}

unsigned init()
{
	return TEST_SRT;
}
