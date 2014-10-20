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

/** @file fbo-generatemipmap.c
 *
 * Tests that glCopyTexSubImage work to a mipmap level of a NPOT texture.
 * This tries to catch a bug with the Intel driver and texture tiling.
 */

#include "piglit-util-gl.h"

#define TEX_WIDTH 254
#define TEX_HEIGHT 254

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 700;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const float red[] =   {1, 0, 0, 0};
static const float green[] = {0, 1, 0, 0};
static const float blue[] =  {0, 0, 1, 0};
static const float white[] = {1, 1, 1, 1};

static int
create_fbo(void)
{
	GLuint tex, copied_tex, fb;
	GLenum status;
	int i, dim;
	int draw_w = TEX_WIDTH / 2, draw_h = TEX_HEIGHT / 2;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	for (i = 0, dim = TEX_WIDTH; dim >0; i++, dim /= 2) {
		glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA,
			     dim, dim,
			     0,
			     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Draw into the second level. */
	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  1);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "FBO incomplete\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	glViewport(0, 0, draw_w, draw_h);
	piglit_ortho_projection(draw_w, draw_h, GL_FALSE);

	glColor4fv(red);
	piglit_draw_rect(0, 0, draw_w / 2, draw_h / 2);
	glColor4fv(green);
	piglit_draw_rect(draw_w / 2, 0, draw_w, draw_h / 2);
	glColor4fv(blue);
	piglit_draw_rect(0, draw_h / 2, draw_w/2, draw_h);
	glColor4fv(white);
	piglit_draw_rect(draw_w / 2, draw_h / 2, draw_w, draw_h);

	glGenTextures(1, &copied_tex);
	glBindTexture(GL_TEXTURE_2D, copied_tex);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, draw_w, draw_h, 0);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glDeleteFramebuffersEXT(1, &fb);
	glDeleteTextures(1, &tex);

	return copied_tex;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint tex;
	int x1 = 10 + TEX_WIDTH / 4;
	int x2 = 10 + TEX_WIDTH * 3 / 4;
	int y1 = 10 + TEX_HEIGHT / 4;
	int y2 = 10 + TEX_HEIGHT * 3 / 4;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_fbo();
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	piglit_draw_rect_tex(10, 10, TEX_WIDTH, TEX_HEIGHT,
			     0, 0, 1, 1);

	pass &= piglit_probe_pixel_rgb(x1, y1, red);
	pass &= piglit_probe_pixel_rgb(x2, y1, green);
	pass &= piglit_probe_pixel_rgb(x1, y2, blue);
	pass &= piglit_probe_pixel_rgb(x2, y2, white);

	glDeleteTextures(1, &tex);
	glDisable(GL_TEXTURE_2D);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_ARB_texture_non_power_of_two");
}
