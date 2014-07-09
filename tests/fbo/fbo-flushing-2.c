/*
 * Copyright © 2009,2011 Intel Corporation
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

/** @file fbo-flushing-2.c
 *
 * Tests that rendering to a texture then texturing from it gets
 * correct results.
 *
 * This caught a bug where the texture cache wasn't flushed
 * appropriately on the Intel drivers once additional state changes
 * had been removed.
 */

#include "piglit-util-gl.h"

#define TEX_WIDTH 8
#define TEX_HEIGHT 8

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int x, y;
	GLuint tex, fb;
	float blue[] =   {1, 0, 0, 0};
	float green[] = {0, 1, 0, 0};
	bool draw_green;
	float *draw_colors[] = {blue, green};
	float w_screen = 2.0f * TEX_WIDTH / piglit_width;
	float h_screen = 2.0f * TEX_HEIGHT / piglit_height;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		     TEX_WIDTH, TEX_HEIGHT, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	assert(glGetError() == 0);

	assert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) ==
	       GL_FRAMEBUFFER_COMPLETE_EXT);

	draw_green = true;
	for (y = 0; y <= piglit_height - TEX_HEIGHT; y += TEX_HEIGHT) {
		float y_screen = -1.0 + 2.0 * ((float)y / piglit_height);

		for (x = 0; x <= piglit_width - TEX_WIDTH; x += TEX_WIDTH) {
			float x_screen = -1.0 + 2.0 * ((float)x / piglit_width);

			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
			glDisable(GL_TEXTURE_2D);
			glColor4fv(draw_colors[draw_green]);
			piglit_draw_rect(-1, -1, 2, 2);

			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
			glEnable(GL_TEXTURE_2D);
			piglit_draw_rect_tex(x_screen, y_screen,
					     w_screen, h_screen,
					     0, 0,
					     1, 1);

			draw_green = !draw_green;
		}
		/* Make it a checkerboard. */
		draw_green = !draw_green;
	}

	glDeleteFramebuffersEXT(1, &fb);
	glDeleteTextures(1, &tex);

	draw_green = true;
	for (y = 0; y <= piglit_height - TEX_HEIGHT; y += TEX_HEIGHT) {
		for (x = 0; x <= piglit_width - TEX_WIDTH; x += TEX_WIDTH) {
			float *expected = draw_colors[draw_green];

			pass = pass && piglit_probe_rect_rgb(x, y,
							     TEX_WIDTH,
							     TEX_HEIGHT,
							     expected);

			draw_green = !draw_green;
		}
		draw_green = !draw_green;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
}
