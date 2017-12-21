/*
 * Copyright (C) 2007  Intel Corporation
 * Copyright (C) 1999  Allen Akin   All Rights Reserved.
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

/** @file mipmap.c
 *
 * Authors:  Shuang He <shuang.he@intel.com>
 *
 * Draw solid colors to all levels of a 2D texture. Then draw rectangles to
 * the framebuffer sampling from the texture.
 */

#include "piglit-util-gl.h"

#define TEXSIZE 64

enum { BLACK, RED, GREEN, BLUE, WHITE };

static const GLfloat colors[][4] = {{0.0, 0.0, 0.0, 0.0},
				    {1.0, 0.0, 0.0, 1.0},
				    {0.0, 1.0, 0.0, 1.0},
				    {0.0, 0.0, 1.0, 1.0},
				    {1.0, 1.0, 1.0, 1.0}};

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 11;

	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	GLuint fbo;
	GLuint texture;

	glGenFramebuffersEXT(1, &fbo);
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glDisable(GL_TEXTURE_2D);

	for (int i = TEXSIZE, level = 0; i > 0; i /= 2, level++) {
		glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, i, i, 0, GL_RGB,
			     GL_INT, NULL);

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					  GL_COLOR_ATTACHMENT0_EXT,
					  GL_TEXTURE_2D, texture, level);

		const GLenum e =
			glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if (e != GL_FRAMEBUFFER_COMPLETE_EXT) {
			printf("FBO incomplete (%x): %s\n",
			       e, piglit_get_gl_enum_name(e));
			return PIGLIT_FAIL;
		}

		glColor4fv(colors[RED + (level % (WHITE - RED))]);
		glClear(GL_COLOR_BUFFER_BIT);

		piglit_draw_rect(0, 0, TEXSIZE, TEXSIZE);
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glEnable(GL_TEXTURE_2D);

	/* Render to the window */
	glClear(GL_COLOR_BUFFER_BIT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	for (int i = TEXSIZE, x = 0; i > 0; i /= 2) {
		piglit_draw_rect_tex(x, 0, i, i, 0, 0, 1, 1);
		x += i;
		assert(x < piglit_width);
	}

	/* Check result */
	bool pass = true;
	for (int i = TEXSIZE, x = 0, level = 0;
	     i > 1;
	     x += i, i /= 2, level++) {

		if (!piglit_probe_pixel_rgb(
			    x, 0, colors[RED + (level % (WHITE - RED))])) {
			printf("level = %d\n", level);
			pass = false;
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
