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

/** @file fbo-cubemap.c
 *
 * Tests that drawing to each face of a cube map FBO and then drawing views
 * of those faces to the window system framebuffer succeeds.
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

float face_color[7][4] = {
	{1.0, 0.0, 0.0, 0.0},
	{0.0, 1.0, 0.0, 0.0},
	{0.0, 0.0, 1.0, 0.0},
	{1.0, 0.0, 1.0, 0.0},
	{1.0, 1.0, 0.0, 0.0},
	{0.0, 1.0, 1.0, 0.0},
	{1.0, 1.0, 1.0, 0.0},
};

static float *get_face_color(int face, int level)
{
	return face_color[(face + 2 * level) % 7];
}

static int
create_cube_fbo(void)
{
	GLuint tex, fb;
	GLenum status;
	int face, dim;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

	for (face = 0; face < 6; face++) {
		int level = 0;

		for (dim = BUF_WIDTH; dim > 0; dim /= 2) {
			glTexImage2D(cube_face_targets[face], level, GL_RGBA,
				     dim, dim,
				     0,
				     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			level++;
		}
	}
	assert(glGetError() == 0);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	for (face = 0; face < 6; face++) {
		int level = 0;

		for (dim = BUF_WIDTH; dim > 0; dim /= 2) {
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
						  GL_COLOR_ATTACHMENT0_EXT,
						  cube_face_targets[face],
						  tex,
						  level);

			assert(glGetError() == 0);

			status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
			if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
				fprintf(stderr, "FBO incomplete\n");
				goto done;
			}

			glViewport(0, 0, dim, dim);
			piglit_ortho_projection(dim, dim, GL_FALSE);

			glColor4fv(get_face_color(face, level));
			/* Draw a little outside the bounds to make
			 * sure clipping's working.
			 */
			piglit_draw_rect(-2, -2, dim + 2, dim + 2);

			level++;
		}
	}


done:
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glDeleteFramebuffersEXT(1, &fb);

	return tex;
}

static void
draw_face(int x, int y, int dim, int face)
{
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	glEnable(GL_TEXTURE_CUBE_MAP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBegin(GL_QUADS);

	glTexCoord3fv(cube_face_texcoords[face][0]);
	glVertex2f(x, y);

	glTexCoord3fv(cube_face_texcoords[face][1]);
	glVertex2f(x + dim, y);

	glTexCoord3fv(cube_face_texcoords[face][2]);
	glVertex2f(x + dim, y + dim);

	glTexCoord3fv(cube_face_texcoords[face][3]);
	glVertex2f(x, y + dim);

	glEnd();

	glDisable(GL_TEXTURE_CUBE_MAP);
}

static GLboolean test_face_drawing(int start_x, int start_y, int dim,
				   float *expected)
{
	return piglit_probe_rect_rgb(start_x, start_y, dim, dim, expected);
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int face, dim;
	GLuint tex;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_cube_fbo();

	for (face = 0; face < 6; face++) {
		int x = 1 + face * (BUF_WIDTH + 1);
		int y = 1;

		for (dim = BUF_WIDTH; dim > 0; dim /= 2) {
			draw_face(x, y, dim, face);
			y += dim + 1;
		}
	}

	for (face = 0; face < 6; face++) {
		int x = 1 + face * (BUF_WIDTH + 1);
		int y = 1;
		int level = 0;

		for (dim = BUF_WIDTH; dim > 2; dim >>= 1) {
			pass &= test_face_drawing(x, y, dim,
						  get_face_color(face, level));
			y += dim + 1;
			level++;
		}
	}

	glDeleteTextures(1, &tex);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_ARB_texture_cube_map");
}
