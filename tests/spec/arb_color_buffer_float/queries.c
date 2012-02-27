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

/** @file spec/arb_color_buffer_float/queries.c
 *
 * Tests that fragment color clamping affects queries as specified by
 * ARB_color_buffer_float
 */

/* Modify Section 6.1.2, Data Conversions, p. 245
 * (add new paragraph at the end of the section, p. 245) If fragment
 * color clamping is enabled, querying of the texture border color,
 * texture environment color, fog color, alpha test reference value,
 * blend color, and RGBA clear color will clamp the corresponding
 * state values to [0,1] before returning them.  This behavior
 * provides compatibility with previous versions of the GL that
 * clamped these values when specified.
 */

#include "common.h"

GLboolean test()
{
	GLboolean pass = GL_TRUE;
	unsigned frag_clamp;

	glBindTexture(GL_TEXTURE_2D, tex); /* for border color */

	for (frag_clamp = sanity ? 1 : 0; frag_clamp < (sanity ? 2 : 3); ++frag_clamp)
	{
		const char *value_names[] = { "texture border color", "texenv color", "fog color", "alpha test reference", "blend color", "clear color" };
		unsigned value;
		for(value = 0; value < 6; ++value)
		{
			float observed[16];
			char test_name[4096];
			GLboolean cpass = GL_TRUE;
			GLboolean opass;
			unsigned clamped = clamp_enums[frag_clamp] == GL_TRUE || (clamp_enums[frag_clamp] == GL_FIXED_ONLY_ARB && fixed);
			unsigned comps = 4;
			float* expected;

			sprintf(test_name, "glGet of %s in %s mode with fragment clamp %s (expecting %sclamping)", value_names[value], mrt_mode_strings[mrt_mode], clamp_strings[frag_clamp], clamped ? "" : "no ");
			printf("%s\n", test_name);

			if (!sanity)
				glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, clamp_enums[frag_clamp]);

			memset(observed, 0, sizeof(observed));
			switch (value)
			{
			case 0:
				/* ARB_color_buffer_float adds a potential clamp on queries, but only ARB_texture_float removes
				 * the clamp on setting the texture border color  */
				if(!piglit_is_extension_supported("GL_ARB_texture_float"))
					clamped = 1;
				glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, pixels);
				glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, observed);
				break;
			case 1:
				glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, pixels);
				glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, observed);
				break;
			case 2:
				glFogfv(GL_FOG_COLOR, pixels);
				glGetFloatv(GL_FOG_COLOR, observed);
				break;
			case 3:
				glAlphaFunc(GL_ALWAYS, pixels[0]);
				glGetFloatv(GL_ALPHA_TEST_REF, observed);
				comps = 1;
				break;
			case 4:
				glBlendColor(pixels[0], pixels[1], pixels[2], pixels[3]);
				glGetFloatv(GL_BLEND_COLOR, observed);
				break;
			case 5:
				glClearColor(pixels[0], pixels[1], pixels[2], pixels[3]);
				glGetFloatv(GL_COLOR_CLEAR_VALUE, observed);
				break;
			}

			error = glGetError();
			if(error)
			{
				printf("GL error after query 0x%04X\n", error);
				cpass = GL_FALSE;
			}

			expected = clamped ? clamped_pixels : pixels;
			cpass = compare_arrays(expected, observed, comps, 1) && cpass;

			opass = cpass;
			if(!cpass && !clamped && ati_driver && value == 0)
			{
				printf("ATI driver known bug: they always clamp queries for the texture border color!\n");
				opass = GL_TRUE;
			}
			if(!cpass && clamped && ati_driver && value == 5)
			{
				printf("ATI driver known bug: they don't clamp queries for the clear color!\n");
				opass = GL_TRUE;
			}
			if(!cpass && !clamped && ati_driver && clamp_enums[frag_clamp] == GL_FIXED_ONLY_ARB && !fixed)
			{
				printf("ATI driver known bug: they clamp queries when FIXED_ONLY is set and the FBO is floating point!\n");
				opass = GL_TRUE;
			}
			if(!cpass && clamped && nvidia_driver && value == 0)
			{
				printf("nVidia driver known bug: they don't clamp queries for the texture border color!\n");
				opass = GL_TRUE;
			}
			printf("%s: %s\n", (cpass ? "PASS" : (opass ? "XFAIL" : "FAIL")), test_name);
			pass = opass && pass;
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	return pass;
}

unsigned
init()
{
	return TEST_SRT_MRT;
}
