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
 */

/** @file fbo-generatemipmap.c
 *
 * Tests that glGenerateMipmapEXT uses appropriate filtering for a 2D texture.
 */

#include "piglit-util.h"

#define TEX_WIDTH 256
#define TEX_HEIGHT 256
int piglit_width = 700;
int piglit_height = 300;
int piglit_window_mode = GLUT_DOUBLE | GLUT_RGB;

static int
create_tex(void)
{
	GLuint tex;
	uint8_t data[TEX_WIDTH * TEX_HEIGHT * 4];
	int x, y;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	for (y = 0; y < TEX_HEIGHT; y++) {
		for (x = 0; x < TEX_WIDTH; x++) {
			uint8_t *p = data + (y * TEX_WIDTH + x) * 4;

			if ((x + 1) % 8 < 4) {
				p[0] = 255;
				p[1] = 0;
			} else {
				p[0] = 0;
				p[1] = 255;
			}
			p[2] = 0;
			p[3] = 255;
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		     TEX_WIDTH, TEX_HEIGHT,
		     0,
		     GL_RGBA, GL_UNSIGNED_BYTE, data);

	/* Leave the worst possible filtering setup in place for calling
	 * glGenerateMipmap.
	 */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenerateMipmapEXT(GL_TEXTURE_2D);

	return tex;
}

static void
draw_mipmap(int x, int y, int dim)
{
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	piglit_draw_rect_tex(x, y, dim, dim,
			     0, 0, 1, 1);
	glDisable(GL_TEXTURE_2D);
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int dim;
	GLuint tex;
	int x;
	float blend[] = {0.5, 0.5, 0.0, 1.0};

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_tex();

	x = 1;
	for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
		draw_mipmap(x, 1, dim);
		x += dim + 1;
	}

	x = 1;
	for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
		if (dim < TEX_WIDTH / 4) {
			pass &= piglit_probe_rect_rgba(x, 1, dim, dim,
						       blend);
		}

		x += dim + 1;
	}

	glDeleteTextures(1, &tex);

	glutSwapBuffers();

	return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
}
