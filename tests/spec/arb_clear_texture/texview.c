/*
 * Copyright 2015 Ilia Mirkin
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

/** @file texview.c
 *
 * A test to make sure that ARB_copy_image respects texture views on
 * both the source and destination ends.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
        config.supports_gl_compat_version = 13;
        config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

static const float green[4] = {0.0, 1.0, 0.0, 1.0};
static const float red[4] = {1.0, 0.0, 0.0, 1.0};

static bool
test_2d(void)
{
	bool pass = true;
	GLuint src;
	float red4[16];
	int i, j;

	memcpy(&red4[0], red, sizeof(red));
	memcpy(&red4[4], red, sizeof(red));
	memcpy(&red4[8], red, sizeof(red));
	memcpy(&red4[12], red, sizeof(red));

	glGenTextures(1, &src);

	glBindTexture(GL_TEXTURE_2D, src);
	glTexStorage2D(GL_TEXTURE_2D, 2, GL_RGBA8, 2, 2);

	for (i = 0; i < 2; i++) {
		GLuint view;

		/* reset src to red */
		glBindTexture(GL_TEXTURE_2D, src);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2,
				GL_RGBA, GL_FLOAT, red4);
		glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, 1, 1,
				GL_RGBA, GL_FLOAT, red);

		/* create a single-level view */
		glGenTextures(1, &view);
		glTextureView(view, GL_TEXTURE_2D, src,
			      GL_RGBA8, i, 1, 0, 1);

		glClearTexImage(view, 0, GL_RGBA, GL_FLOAT, green);

		/* check that the i'th level is cleared while others aren't */
		for (j = 0; j < 2; j++) {
			pass &= piglit_probe_texel_rect_rgba(
				GL_TEXTURE_2D, j,
				0, 0, 2 >> j, 2 >> j,
				j == i ? green : red);
		}
		glDeleteTextures(1, &view);
	}

	glDeleteTextures(1, &src);
	return pass;
}

static bool
test_2d_array(void)
{
	bool pass = true;
	GLuint src;
	float red2[8];
	int i, j;

	memcpy(&red2[0], red, sizeof(red));
	memcpy(&red2[4], red, sizeof(red));

	glGenTextures(1, &src);

	glBindTexture(GL_TEXTURE_2D_ARRAY, src);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 1, 1, 2);

	for (i = 0; i < 2; i++) {
		GLuint view;

		/* reset src to red */
		glBindTexture(GL_TEXTURE_2D_ARRAY, src);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
				1, 1, 2,
				GL_RGBA, GL_FLOAT, red2);

		/* create view of the layer1 */
		glGenTextures(1, &view);
		glTextureView(view, GL_TEXTURE_2D_ARRAY, src,
			      GL_RGBA8, 0, 1, i, 1);

		glClearTexImage(view, 0, GL_RGBA, GL_FLOAT, green);

		/* check that the i'th layer is cleared while others aren't */
		for (j = 0; j < 2; j++) {
			pass &= piglit_probe_texel_volume_rgba(
				GL_TEXTURE_2D_ARRAY, 0,
				0, 0, j, 1, 1, 1,
				j == i ? green : red);
		}
		glDeleteTextures(1, &view);
	}

	glDeleteTextures(1, &src);
	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	piglit_require_extension("GL_ARB_clear_texture");
	piglit_require_extension("GL_ARB_texture_view");
	piglit_require_extension("GL_ARB_texture_storage");

	pass &= test_2d();
	if (piglit_is_extension_supported("GL_EXT_texture_array"))
		pass &= test_2d_array();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
