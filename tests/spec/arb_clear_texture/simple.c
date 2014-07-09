/*
 * Copyright 2014 Ilia Mirkin
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

/** @file simple.c
 *
 * A very simple test of basic ClearTexImage and ClearTexSubImage
 * functionality. Clears 2 textures, and puts them up side-by-side for
 * display.
 *
 * The output should look like
 *
 * +-----+--+--+
 * |     |  |  |
 * |     |  |  |
 * +-----+--+--+
 *
 * With the boxes from left to right being green, blue, and yellow.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;

	config.window_width = 128;
	config.window_height = 64;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint texture[2];
static const float green[3] = {0.0, 1.0, 0.0};
static const float red[3] = {1.0, 0.0, 0.0};
static const float blue[3] = {0.0, 0.0, 1.0};
static const float yellow[3] = {1.0, 1.0, 0.0};

void
piglit_init(int argc, char **argv)
{
	int i;
	float *color = malloc(sizeof(float) * 64 * 64 * 3);

	piglit_require_extension("GL_ARB_clear_texture");
	piglit_require_extension("GL_EXT_framebuffer_object");

	/* Create color data for texture */
	for (i = 0; i < 64 * 64; i++) {
		memcpy(&color[i*3], &red[0], sizeof(red));
	}

	glGenTextures(2, texture);

	/* Initialize textures to all red. */
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64,
		     0, GL_RGB, GL_FLOAT, color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64,
		     0, GL_RGB, GL_FLOAT, color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	free(color);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Clear the whole first texture with green */
	glClearTexImage(texture[0], 0, GL_RGB, GL_FLOAT, green);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* Clear the left half of the second texture with blue */
	glClearTexSubImage(texture[1], 0,
			   0, 0, 0,
			   32, 64, 1,
			   GL_RGB, GL_FLOAT, blue);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	/* And the right half with yellow */
	glClearTexSubImage(texture[1], 0,
			   32, 0, 0,
			   32, 64, 1,
			   GL_RGB, GL_FLOAT, yellow);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* Render both textures to the screen */
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	piglit_draw_rect_tex(0, 0, 64, 64, 0, 0, 1, 1);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	piglit_draw_rect_tex(64, 0, 64, 64, 0, 0, 1, 1);

	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(2, texture);

	/* Check for the 3 separate regions */
	pass &= piglit_probe_rect_rgb(0, 0, 64, 64, green);
	pass &= piglit_probe_rect_rgb(64, 0, 32, 64, blue);
	pass &= piglit_probe_rect_rgb(96, 0, 32, 64, yellow);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
