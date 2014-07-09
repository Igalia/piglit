/*
 * Copyright © 2012 Intel Corporation
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
 * @file srgb-decode.c
 *
 * Tests interaction between GL_ARB_sampler_objects and
 * GL_EXT_texture_sRGB_decode.
 *
 * From the GL_EXT_texture_sRGB_decode spec:
 *
 *     "4) Should we add forward-looking support for
 *         ARB_sampler_objects?
 *
 *         RESOLVED: YES
 *
 *         If ARB_sampler_objects exists in the implementation, the
 *         sampler objects should also include this parameter per
 *         sampler."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static bool
test_getter(GLuint sampler)
{
	bool pass = true;
	GLint i;

	glGetSamplerParameteriv(sampler, GL_TEXTURE_SRGB_DECODE_EXT, &i);
	if (i != GL_DECODE_EXT) {
		fprintf(stderr, "Default sampler decode was %s, expected %s\n",
			piglit_get_gl_enum_name(i), "GL_DECODE_EXT");
		pass = false;
	}
	glSamplerParameteri(sampler, GL_TEXTURE_SRGB_DECODE_EXT,
			    GL_SKIP_DECODE_EXT);
	glGetSamplerParameteriv(sampler, GL_TEXTURE_SRGB_DECODE_EXT, &i);
	if (i != GL_SKIP_DECODE_EXT) {
		fprintf(stderr, "Updated sampler decode was %s, expected %s\n",
			piglit_get_gl_enum_name(i), "GL_SKIP_DECODE_EXT");
		pass = false;
	}

	glSamplerParameteri(sampler, GL_TEXTURE_SRGB_DECODE_EXT, GL_DECODE_EXT);

	return pass;
}

static bool
draw_and_test(int x, int y, const float *expected)
{
	piglit_draw_rect_tex(x, y, piglit_width / 2, piglit_height / 2,
			     0, 0, 1, 1);
	return piglit_probe_rect_rgba(x, y, piglit_width / 2, piglit_height / 2,
				      expected);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLuint tex, sampler, sampler2;
	const float tex_data[4] = {0.2, 0.4, 0.6, 0.8};
	float decoded_tex_data[4];
	int i;

	for (i = 0; i < 3; i++) {
		decoded_tex_data[i] = piglit_srgb_to_linear(tex_data[i]);
	}
	decoded_tex_data[3] = tex_data[3];

	glClear(GL_COLOR_BUFFER_BIT);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, 1, 1, 0,
		     GL_RGBA, GL_FLOAT, tex_data);

	glGenSamplers(1, &sampler);

	pass = test_getter(sampler) && pass;

	/* First, test statechanging the value of the flag between the
	 * bottom left and bottom right corners.
	 */
	glEnable(GL_TEXTURE_2D);
	glBindSampler(0, sampler);
	pass = draw_and_test(0, 0, decoded_tex_data) && pass;

	glSamplerParameteri(sampler, GL_TEXTURE_SRGB_DECODE_EXT,
			    GL_SKIP_DECODE_EXT);
	pass = draw_and_test(piglit_width / 2, 0, tex_data) && pass;

	/* Now, test statechanging the samplers themselves between top left
	 * and top right.
	 */
	glGenSamplers(1, &sampler2);
	glBindSampler(0, sampler2);
	pass = draw_and_test(0, piglit_height / 2, decoded_tex_data) && pass;

	glBindSampler(0, sampler);
	pass = draw_and_test(piglit_width / 2, piglit_height / 2,
			     tex_data) && pass;

	piglit_present_results();

	glDeleteSamplers(1, &sampler);
	glDeleteSamplers(1, &sampler2);
	glDeleteTextures(1, &tex);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_sampler_objects");
	piglit_require_extension("GL_EXT_texture_sRGB");
	piglit_require_extension("GL_EXT_texture_sRGB_decode");

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
