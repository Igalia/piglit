/*
 * Copyright © 2010 Intel Corporation
 * Copyright © 2010 Marek Olšák <maraeo@gmail.com>
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
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *    Marek Olšák <maraeo@gmail.com>
 *
 */

#include "piglit-util-gl.h"
#include "fbo-formats.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Draw a quad with alpha testing
 * \param rect  the coords of the rectangle to draw
 * \param alpha  the alpha value to use when drawing the rect (color is white)
 * \param func  the glAlphaFunc mode to test
 * \param ref  the glAlphaFunc reference value
 */
static void alphatest(const float *rect, float alpha, GLenum func, float ref)
{
	glColor4f(1, 1, 1, alpha);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(func, ref);
	piglit_draw_rect(rect[0], rect[1], rect[2], rect[3]);
	glDisable(GL_ALPHA_TEST);
	glColor4f(1, 1, 1, 1);
}

static enum piglit_result test_format(const struct format_desc *format)
{
	GLboolean pass = GL_TRUE;
	GLuint tex, fb;
	GLenum status;
	int r, g, b, l, a, i;

	float cpass[] = {1, 1, 1, 1};
	float cfail[] = {0, 0, 0, 0};

	float pos0[] = {-1.0,  -1.0, 0.25, 2.0};
	float pos1[] = {-0.75, -1.0, 0.25, 2.0};
	float pos2[] = {-0.5,  -1.0, 0.25, 2.0};
	float pos3[] = {-0.25, -1.0, 0.25, 2.0};
	float pos4[] = { 0.0,  -1.0, 0.25, 2.0};
	float pos5[] = { 0.25, -1.0, 0.25, 2.0};
	float pos6[] = { 0.5,  -1.0, 0.25, 2.0};
	float pos7[] = { 0.75, -1.0, 0.25, 2.0};

        if (format->base_internal_format == GL_DEPTH_COMPONENT ||
            format->base_internal_format == GL_DEPTH_STENCIL ||
	    format->base_internal_format == GL_ALPHA)
		return PIGLIT_SKIP;

