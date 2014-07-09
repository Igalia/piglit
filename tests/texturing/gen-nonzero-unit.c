/*
 * Copyright © 2008, 2009 Intel Corporation
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
 *    Ian Romanick <ian.d.romanick@intel.com>
 *
 */

/** @file gen-nonzero-unit.c
 *
 * Tests that:
 * - Only uses textures bound to texture unit 1.  This seems to be the source
 *   of bugzilla #24219.
 * - The full mipmap tree is generated when level 0 is set in a new
 *   texture object.
 * - Changing GL_GENERATE_MIPMAP state flushes previous vertices.
 * - The full mipmap tree is regenerated when level 0 is updated in an
 *   existing texture.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 512;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static PFNGLACTIVETEXTUREPROC ActiveTexture = NULL;

#define SIZE 128

static void display_mipmaps(int start_x, int start_y)
{
	int i;

	/* Disply all the mipmap levels */
	for (i = SIZE; i > 0; i /= 2) {
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(start_x + 0, start_y + 0);
		glTexCoord2f(1.0, 0.0); glVertex2f(start_x + i, start_y + 0);
		glTexCoord2f(1.0, 1.0); glVertex2f(start_x + i, start_y + i);
		glTexCoord2f(0.0, 1.0); glVertex2f(start_x + 0, start_y + i);
		glEnd();

		start_x += i;
	}
}

static void fill_level(int level, const GLfloat *color)
{
	GLfloat *data;
	int size = SIZE / (1 << level);
	int i;

	/* Update a square inside the texture to red */
	data = malloc(size * size * 4 * sizeof(GLfloat));
	for (i = 0; i < 4 * size * size; i += 4) {
		data[i + 0] = color[0];
		data[i + 1] = color[1];
		data[i + 2] = color[2];
		data[i + 3] = color[3];
	}
        glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, size, size, 0,
		     GL_RGBA, GL_FLOAT, data);
	free(data);
}

static GLboolean check_resulting_mipmaps(int x, int y, const GLfloat *color)
{
	GLboolean pass = GL_TRUE;
	int i;

	for (i = SIZE; i > 4; i /= 2) {
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
	const GLfloat red[4] = {1.0, 0.0, 0.0, 0.0};
	const GLfloat blue[4] = {0.0, 0.0, 1.0, 0.0};
	GLuint textures[3];
	int i;

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glGenTextures(3, textures);

	ActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SIZE, SIZE, 0,
		     GL_RGBA, GL_FLOAT, NULL);

	ActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);

	/* Set up a texture object with mipmap generation */
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	/* Set the first level of the new texture to red and display. */
	fill_level(0, red);
	display_mipmaps(0, 0);

	/* Set up texture object without mipmap generation */
	glBindTexture(GL_TEXTURE_2D, textures[2]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_NEAREST);

	/* Paint normal blue mipmap set */
	for (i = 0; SIZE / (1 << i) > 0; i++)
		fill_level(i, blue);

	display_mipmaps(0, SIZE);

	/* Enable GENERATE_MIPMAP and set the first (and thus all) levels to
	 * red.
	 */
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	fill_level(0, red);
	display_mipmaps(0, SIZE * 2);

	pass = pass && check_resulting_mipmaps(0, 0, red);
	pass = pass && check_resulting_mipmaps(0, SIZE, blue);
	pass = pass && check_resulting_mipmaps(0, SIZE * 2, red);

	piglit_present_results();

	glDeleteTextures(3, textures);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_SGIS_generate_mipmap");

	if (piglit_get_gl_version() >= 13) {
		ActiveTexture = glActiveTexture;
	} else {
		piglit_require_extension("GL_ARB_multitexture");
		ActiveTexture = glActiveTextureARB;
	}

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
