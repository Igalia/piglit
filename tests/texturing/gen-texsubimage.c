/*
 * Copyright © 2008 Intel Corporation
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
 *    Chris Lord <chris@openedhand.com>
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file gen-texsubimage.c
 *
 * Tests that the full mipmap tree is correctly updated after calling
 * glTexSubImage() when GL_GENERATE_MIPMAP is enabled.  Based on a test
 * in bug #17077.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 512;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static void display_mipmaps(int start_x, int start_y)
{
	int i;

	/* Disply all the mipmap levels */
	for (i = 256; i > 0; i /= 2) {
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(start_x + 0, start_y + 0);
		glTexCoord2f(1.0, 0.0); glVertex2f(start_x + i, start_y + 0);
		glTexCoord2f(1.0, 1.0); glVertex2f(start_x + i, start_y + i);
		glTexCoord2f(0.0, 1.0); glVertex2f(start_x + 0, start_y + i);
		glEnd();

		start_x += i;
	}
}

static GLboolean check_resulting_mipmaps(int x, int y, const GLfloat *color)
{
	GLboolean pass = GL_TRUE;
	int i;

	for (i = 256; i > 4; i /= 2) {
		pass = pass && piglit_probe_pixel_rgb(x + i / 2, y + i / 2,
						      color);
		x += i;
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLfloat *data;
	const GLfloat red[4] = {1.0, 0.0, 0.0, 0.0};
	const GLfloat blue[4] = {0.0, 0.0, 1.0, 0.0};
	GLuint texture;
	int i;

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Set up texture object with mipmap generation */
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	data = malloc(256 * 256 * 4 * sizeof(GLfloat));
	for (i = 0; i < 4 * 256 * 256; i += 4) {
		data[i + 0] = blue[0];
		data[i + 1] = blue[1];
		data[i + 2] = blue[2];
		data[i + 3] = blue[3];
	}

	/* Initialize the texture to blue */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0,
		     GL_RGBA, GL_FLOAT, data);

	free(data);
	/* Display the original mipmaps */
	display_mipmaps(0, 0);

	/* Update a square inside the texture to red */
	data = malloc(128 * 128 * 4 * sizeof(GLfloat));
	for (i = 0; i < 4 * 128 * 128; i += 4) {
		data[i + 0] = red[0];
		data[i + 1] = red[1];
		data[i + 2] = red[2];
		data[i + 3] = red[3];
	}
        glTexSubImage2D(GL_TEXTURE_2D, 0, 64, 64, 128, 128,
			GL_RGBA, GL_FLOAT, data);
	free(data);

	/* Display the mipmaps after subimage */
	display_mipmaps(0, 256);

	pass = pass && check_resulting_mipmaps(0, 0, blue);
	pass = pass && check_resulting_mipmaps(0, 256, red);

	piglit_present_results();

	glDeleteTextures(1, &texture);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_SGIS_generate_mipmap");

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glEnable(GL_TEXTURE_2D);
}