	/*
	 * Check alpha test using an FBO that contains/wraps a texture.
	 */

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glViewport(0, 0, piglit_width, piglit_height);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, format->internalformat,
		     piglit_width, piglit_height, 0,
		     GL_RGBA, GL_FLOAT, NULL);

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_LUMINANCE_SIZE, &l);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_ALPHA_SIZE, &a);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_INTENSITY_SIZE, &i);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_RED_SIZE, &r);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_GREEN_SIZE, &g);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_BLUE_SIZE, &b);

	/* Set up expected colors for testing the pass and fail cases.
	 * We're using glReadPixels from the texture.
	 */
        if (i) {
		/* GL_INTENSITY texture:  result = (I,0,0,0) */
		cpass[3] = cpass[2] = cpass[1] = 0;
		cfail[3] = cfail[2] = cfail[1] = 0;
        } else if (l) {
		/* GL_LUMINANCE texture:  result = (L,0,0,A) */
		cpass[2] = cpass[1] = 0;
		cfail[2] = cfail[1] = 0;
		if (!a) {
			cpass[3] = 1;
			cfail[3] = 1;
		}
        } else {
		/* other format */
		if (!r) {
			cpass[0] = 0;
			cfail[0] = 0;
		}
		if (!g) {
			cpass[1] = 0;
			cfail[1] = 0;
		}
		if (!b) {
			cpass[2] = 0;
			cfail[2] = 0;
		}
		if (!a) {
			cpass[3] = 1;
			cfail[3] = 1;
		}
        }

	/* Clamp the bits for the framebuffer, except we aren't checking
	 * the actual framebuffer bits.
	 */
	if (l > 8)
		l = 8;
	if (i > 8)
		i = 8;
	if (r > 8)
		r = 8;
	if (g > 8)
		g = 8;
	if (b > 8)
		b = 8;
	if (a > 8)
		a = 8;

        if (i) {
		piglit_set_tolerance_for_bits(i, i, i, i);
        } else if (l) {
		piglit_set_tolerance_for_bits(l, l, l, a);
        } else {
		piglit_set_tolerance_for_bits(r, g, b, a);
        }

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	printf("Testing %s", format->name);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("- fbo incomplete (status = %s)\n",
		       piglit_get_gl_enum_name(status));
		piglit_report_subtest_result(PIGLIT_SKIP, "%s", format->name);
		return PIGLIT_SKIP;
	}
        printf("\n");

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	alphatest(pos0, 0.2, GL_LESS, 0.25);
	alphatest(pos1, 0.96, GL_LEQUAL, 0.92);
	alphatest(pos2, 0.6, GL_GREATER, 0.55);
	alphatest(pos3, 0.9, GL_GREATER, 0.1);
	alphatest(pos4, 0.35, GL_GEQUAL, 0.4);
	alphatest(pos5, 0.4, GL_EQUAL, 0.4);
	alphatest(pos6, 0.8, GL_NOTEQUAL, 0.8);
	alphatest(pos7, 0.3, GL_NEVER, 3);

	if (!piglit_probe_pixel_rgb_silent(piglit_width * 1 / 16, 0, cpass, NULL)) {
		printf("  FAIL when testing FBO result, 1: 0.2 < 0.25.\n");
		pass = GL_FALSE;
        }
	if (!piglit_probe_pixel_rgb_silent(piglit_width * 3 / 16, 0, cfail, NULL)) {
		printf("  FAIL when testing FBO result, 2: 0.96 <= 0.92.\n");
		pass = GL_FALSE;
        }
	if (!piglit_probe_pixel_rgb_silent(piglit_width * 5 / 16, 0, cpass, NULL)) {
		printf("  FAIL when testing FBO result, 3: 0.6 > 0.55.\n");
		pass = GL_FALSE;
        }
	if (!piglit_probe_pixel_rgb_silent(piglit_width * 7 / 16, 0, cpass, NULL)) {
		printf("  FAIL when testing FBO result, 4: 0.9 > 0.1.\n");
		pass = GL_FALSE;
        }
	if (!piglit_probe_pixel_rgb_silent(piglit_width * 9 / 16, 0, cfail, NULL)) {
		printf("  FAIL when testing FBO result, 5: 0.35 >= 0.4.\n");
		pass = GL_FALSE;
	}
	if (!piglit_probe_pixel_rgb_silent(piglit_width * 11 / 16, 0, cpass, NULL)) {
		printf("  FAIL when testing FBO result, 6: 0.4 == 0.4.\n");
		pass = GL_FALSE;
	}
	if (!piglit_probe_pixel_rgb_silent(piglit_width * 13 / 16, 0, cfail, NULL)) {
		printf("  FAIL when testing FBO result, 7: 0.8 != 0.8.\n");
		pass = GL_FALSE;
	}
	if (!piglit_probe_pixel_rgb_silent(piglit_width * 15 / 16, 0, cfail, NULL)) {
		printf("  FAIL when testing FBO result, 8: FALSE.\n");
		pass = GL_FALSE;
	}


	/*
	 * Now check alpha test using the window buffer.
	 */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glViewport(0, 0, piglit_width, piglit_height);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,   GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

	glColor4f(1, 1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	piglit_draw_rect_tex(-1, -1, 2, 2,
			     0, 0, 1, 1);

	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &tex);
	glBindFramebufferEXT(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glDeleteFramebuffersEXT(1, &fb);

	if (!pass) {
		piglit_present_results();
		piglit_report_subtest_result(PIGLIT_FAIL, "%s", format->name);
		return PIGLIT_FAIL;
	}

	/* Set up expected colors for testing the pass and fail cases.
	 * These are the colors we expect to see with glReadPixels from the window.
	 * The colors are different than above for the intensity/luminance cases
	 * because here we're actually sampling from the texture.
	 */
        if (i) {
		/* GL_INTENSITY texture: RGBA=(I,I,I,I) */
		cpass[3] = cpass[2] = cpass[1] = cpass[0];
		cfail[3] = cfail[2] = cfail[1] = cfail[0];
        } else if (l) {
		/* GL_LUMINANCE texture: RGBA=(L,L,L,A) */
		cpass[2] = cpass[1] = cpass[0];
		cfail[2] = cfail[1] = cfail[0];
		if (!a) {
			cpass[3] = 1;
			cfail[3] = 1;
		}
	}
	else {
		/* same R,G,B,A that we computed above */
	}

	if (!piglit_probe_pixel_rgb_silent(piglit_width * 1 / 16, 0, cpass, NULL)) {
		printf("  FAIL when testing window result, 1: 0.2 < 0.25.\n");
		pass = GL_FALSE;
	}
	if (!piglit_probe_pixel_rgb_silent(piglit_width * 3 / 16, 0, cfail, NULL)) {
		printf("  FAIL when testing window result, 2: 0.96 <= 0.92.\n");
		pass = GL_FALSE;
	}
	if (!piglit_probe_pixel_rgb_silent(piglit_width * 5 / 16, 0, cpass, NULL)) {
		printf("  FAIL when testing window result, 3: 0.6 > 0.55.\n");
		pass = GL_FALSE;
	}
	if (!piglit_probe_pixel_rgb_silent(piglit_width * 7 / 16, 0, cpass, NULL)) {
		printf("  FAIL when testing window result, 4: 0.9 > 0.1.\n");
		pass = GL_FALSE;
	}
	if (!piglit_probe_pixel_rgb_silent(piglit_width * 9 / 16, 0, cfail, NULL)) {
		printf("  FAIL when testing window result, 5: 0.35 >= 0.4.\n");
		pass = GL_FALSE;
	}
	if (!piglit_probe_pixel_rgb_silent(piglit_width * 11 / 16, 0, cpass, NULL)) {
		printf("  FAIL when testing window result, 6: 0.4 == 0.4.\n");
		pass = GL_FALSE;
	}
	if (!piglit_probe_pixel_rgb_silent(piglit_width * 13 / 16, 0, cfail, NULL)) {
		printf("  FAIL when testing window result, 7: 0.8 != 0.8.\n");
		pass = GL_FALSE;
	}
	if (!piglit_probe_pixel_rgb_silent(piglit_width * 15 / 16, 0, cfail, NULL)) {
		printf("  FAIL when testing window result, 8: FALSE.\n");
		pass = GL_FALSE;
	}

	piglit_present_results();

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "%s", format->name);
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result piglit_display(void)
{
	return fbo_formats_display(test_format);
}

void piglit_init(int argc, char **argv)
{
	fbo_formats_init(argc, argv, GL_TRUE);
}
