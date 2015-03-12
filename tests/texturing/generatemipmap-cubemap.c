/*
 * Copyright 2015 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file generatemipmap-cubemap.c
 *
 * Test to make sure that glGenerateMipmap(GL_TEXTURE_CUBE_MAP) works
 * correctly when the cube map texture is generated using glTexImage and not
 * using glTexStorage and glTexSubImage.
 *
 * Attempts to reproduce Mesa Bug 89526.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	/* glGenerateMipmap was introduced in OpenGL 3.0. */
	config.supports_gl_compat_version = 30;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define WIDTH 32
#define HEIGHT 32
#define IMAGE_SIZE (WIDTH * HEIGHT * 4)

static GLubyte *expected;

static void
init_random_data(void)
{
	int i;

	expected = malloc(6 * IMAGE_SIZE);
	for (i = 0; i < 6 * IMAGE_SIZE; ++i) {
		expected[i] = (GLubyte) rand();
	}
}

void
piglit_init(int argc, char **argv)
{
	srand(0);
	init_random_data();
}

enum piglit_result
piglit_display(void)
{
	GLuint tex;
	int i;
	bool pass = true;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

	for (i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			     0, GL_RGBA8, WIDTH, HEIGHT, 0, GL_RGBA,
			     GL_UNSIGNED_BYTE, expected + i * IMAGE_SIZE);
	}

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	free(expected);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
