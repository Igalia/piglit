/*
 * Copyright © 2014 VMware, Inc.
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
 */

/*
 * Test GL_EXT_texture_swizzle (including the _ZERO and _ONE terms).
 * Brian Paul
 * 24 April 2014
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 12;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


#define RED 0.2f
#define GREEN 0.4f
#define BLUE 0.6f
#define ALPHA 0.8f


static float
get_component_color(GLenum swz)
{
	switch (swz) {
	case GL_RED:
		return RED;
	case GL_GREEN:
		return GREEN;
	case GL_BLUE:
		return BLUE;
	case GL_ALPHA:
		return ALPHA;
	case GL_ZERO:
		return 0.0f;
	case GL_ONE:
		return 1.0f;
	default:
		assert(!"Invalid swizzle term");
		return 0.0;
	}
}


static void
get_expected_color(GLenum swz_r, GLenum swz_g, GLenum swz_b, GLenum swz_a,
						 float expected[4])
{
	expected[0] = get_component_color(swz_r);
	expected[1] = get_component_color(swz_g);
	expected[2] = get_component_color(swz_b);
	expected[3] = get_component_color(swz_a);
}


static bool
test_swizzle(GLenum swz_r, GLenum swz_g, GLenum swz_b, GLenum swz_a)
{
	float expected[4];
	bool p;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R_EXT, swz_r);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G_EXT, swz_g);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B_EXT, swz_b);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A_EXT, swz_a);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	get_expected_color(swz_r, swz_g, swz_b, swz_a, expected);

	piglit_draw_rect_tex(-1.0, -1.0, 2.0, 2.0,
			     0.0, 0.0, 1.0, 1.0);

	p = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, expected);

	piglit_present_results();

	return p;
}


enum piglit_result
piglit_display(void)
{
	static const GLenum swizzle_terms[6] = {
		GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA, GL_ZERO, GL_ONE
	};
	bool pass = true;
	int num_failures = 0;
	int r, g, b, a;

	for (r = 0; r < 6; r++) {
		for (g = 0; g < 6; g++) {
			for (b = 0; b < 6; b++) {
				for (a = 0; a < 6; a++) {
					bool p = test_swizzle(swizzle_terms[r],
							      swizzle_terms[g],
							      swizzle_terms[b],
							      swizzle_terms[a]);
					if (!p) {
						/* give up after 10 failures */
						num_failures++;
						if (num_failures >= 10)
							return PIGLIT_FAIL;
					}
					pass = pass && p;
				}
			}
		}
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


static void
setup_texture(void)
{
	GLfloat image[8][8][4];
	int i, j;
	GLuint tex;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			image[i][j][0] = RED;
			image[i][j][1] = GREEN;
			image[i][j][2] = BLUE;
			image[i][j][3] = ALPHA;
		}
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_FLOAT, image);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_texture_swizzle");

	setup_texture();
}
