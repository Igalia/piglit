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

/** @file fbo-flushing.c
 *
 * Tests that rendering to a texture then texturing from it gets correct
 * results.
 *
 * This caught a bug where the texture cache wasn't flushed appropriately
 * on the Intel drivers once additional batchbuffer flushing had been
 * removed.
 */

#include "piglit-util-gl.h"

#define TEX_WIDTH 128
#define TEX_HEIGHT 128

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 300;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int y, size;
	GLuint tex, fb;
	const float red[] =   {1, 0, 0, 0};
	const float green[] = {0, 1, 0, 0};

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		     TEX_WIDTH, TEX_HEIGHT, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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

	/* For each power of two size we test, draw red to it, draw it to
	 * the framebuffer, then draw green to it and draw it to the
	 * framebuffer.
	 *
	 * Hopefully between these we'll catch any flushing fail.
	 */
	y = 0;
	for (size = TEX_WIDTH; size > 0; size /= 2) {
		glDisable(GL_TEXTURE_2D);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
		glColor4fv(red);
		piglit_ortho_projection(TEX_WIDTH, TEX_HEIGHT, GL_FALSE);
		piglit_draw_rect(0, y, size, size);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
 		piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
		piglit_draw_rect_tex(0, y, size, size, 0, 0, 1, 1);

		glDisable(GL_TEXTURE_2D);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
		glColor4fv(green);
		piglit_ortho_projection(TEX_WIDTH, TEX_HEIGHT, GL_FALSE);
		piglit_draw_rect(0, y, size, size);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
 		piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
		piglit_draw_rect_tex(0, y, size, size, 0, 0, 1, 1);
		y += size + 5;
	}

	glDeleteFramebuffersEXT(1, &fb);
	glDeleteTextures(1, &tex);

	y = 0;
	for (size = TEX_WIDTH; size > 0; size /= 2) {
		pass = pass && piglit_probe_rect_rgb(0, y, size, size, green);
		y += size + 5;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
}
