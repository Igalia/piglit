/*
 * Copyright © 2013 Intel Corporation
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
 * Tests that sampler behavior works when texture units are not used
 * contiguously starting from 0.
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_ALPHA;

PIGLIT_GL_TEST_CONFIG_END

static int texrect_w;
static int texrect_h;
static const float r[] = {1, 0, 0, 1};
static const float g[] = {0, 1, 0, 1};
static const float b[] = {0, 0, 1, 1};
static const float w[] = {1, 1, 1, 1};

static bool
test_nearest(int x)
{
	return (piglit_probe_rect_rgba(x, 0,
				       texrect_w, texrect_h, r) &&
		piglit_probe_rect_rgba(x + texrect_w, 0,
				       texrect_w, texrect_h, g) &&
		piglit_probe_rect_rgba(x, texrect_h,
				       texrect_w, texrect_h, b) &&
		piglit_probe_rect_rgba(x + texrect_w, texrect_h,
				       texrect_w, texrect_h, w));
}

static bool
test_linear(int x)
{
	float average[4];
	int i;

	/* Figure out what the blended middle pixel of the magnified
	 * texture ought to be.  Given a default piglit tolerance of
	 * .02, the texture needs to be at least around 64x64 for this
	 * to probably work out.
	 */
	for (i = 0; i < 4; i++)
		average[i] = (r[i] + g[i] + b[i] + w[i]) / 4;

	return (piglit_probe_pixel_rgba(x,
					0,
					r) &&
		piglit_probe_pixel_rgba(x + texrect_w * 2 - 1,
					0,
					g) &&
		piglit_probe_pixel_rgba(x,
					texrect_h * 2 - 1,
					b) &&
		piglit_probe_pixel_rgba(x + texrect_w * 2 - 1,
					texrect_h * 2 - 1,
					w) &&
		piglit_probe_pixel_rgba(x + texrect_w,
					texrect_h, average));
}

enum piglit_result
piglit_display(void)
{
	static const char *fp_source =
		"!!ARBfp1.0\n"
		"TEX result.color, fragment.texcoord[0], texture[1], 2D;\n"
		"END\n";
	bool pass = true;
	GLuint tex;
	GLuint prog;

	texrect_w = piglit_width / 4 / 2;
	texrect_h = piglit_height / 2;

	glGenProgramsARB(1, &prog);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, prog);
	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,
			   GL_PROGRAM_FORMAT_ASCII_ARB,
			   strlen(fp_source),
			   (const GLubyte *) fp_source);

	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	glActiveTexture(GL_TEXTURE1);
	tex = piglit_rgbw_texture(GL_RGBA, 2, 2, false, false,
				  GL_UNSIGNED_NORMALIZED);

	/* Given that the failure mode we had that led to this test
	 * being written was that the sampler state read was
	 * pseudo-random, go through several statechanges on the
	 * sampler to make sure we're reliably getting our sampler
	 * state.
	 */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	piglit_draw_rect_tex(-1, -1, 0.5, 2,
			     0, 0, 1, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	piglit_draw_rect_tex(-0.5, -1, 0.5, 2,
			     0, 0, 1, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	piglit_draw_rect_tex(0, -1, 0.5, 2,
			     0, 0, 1, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	piglit_draw_rect_tex(0.5, -1, 0.5, 2,
			     0, 0, 1, 1);

	pass = pass && test_nearest(piglit_width * 0 / 4);
	pass = pass && test_linear(piglit_width * 1 / 4);
	pass = pass && test_nearest(piglit_width * 2 / 4);
	pass = pass && test_linear(piglit_width * 3 / 4);

	piglit_present_results();

	glDeleteTextures(1, &tex);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	glDeleteProgramsARB(1, &prog);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
}
