/*
 * Copyright © 2010 Red Hat
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
 *    Dave Airlie
 *
 */

/** @file tex-srgb
 *
 * Check srgb texturing and EXT_texture_sRGB_decode
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define SIZE 128

// Convert an 8-bit sRGB value from non-linear space to a
// linear RGB value in [0, 1].
// Implemented with a 256-entry lookup table.
static float
nonlinear_to_linear(GLubyte cs8)
{
	static GLfloat table[256];
	static GLboolean tableReady = GL_FALSE;
	if (!tableReady) {
		// compute lookup table now
		GLuint i;
		for (i = 0; i < 256; i++) {
			const GLfloat cs = i / 255.0;
			if (cs <= 0.04045) {
				table[i] = cs / 12.92;
			}
			else {
				table[i] = piglit_srgb_to_linear(cs);
			}
		}
		tableReady = GL_TRUE;
	}
	return table[cs8];
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
        glTexImage2D(GL_TEXTURE_2D, level, GL_SRGB8_ALPHA8, size, size, 0,
                     GL_RGBA, GL_FLOAT, data);
        free(data);
}

static GLboolean
srgb_tex_test(int srgb_format)
{
	GLboolean pass = GL_TRUE;
	float green[] = {0, 0.3, 0.0, 0};
	float expected_green[4];
	float expected_srgb_green[4];
	GLuint tex;
	GLboolean have_decode;

	have_decode = piglit_is_extension_supported("GL_EXT_texture_sRGB_decode");

	glGenTextures(1, &tex);

	glBindTexture(GL_TEXTURE_2D, tex);

	fill_level(0, green);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	piglit_draw_rect_tex(0, 0, 20, 20, 0, 0, 1, 1);

	if (have_decode) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SRGB_DECODE_EXT,
				GL_SKIP_DECODE_EXT);

		piglit_draw_rect_tex(20, 0, 20, 20, 0, 0, 1, 1);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SRGB_DECODE_EXT,
				GL_DECODE_EXT);

		piglit_draw_rect_tex(40, 0, 20, 20, 0, 0, 1, 1);
	}

	memcpy(expected_green, green, sizeof(float) * 4);
	memcpy(expected_srgb_green, green, sizeof(float) * 4);
	expected_srgb_green[1] = nonlinear_to_linear(255.0*green[1]);

	if (!piglit_probe_rect_rgb(0, 0, 20, 20, expected_srgb_green))
		pass = GL_FALSE;

	if (have_decode) {

		if (!piglit_probe_rect_rgb(20, 0, 20, 20, expected_green))
			pass = GL_FALSE;

		if (!piglit_probe_rect_rgb(40, 0, 20, 20, expected_srgb_green))
			pass = GL_FALSE;
	}


	glDeleteTextures(1, &tex);
	piglit_present_results();

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;

	pass = srgb_tex_test(0);
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


static void reshape(int width, int height)
{
	piglit_width = width;
	piglit_height = height;

	piglit_ortho_projection(width, height, GL_FALSE);
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_texture_sRGB");
	reshape(piglit_width, piglit_height);
}
