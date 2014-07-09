/*
 * Copyright © 2011 LunarG, Inc.
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
 * Author: Chia-I Wu <olv@lunarg.com>
 */

/** @file draw-tex.c
 *
 * Test GL_OES_draw_texture.
 */

#include <EGL/egl.h>

#include "piglit-util-gl.h"

#define TEXTURE_SIZE 2

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 10;

	config.window_width = 100;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/* see piglit_rgbw_texture */
static const float red[4] =   { 1.0f, 0.0f, 0.0f, 0.0f };
static const float green[4] = { 0.0f, 1.0f, 0.0f, 0.25f };
static const float blue[4] =  { 0.0f, 0.0f, 1.0f, 0.50f };
static const float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

static PFNGLDRAWTEXIOESPROC piglit_glDrawTexiOES;

/**
 * Test the basic use of glDrawTex
 */
static int
test_basic(void)
{
	const GLint crop[4] = {
		0, 0, TEXTURE_SIZE, TEXTURE_SIZE
	};
	const int x = piglit_width / 2 - 2;
	const int y = piglit_height / 2 - 2;
	int pass;

	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop);

	/* draw the RGBW texture */
	piglit_glDrawTexiOES(0, 0, 0, piglit_width, piglit_height);

	pass = piglit_probe_pixel_rgb(x,     y,     red);
	pass = piglit_probe_pixel_rgb(x + 5, y,     green) && pass;
	pass = piglit_probe_pixel_rgb(x,     y + 5, blue)  && pass;
	pass = piglit_probe_pixel_rgb(x + 5, y + 5, white) && pass;

	if (!pass)
		fprintf(stderr, "glDrawTexiOES() failed\n");

	return pass;
}

/**
 * Test glDrawTex with a crop rectangle with negative width/height.
 */
static int
test_negative_crop(void)
{
	const GLint crop[4] = {
		TEXTURE_SIZE, TEXTURE_SIZE, -TEXTURE_SIZE, -TEXTURE_SIZE
	};
	const int x = piglit_width / 2 - 2;
	const int y = piglit_height / 2 - 2;
	int pass;

	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop);

	/* draw the RGBW texture with negative crop */
	piglit_glDrawTexiOES(0, 0, 0, piglit_width, piglit_height);

	pass = piglit_probe_pixel_rgb(x,     y,     white);
	pass = piglit_probe_pixel_rgb(x + 5, y,     blue)  && pass;
	pass = piglit_probe_pixel_rgb(x,     y + 5, green) && pass;
	pass = piglit_probe_pixel_rgb(x + 5, y + 5, red)   && pass;

	if (!pass)
		fprintf(stderr, "negative crop width/height failed\n");

	return pass;
}

/**
 * Test glDrawTex with a small crop rectangle covering only the right-top of
 * the texture.
 */
static int
test_right_top_crop(void)
{
	const GLint crop[4] = {
		TEXTURE_SIZE / 2, TEXTURE_SIZE / 2,
		TEXTURE_SIZE / 2, TEXTURE_SIZE / 2
	};
	const int x = piglit_width / 2 - 2;
	const int y = piglit_height / 2 - 2;
	int pass;

	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop);

	/* draw the right top quarter of RGBW texture */
	piglit_glDrawTexiOES(0, 0, 0, piglit_width, piglit_height);

	pass = piglit_probe_pixel_rgb(x,     y,     white);
	pass = piglit_probe_pixel_rgb(x + 5, y,     white) && pass;
	pass = piglit_probe_pixel_rgb(x,     y + 5, white) && pass;
	pass = piglit_probe_pixel_rgb(x + 5, y + 5, white) && pass;

	if (!pass)
		fprintf(stderr, "sub crop rect failed\n");

	return pass;
}

/**
 * Test glDrawTex with non-zero x and y.
 */
static int
test_right_top_win(void)
{
	const GLint crop[4] = {
		0, 0, TEXTURE_SIZE, TEXTURE_SIZE
	};
	const int half_width = piglit_width / 2;
	const int half_height = piglit_height / 2;
	const int x = half_width + half_width / 2 - 2;
	const int y = half_height + half_height / 2 - 2;
	int pass;

	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop);

	/* draw the RGBW texture at the right top */
	piglit_glDrawTexiOES(half_width, half_height, 0, half_width, half_height);

	pass = piglit_probe_pixel_rgb(x,     y,     red);
	pass = piglit_probe_pixel_rgb(x + 5, y,     green) && pass;
	pass = piglit_probe_pixel_rgb(x,     y + 5, blue)  && pass;
	pass = piglit_probe_pixel_rgb(x + 5, y + 5, white) && pass;

	if (!pass)
		fprintf(stderr, "non-zero (x, y) failed\n");

	return pass;
}

/**
 * Test glDrawTex with non-zero z.
 */
static int
test_depth(void)
{
	const GLint crop[4] = {
		0, 0, TEXTURE_SIZE, TEXTURE_SIZE
	};
	const int x = piglit_width / 2 - 2;
	const int y = piglit_height / 2 - 2;
	int pass;

	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop);

	glEnable(GL_DEPTH_TEST);

	/* draw at near plane */
	piglit_glDrawTexiOES(0, 0, 0, piglit_width, piglit_height);
	/* draw at far plane: should be no-op */
	piglit_glDrawTexiOES(0, 0, 1, piglit_width / 2, piglit_height / 2);

	glDisable(GL_DEPTH_TEST);

	pass = piglit_probe_pixel_rgb(x, y, red);

	if (!pass)
		fprintf(stderr, "non-zero depth failed\n");

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pass = test_basic();
	pass = test_negative_crop() && pass;
	pass = test_right_top_win() && pass;
	pass = test_right_top_crop() && pass;
	pass = test_depth() && pass;

	glFinish();
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint tex;

	piglit_require_extension("GL_OES_draw_texture");
	piglit_glDrawTexiOES = (PFNGLDRAWTEXIOESPROC)
		eglGetProcAddress("glDrawTexiOES");
	if (!piglit_glDrawTexiOES)
		piglit_report_result(PIGLIT_FAIL);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	tex = piglit_rgbw_texture(GL_RGBA,
			TEXTURE_SIZE, TEXTURE_SIZE, GL_FALSE, GL_TRUE,
			GL_UNSIGNED_BYTE);

	glBindTexture(GL_TEXTURE_2D, tex);
	glEnable(GL_TEXTURE_2D);
}
