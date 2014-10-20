/*
 * Copyright © 2009 Intel Corporation
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
 *
 */

/** @file fbo-readpixels.c
 *
 * Tests that various formats of color renderbuffer get correct results from
 * glReadPixels() versus glClear and immediate mode rendering.
 */

#include "piglit-util-gl.h"

#define BUF_WIDTH 32
#define BUF_HEIGHT 32

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static GLboolean
test_with_format(GLenum internal_format, GLenum format,
		 float results_x, float results_y)
{
	GLuint tex, fb;
	GLenum status;
	GLboolean pass = GL_TRUE;
	int subrect_w = BUF_WIDTH / 5;
	int subrect_h = BUF_HEIGHT / 5;
	int x, y;
	int rbits, gbits, bbits, abits;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format,
		     BUF_WIDTH, BUF_HEIGHT, 0,
		     format, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE,
				 &rbits);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE,
				 &gbits);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE,
				 &bbits);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE,
				 &abits);

	printf("testing with format %s, %s "
	       "(%d,%d,%d,%d rgba)\n",
	       piglit_get_gl_enum_name(internal_format),
	       piglit_get_gl_enum_name(format),
	       rbits, gbits, bbits, abits);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "texture for internalformat %s. "
			"format %s is framebuffer "
			"incomplete (status = %s)\n",
			piglit_get_gl_enum_name(internal_format),
			piglit_get_gl_enum_name(format),
			piglit_get_gl_enum_name(status));
		goto done;
	}

	/* Set matrices */
	glViewport(0, 0, BUF_WIDTH, BUF_HEIGHT);
	piglit_ortho_projection(BUF_WIDTH, BUF_HEIGHT, GL_FALSE);

	/* clear background to purple */
	glClearColor(1.0, 0.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* lower-left square: red */
	glColor4f(1.0, 0.0, 0.0, 0.0);
	piglit_draw_rect(subrect_w * 1, subrect_h * 1,
			 subrect_w, subrect_h);

	/* lower-right square: green */
	glColor4f(0.0, 1.0, 0.0, 0.0);
	piglit_draw_rect(subrect_w * 3, subrect_h * 1,
			 subrect_w, subrect_h);

	/* upper-left square: blue */
	glColor4f(0.0, 0.0, 1.0, 0.0);
	piglit_draw_rect(subrect_w * 1, subrect_h * 3,
			 subrect_w, subrect_h);

	/* upper-right square: black */
	glColor4f(0.0, 0.0, 0.0, 1.0);
	piglit_draw_rect(subrect_w * 3, subrect_h * 3,
			 subrect_w, subrect_h);

	for (y = 0; y < BUF_HEIGHT; y++) {
		for (x = 0; x < BUF_WIDTH; x++) {
			float expected[4];

			if (x >= subrect_w * 1 && x < subrect_w * 2 &&
			    y >= subrect_h * 1 && y < subrect_h * 2) {
				expected[0] = 1.0;
				expected[1] = 0.0;
				expected[2] = 0.0;
				expected[3] = 0.0;
			} else if (x >= subrect_w * 3 && x < subrect_w * 4 &&
				   y >= subrect_h * 1 && y < subrect_h * 2) {
				expected[0] = 0.0;
				expected[1] = 1.0;
				expected[2] = 0.0;
				expected[3] = 0.0;
			} else if (x >= subrect_w * 1 && x < subrect_w * 2 &&
				   y >= subrect_h * 3 && y < subrect_h * 4) {
				expected[0] = 0.0;
				expected[1] = 0.0;
				expected[2] = 1.0;
				expected[3] = 0.0;
			} else if (x >= subrect_w * 3 && x < subrect_w * 4 &&
				   y >= subrect_h * 3 && y < subrect_h * 4) {
				expected[0] = 0.0;
				expected[1] = 0.0;
				expected[2] = 0.0;
				expected[3] = 1.0;
			} else {
				expected[0] = 1.0;
				expected[1] = 0.0;
				expected[2] = 1.0;
				expected[3] = 0.0;
			}
			pass &= piglit_probe_pixel_rgb(x, y, expected);
		}
	}

	/* display the texture by drawing a quad */
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	piglit_draw_rect_tex(results_x, results_y, BUF_WIDTH, BUF_HEIGHT,
			     0, 0, 1, 1);
	glDisable(GL_TEXTURE_2D);

done:
	glDeleteFramebuffersEXT(1, &fb);
	glDeleteTextures(1, &tex);
	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	pass &= test_with_format(GL_RGBA8, GL_BGRA,
				 0, 0);
	pass &= test_with_format(GL_RGB5, GL_RGB,
				 0, BUF_HEIGHT + 1);
	pass &= test_with_format(GL_RGBA4, GL_BGRA,
				 0, (BUF_HEIGHT + 1) * 2);
	pass &= test_with_format(GL_RGB5_A1, GL_BGRA,
				 0, (BUF_HEIGHT + 1) * 3);
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
}
