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

/** @file streaming-texture-leak.c
 *
 * Tests that allocating and freeing textures over and over doesn't OOM
 * the system due to various refcounting issues drivers may have.
 *
 * Textures used are around 4MB, and we make 5k of them, so OOM-killer
 * should catch any failure.
 *
 * Bug #23530
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define TEX_SIZE 1024
static int tex_buffer[TEX_SIZE * TEX_SIZE];

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint texture;
	int i;
	float expected[4] = {0.0, 1.0, 0.0, 0.0};

	for (i = 0; i < 5000; i++) {
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_SIZE, TEX_SIZE,
			     0, GL_RGBA,
			     GL_UNSIGNED_BYTE, tex_buffer);

		piglit_draw_rect_tex(0, 0, piglit_width, piglit_height,
				     0, 0, 1, 1);

		glDeleteTextures(1, &texture);
	}

	pass = piglit_probe_pixel_rgb(piglit_width / 2, piglit_height / 2,
				      expected);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	int i;

	piglit_automatic = GL_TRUE;
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	for (i = 0; i < TEX_SIZE * TEX_SIZE; i++)
		tex_buffer[i] = 0x0000ff00;

	glEnable(GL_TEXTURE_2D);
}
