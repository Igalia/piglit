/*
 * Copyright © 2010 Intel Corporation
 * Copyright © 2012 Marek Olšák <maraeo@gmail.com>
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
 *    Marek Olšák <maraeo@gmail.com>
 *
 */

#include "piglit-util-gl.h"
#include "fbo-formats.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Draw a quad with colormask
 */
static void colormask(const float *rect, unsigned mask)
{
	glColorMask(!!(mask & 1), !!(mask & 2), !!(mask & 4), !!(mask & 8));
	piglit_draw_rect(rect[0], rect[1], rect[2], rect[3]);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

static enum piglit_result test_format(const struct format_desc *format)
{
	GLboolean pass = GL_TRUE;
	GLuint tex, fb;
	GLenum status;
	int r, g, b, l, a, i;
	unsigned mask, k;
	float defaults[] = {-1, -1, -1, -1};

	if (format->base_internal_format == GL_DEPTH_COMPONENT ||
	    format->base_internal_format == GL_DEPTH_STENCIL)
		return PIGLIT_SKIP;

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

	/* Set up expected defaults.
	 * We're using glReadPixels from the texture.
	 */
	if (i) {
		/* GL_INTENSITY texture:  result = (I,0,0,1) */
		defaults[3] = 1;
		defaults[2] = defaults[1] = 0;
	} else if (l) {
		/* GL_LUMINANCE texture:  result = (L,0,0,A) */
		defaults[2] = defaults[1] = 0;
		if (!a) {
			defaults[3] = 1;
		}
	} else {
		/* other format */
		if (!r) {
			defaults[0] = 0;
		}
		if (!g) {
			defaults[1] = 0;
		}
		if (!b) {
			defaults[2] = 0;
		}
		if (!a) {
			defaults[3] = 1;
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
	assert(glGetError() == 0);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	printf("Testing %s", format->name);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf(" - fbo incomplete (status = %s)\n",
		       piglit_get_gl_enum_name(status));
		piglit_report_subtest_result(PIGLIT_SKIP,
					     "%s (fbo incomplete)",
					     format->name);
		return PIGLIT_SKIP;
	}
	printf("\n");

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	for (mask = 0; mask <= 15; mask++) {
		float rect[4];
		rect[0] = -1 + (mask / 8.0);
		rect[1] = -1;
		rect[2] = 2 / 16.0;
		rect[3] = 2;

		colormask(rect, mask);
	}

	for (mask = 0; mask <= 15; mask++) {
		float color[4], out[4];

		for (k = 0; k < 4; k++) {
			if (defaults[k] >= 0)
				color[k] = defaults[k];
			else
				color[k] = (mask & (1 << k)) ? 1 : 0;
		}

		if (!piglit_probe_pixel_rgba_silent(piglit_width * mask / 16, 0,
						    color, out)) {
			printf("glColorMask(%i, %i, %i, %i)\n",
			       !!(mask & 1), !!(mask & 2),
			       !!(mask & 4), !!(mask & 8));
			printf("  Expected: %f %f %f %f\n",
			       color[0], color[1], color[2], color[3]);
			printf("  Observed: %f %f %f %f\n",
			       out[0], out[1], out[2], out[3]);
			pass = GL_FALSE;
		}
	}

	/*
	 * Display the texture.
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
