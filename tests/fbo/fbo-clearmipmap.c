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

/** @file fbo-clear.c
 *
 * Tests that glClear works correctly on the mipmap levels of a 2D texture.
 *
 * Catches a 965 driver bug that idr ran into trying to write an app.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 700;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define TEX_WIDTH 256
#define TEX_HEIGHT 256

static const float colors[][3] = {
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0},
	{1.0, 1.0, 0.0},
	{0.0, 1.0, 1.0},
	{1.0, 0.0, 1.0},
	{0.5, 0.0, 0.5},
	{1.0, 1.0, 1.0},
};

void
piglit_init(int argc, char **argv)
{
	GLuint tex, fb;
	GLenum status;
	int i, dim;

	piglit_require_extension("GL_EXT_framebuffer_object");

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	for (i = 0, dim = TEX_WIDTH; dim >0; i++, dim /= 2) {
		glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA,
			     dim, dim,
			     0,
			     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	assert(glGetError() == 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	for (i = 0, dim = TEX_WIDTH; dim >0; i++, dim /= 2) {
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					  GL_COLOR_ATTACHMENT0_EXT,
					  GL_TEXTURE_2D,
					  tex,
					  i);


		status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			fprintf(stderr, "FBO incomplete\n");
			goto done;
		}

		glClearColor(colors[i][0],
			     colors[i][1],
			     colors[i][2],
			     0.0);
		glClear(GL_COLOR_BUFFER_BIT);

		assert(glGetError() == 0);
	}
done:
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glDeleteFramebuffersEXT(1, &fb);
	glBindTexture(GL_TEXTURE_2D, tex);
}

static void
draw_mipmap(int x, int y, int dim)
{
	glViewport(0, 0, piglit_width, piglit_height);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBegin(GL_QUADS);

	glTexCoord2f(0, 0);
	glVertex2f(x, y);

	glTexCoord2f(1, 0);
	glVertex2f(x + dim, y);

	glTexCoord2f(1, 1);
	glVertex2f(x + dim, y + dim);

	glTexCoord2f(0, 1);
	glVertex2f(x, y + dim);

	glEnd();
}

static GLboolean
test_mipmap_drawing(int start_x, int start_y, int dim, const float *expected)
{
	return piglit_probe_rect_rgb(start_x, start_y, dim, dim, expected);
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int dim;
	int x, i;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	x = 1;
	for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
		draw_mipmap(x, 1, dim);
		x += dim + 1;
	}

	x = 1;
	for (i = 0, dim = TEX_WIDTH; dim > 1; dim /= 2, i++) {
		pass &= test_mipmap_drawing(x, 1, dim, colors[i]);
		x += dim + 1;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
