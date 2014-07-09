/*
 * Copyright © 2010 Intel Corporation
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

/**
 * \file rg-teximage-02.c
 * Verify setting and getting image data for RED or RG formats
 *
 * Specify an RGBA image with a RG internal format.  Read the image back as
 * RGBA.  Verify the red and green components read back match the source image
 * and the blue and alpha components are 0 and 1, respectively.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl.h"
#include "rg-teximage-common.h"

#define WIDTH  256
#define HEIGHT 256
static float rgba_image[4 * WIDTH * HEIGHT];

static const GLenum internal_formats[] = {
	GL_RG,
	GL_RG8,
	GL_RG16,
	GL_RGBA,
};

GLuint tex[Elements(internal_formats)];
const unsigned num_tex = Elements(tex);
GLboolean pass = GL_TRUE;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = WIDTH*Elements(tex);
	config.window_height = HEIGHT;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	unsigned i;
	GLfloat *result;

	piglit_require_extension("GL_ARB_texture_rg");

	result = (GLfloat *) malloc(4 * WIDTH * HEIGHT * sizeof(GLfloat));
	generate_rainbow_texture_data(WIDTH, HEIGHT, rgba_image);

	glEnable(GL_TEXTURE_2D);
	glGenTextures(Elements(tex), tex);

	for (i = 0; i < Elements(internal_formats); i++) {
		GLenum err;

		glBindTexture(GL_TEXTURE_2D, tex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, internal_formats[i],
			     WIDTH, HEIGHT, 0, GL_RGBA,
			     GL_FLOAT, rgba_image);
		err = glGetError();
		if (err) {
			fprintf(stderr, "glTexImage2D(internalFormat = 0x%04x) "
				"generated GL error 0x%04x\n",
				internal_formats[i], err);
			pass = GL_FALSE;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);

		if (internal_formats[i] != GL_RGBA) {
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT,
				      result);
			pass = compare_texture(rgba_image, result,
					       internal_formats[i], GL_RGBA,
					       (WIDTH * HEIGHT), GL_TRUE)
				&& pass;
		}
	}

	free(result);

	if (piglit_automatic)
		piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
