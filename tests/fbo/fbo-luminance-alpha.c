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

/** @file fbo-luminance-alpha.c
 *
 * Tests that rendering to and blending on a GL_LUMINANCE8_ALPHA8 FBO works with
 * GL_ARB_framebuffer_object.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint tex, fb;
	GLenum status;
	float fbo_simple1[] = {0.3, 0.0, 0.0, 0.0};
	float fbo_simple2[] = {0.6, 0.0, 0.0, 1.0};
	float fbo_blend1[] =  {0.4, 0.0, 0.0, 0.5};
	float fbo_blend2[] =  {0.56, 0.0, 0.0, 0.4};
	float win_simple1[] = {0.3, 0.3, 0.3, 0.0};
	float win_simple2[] = {0.6, 0.6, 0.6, 1.0};
	float win_blend1[] =  {0.4, 0.4, 0.4, 0.5};
	float win_blend2[] =  {0.56, 0.56, 0.56, 0.4};
	int fbo_width = 64;
	int fbo_height = 64;

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glViewport(0, 0, fbo_width, fbo_height);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8,
		     fbo_width, fbo_height, 0,
		     GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, NULL);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	assert(glGetError() == 0);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "fbo incomplete (status = 0x%04x)\n", status);
		piglit_report_result(PIGLIT_SKIP);
	}

	/* Clear to no alpha. */
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

        /* Draw with two different colors, simple shading. */
	glColor4fv(fbo_simple1);
	piglit_draw_rect(-1.0, -1.0, 0.5, 2.0);

	glColor4fv(fbo_simple2);
	piglit_draw_rect(-0.5, -1.0, 0.5, 2.0);

        /* Draw with blending, test DST_ALPHA. */
	glColor4f(0.0, 0.0, 0.0, 0.5);
	piglit_draw_rect(0.0, -1.0, 0.5, 2.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_ALPHA, GL_ZERO);
	glColor4f(0.8, 0.8, 0.8, 1.0);
	piglit_draw_rect(0.0, -1.0, 0.5, 2.0);
	glDisable(GL_BLEND);

        /* Draw with blending, test SRC_ALPHA. */
	glColor4f(0.7, 0.7, 0.7, 0.5);
	piglit_draw_rect(0.5, -1.0, 0.5, 2.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
	glColor4f(0.0, 0.0, 0.0, 0.8);
	piglit_draw_rect(0.5, -1.0, 0.5, 2.0);
	glDisable(GL_BLEND);

	printf("Testing FBO result, simple 1.\n");
	pass = piglit_probe_pixel_rgba(fbo_width * 1 / 8, 0, fbo_simple1) && pass;
	printf("Testing FBO result, simple 2.\n");
	pass = piglit_probe_pixel_rgba(fbo_width * 3 / 8, 0, fbo_simple2) && pass;
	printf("Testing FBO result, blending DST_ALPHA.\n");
	pass = piglit_probe_pixel_rgba(fbo_width * 5 / 8, 0, fbo_blend1) && pass;
	printf("Testing FBO result, blending SRC_ALPHA.\n");
	pass = piglit_probe_pixel_rgba(fbo_width * 7 / 8, 0, fbo_blend2) && pass;

	/* Draw the two textures to halves of the window. */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glViewport(0, 0, piglit_width, piglit_height);

	glColor4f(1, 1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	piglit_draw_rect_tex(-1, -1, 2, 2,
			     0, 0, 1, 1);

	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &tex);
	glDeleteFramebuffersEXT(1, &fb);

	printf("Testing window result, simple 1.\n");
	pass = piglit_probe_pixel_rgba(piglit_width * 1 / 8, 0, win_simple1) && pass;
	printf("Testing window result, simple 2.\n");
	pass = piglit_probe_pixel_rgba(piglit_width * 3 / 8, 0, win_simple2) && pass;
	printf("Testing window result, blending DST_ALPHA.\n");
	pass = piglit_probe_pixel_rgba(piglit_width * 5 / 8, 0, win_blend1) && pass;
	printf("Testing window result, blending SRC_ALPHA.\n");
	pass = piglit_probe_pixel_rgba(piglit_width * 7 / 8, 0, win_blend2) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_framebuffer_object");
}
