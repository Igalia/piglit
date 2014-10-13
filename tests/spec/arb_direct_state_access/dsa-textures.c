/*
 * Copyright 2014 Intel Corporation
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

/** @file dsa-textures.c
 *
 * Tests the direct state access functionality for creating, initializing, and
 * rendering texture objects.
 */
#include "piglit-util-gl.h"
#include "dsa-utils.h"

#include <stdlib.h>

static const char* glversion;
static bool nv340_23; /* Are we using the NVIDIA 340.23 driver? */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

GLfloat*
random_image_data(void)
{
	int i;
	GLfloat *img = malloc(4*piglit_width*piglit_height*sizeof(GLfloat));
	for (i = 0; i < 4*piglit_width*piglit_height; ++i) {
		img[i] = (float) rand() / RAND_MAX;
	}
	return img;
} /* random_image_data */

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_direct_state_access");
	piglit_require_extension("GL_ARB_texture_storage");

	srand(0);

	glversion = (const char*) glGetString(GL_VERSION);
	printf("Using driver %s.\n", glversion);
	if (strcmp("2.1.2 NVIDIA 340.23.03", glversion) == 0)
		nv340_23 = true;
	else
		nv340_23 = false;

	dsa_init_program();

} /* piglit_init */

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLfloat* data = random_image_data();
	GLuint name;
	int texunit = 3;

	glCreateTextures(GL_TEXTURE_2D, 1, &name);
	if (nv340_23) {
		printf("Working around bugs in NVIDIA 340.23.03.\n");
		glBindTexture(GL_TEXTURE_2D, name);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
			piglit_width, piglit_height, 0, GL_RGBA,
			GL_FLOAT, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	}
	else {
		/* These do not appear to work on nv340_23. */
		glTextureStorage2D(name, 1, GL_RGBA32F, piglit_width,
			piglit_height);
		glTextureSubImage2D(name, 0, 0, 0,
			piglit_width, piglit_height, GL_RGBA, GL_FLOAT, data);
		glTextureParameteri(name, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(name, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	/* Draw the image */
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	dsa_texture_with_unit(texunit);
	glEnable(GL_TEXTURE_2D);
	glBindTextureUnit(texunit, name);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	piglit_draw_rect_tex(0, 0, piglit_width, piglit_height, 0, 0, 1, 1);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* Check to make sure the image was drawn correctly */
	pass &= piglit_probe_image_rgba(0, 0, piglit_width, piglit_height,
		data);

	if (!piglit_automatic) {
		piglit_present_results();
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
} /* piglit_display */
