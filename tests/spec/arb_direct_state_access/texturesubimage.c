/*
 * Copyright © 2011 VMware, Inc.
 * Copyright © 2014 Intel Corporation
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
 *
 */


/**
 * Test glTextureSubImage*D. This is pretty trivial, since it only uses
 * glTextureSubImage*D with offsets of 0 and the width, height, and depth of
 * the full image. Moreover, it doesn't test varying depths for the 3D case.
 * But since DSA functions share backends with the non-DSA ones, we really
 * only need to test entry points here.
 *
 * Laura Ekstrand
 * October 2014
 *
 * Based on texsubimage.c by
 * Brian Paul
 * October 2011
 *
 */


#include "piglit-util-gl.h"
#include <stdlib.h>
#include <string.h>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
			       PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

GLubyte*
random_image_data(int width, int height, int depth)
{
	int i;
	GLubyte *img = malloc(4 * width * height * depth * sizeof(GLubyte));
	for (i = 0; i < 4 * width * height * depth; ++i) {
		img[i] = rand() % 256;
	}
	return img;
} /* random_image_data */

static int depth = 4;
static GLubyte *refImg;

static enum piglit_result
subtest(GLenum target)
{
	bool pass = true;
	GLuint tex;

	/* Draw the reference image. */
	glCreateTextures(target, 1, &tex);
	glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (target == GL_TEXTURE_1D) {
		glTextureStorage1D(tex, 1, GL_RGBA8, piglit_width);
		glTextureSubImage1D(tex, 0, 0, piglit_width, GL_RGBA,
			GL_UNSIGNED_BYTE, refImg);
	}
	else if (target == GL_TEXTURE_2D) {
		glTextureStorage2D(tex, 1, GL_RGBA8, piglit_width,
				   piglit_height);
		glTextureSubImage2D(tex, 0, 0, 0, piglit_width, piglit_height,
				    GL_RGBA, GL_UNSIGNED_BYTE, refImg);
	}
	else if (target == GL_TEXTURE_3D) {
		glTextureStorage3D(tex, 1, GL_RGBA8, piglit_width,
				   piglit_height, depth);
		glTextureSubImage3D(tex, 0, 0, 0, 0,
				    piglit_width, piglit_height, 1,
				    GL_RGBA, GL_UNSIGNED_BYTE,
				    refImg);
	}

	glBindTextureUnit(0, tex);
	glEnable(target);
	glClear(GL_COLOR_BUFFER_BIT);
	piglit_draw_rect_tex(0, 0, piglit_width, piglit_height, 0, 0, 1, 1);
	if (target == GL_TEXTURE_1D) {
		pass &= piglit_probe_image_ubyte(0, 0, piglit_width, 1,
						 GL_RGBA, refImg);
	}
	else {
		pass &= piglit_probe_image_ubyte(0, 0, piglit_width,
						 piglit_height,
						 GL_RGBA, refImg);
	}

	if (!piglit_automatic)
		piglit_present_results();

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "%s", piglit_get_gl_enum_name(target));
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
piglit_display(void)
{
	static const GLenum targets[] = {
		GL_TEXTURE_1D,
		GL_TEXTURE_2D,
		GL_TEXTURE_3D
	};
	int i;
	enum piglit_result result = PIGLIT_PASS;
	enum piglit_result subtest_result;

	/* Loop over 1/2/3D texture targets */
	for (i = 0; i < ARRAY_SIZE(targets); i++) {
		subtest_result = subtest(targets[i]);
		if (subtest_result != PIGLIT_PASS)
			result = PIGLIT_FAIL;
	}

	return result;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_direct_state_access");
	piglit_require_extension("GL_ARB_texture_storage");

	srand(0);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* Make the image data for testing. */
	refImg = random_image_data(piglit_width, piglit_height, depth);
}
