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

/** @file fbo-3d.c
 *
 * Tests that drawing to each depth of a 3D texture FBO and then drawing views
 * of those individual depths to the window system framebuffer succeeds.
 */

#include "piglit-util-gl.h"

#define BUF_WIDTH 32
#define BUF_HEIGHT 32

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 200;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

#define NUM_DEPTHS	6
#define POT_DEPTHS	8
float depth_color[NUM_DEPTHS][4] = {
	{1.0, 0.0, 0.0, 0.0},
	{0.0, 1.0, 0.0, 0.0},
	{0.0, 0.0, 1.0, 0.0},
	{1.0, 0.0, 1.0, 0.0},
	{1.0, 1.0, 0.0, 0.0},
	{0.0, 1.0, 1.0, 0.0},
};

int pot_depth;

static int
create_3d_fbo(void)
{
	GLuint tex, fb;
	GLenum status;
	int depth;
	pot_depth = piglit_is_extension_supported("GL_ARB_texture_non_power_of_two") ?
		NUM_DEPTHS: POT_DEPTHS;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_3D, tex);

	/* allocate empty 3D texture */
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA,
		     BUF_WIDTH, BUF_HEIGHT, pot_depth,
		     0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	assert(glGetError() == 0);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	/* draw something into each slice of the 3D texture */
	for (depth = 0; depth < NUM_DEPTHS; depth++) {
		glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
					  GL_COLOR_ATTACHMENT0_EXT,
					  GL_TEXTURE_3D,
					  tex,
					  0,
					  depth);

		assert(glGetError() == 0);

		status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			fprintf(stderr, "FBO incomplete\n");
			goto done;
		}

		glViewport(0, 0, BUF_WIDTH, BUF_HEIGHT);
		piglit_ortho_projection(BUF_WIDTH, BUF_HEIGHT, GL_FALSE);

		/* solid color quad */
		glColor4fv(depth_color[depth]);
		piglit_draw_rect(-2, -2, BUF_WIDTH + 2, BUF_HEIGHT + 2);
	}


done:
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glDeleteFramebuffersEXT(1, &fb);

	return tex;
}

/* Draw a textured quad, sampling only the given depth/slice of the
 * 3D texture.
 */
static void
draw_depth(int x, int y, int depth)
{
	float depth_coord = (float)depth / (pot_depth - 1);

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	glEnable(GL_TEXTURE_3D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBegin(GL_QUADS);

	glTexCoord3f(0, 0, depth_coord);
	glVertex2f(x, y);

	glTexCoord3f(1, 0, depth_coord);
	glVertex2f(x + BUF_WIDTH, y);

	glTexCoord3f(1, 1, depth_coord);
	glVertex2f(x + BUF_WIDTH, y + BUF_HEIGHT);

	glTexCoord3f(0, 1, depth_coord);
	glVertex2f(x, y + BUF_HEIGHT);

	glEnd();
}

static GLboolean test_depth_drawing(int start_x, int start_y, float *expected)
{
	return piglit_probe_rect_rgb(start_x, start_y, BUF_WIDTH, BUF_HEIGHT,
				     expected);
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int depth;
	GLuint tex;

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_3d_fbo();

	for (depth = 0; depth < NUM_DEPTHS; depth++) {
		int x = 1 + depth * (BUF_WIDTH + 1);
		int y = 1;
		draw_depth(x, y, depth);
	}

	for (depth = 0; depth < NUM_DEPTHS; depth++) {
		int x = 1 + depth * (BUF_WIDTH + 1);
		int y = 1;
		pass &= test_depth_drawing(x, y, depth_color[depth]);
	}

	glDeleteTextures(1, &tex);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
}
