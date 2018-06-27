/* BEGIN_COPYRIGHT -*- glean -*-
 *
 * Copyright (C) 1999  Allen Akin   All Rights Reserved.
 * Copyright 2014 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * END_COPYRIGHT
 */

/** @file logicop.c
 *
 * Test RGBA logic op functions.
 *
 * Based on Allen's blendFunc test.
 * Brian Paul  10 May 2001
 * Adapted to Piglit by Laura Ekstrand <laura@jlekstrand.net>, November 2014.
 */

#include "piglit-util-gl.h"
#include <stdlib.h> /* for rand, srand */

#define drawing_size 64
#define img_width drawing_size
#define img_height drawing_size

static struct piglit_gl_test_config *piglit_config;

static enum piglit_result test_logicop(void * data);

#define TEST_ELEMENT(mode)              \
	{                               \
		#mode,                  \
		#mode,                  \
		test_logicop,           \
		(void *)(intptr_t)mode  \
	}
static struct piglit_subtest tests[] = {
	TEST_ELEMENT(GL_CLEAR),
	TEST_ELEMENT(GL_SET),
	TEST_ELEMENT(GL_COPY),
	TEST_ELEMENT(GL_COPY_INVERTED),
	TEST_ELEMENT(GL_NOOP),
	TEST_ELEMENT(GL_INVERT),
	TEST_ELEMENT(GL_AND),
	TEST_ELEMENT(GL_NAND),
	TEST_ELEMENT(GL_OR),
	TEST_ELEMENT(GL_NOR),
	TEST_ELEMENT(GL_XOR),
	TEST_ELEMENT(GL_EQUIV),
	TEST_ELEMENT(GL_AND_REVERSE),
	TEST_ELEMENT(GL_AND_INVERTED),
	TEST_ELEMENT(GL_OR_REVERSE),
	TEST_ELEMENT(GL_OR_INVERTED),
	{ 0 }
};
#undef TEST_ELEMENT

PIGLIT_GL_TEST_CONFIG_BEGIN

	piglit_config = &config;
	config.subtests = tests;
	config.supports_gl_compat_version = 11;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static GLubyte*
random_image_data(void)
{
	int i;
	GLubyte *img = malloc(4 * img_width * img_height * sizeof(GLubyte));
	for (i = 0; i < 4 * img_width * img_height; ++i) {
		img[i] = rand() % 256;
	}
	return img;
}

static GLubyte*
color_fill_data(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
	int i, j;
	GLubyte *img = malloc(4*img_width*img_height*sizeof(GLubyte));
	for (j = 0; j < img_height; ++j) { /* j = vertical, i = horizontal */
		for (i = 0; i < img_width; ++i) {
			img[4*(img_width*j + i) + 0] = r;
			img[4*(img_width*j + i) + 1] = g;
			img[4*(img_width*j + i) + 2] = b;
			img[4*(img_width*j + i) + 3] = a;
		}
	}
	return img;
}

static void
apply_logicop(GLenum logicop, GLubyte dst[4], const GLubyte src[4])
{
	switch (logicop) {
	case GL_CLEAR:
		dst[0] = dst[1] = dst[2] = dst[3] = 0;
		break;
	case GL_SET:
		dst[0] = dst[1] = dst[2] = dst[3] = ~0;
		break;
	case GL_COPY:
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3] = src[3];
		break;
	case GL_COPY_INVERTED:
		dst[0] = ~src[0];
		dst[1] = ~src[1];
		dst[2] = ~src[2];
		dst[3] = ~src[3];
		break;
	case GL_NOOP:
		break;
	case GL_INVERT:
		dst[0] = ~dst[0];
		dst[1] = ~dst[1];
		dst[2] = ~dst[2];
		dst[3] = ~dst[3];
		break;
	case GL_AND:
		dst[0] = src[0] & dst[0];
		dst[1] = src[1] & dst[1];
		dst[2] = src[2] & dst[2];
		dst[3] = src[3] & dst[3];
		break;
	case GL_NAND:
		dst[0] = ~(src[0] & dst[0]);
		dst[1] = ~(src[1] & dst[1]);
		dst[2] = ~(src[2] & dst[2]);
		dst[3] = ~(src[3] & dst[3]);
		break;
	case GL_OR:
		dst[0] = src[0] | dst[0];
		dst[1] = src[1] | dst[1];
		dst[2] = src[2] | dst[2];
		dst[3] = src[3] | dst[3];
		break;
	case GL_NOR:
		dst[0] = ~(src[0] | dst[0]);
		dst[1] = ~(src[1] | dst[1]);
		dst[2] = ~(src[2] | dst[2]);
		dst[3] = ~(src[3] | dst[3]);
		break;
	case GL_XOR:
		dst[0] = src[0] ^ dst[0];
		dst[1] = src[1] ^ dst[1];
		dst[2] = src[2] ^ dst[2];
		dst[3] = src[3] ^ dst[3];
		break;
	case GL_EQUIV:
		dst[0] = ~(src[0] ^ dst[0]);
		dst[1] = ~(src[1] ^ dst[1]);
		dst[2] = ~(src[2] ^ dst[2]);
		dst[3] = ~(src[3] ^ dst[3]);
		break;
	case GL_AND_REVERSE:
		dst[0] = src[0] & ~dst[0];
		dst[1] = src[1] & ~dst[1];
		dst[2] = src[2] & ~dst[2];
		dst[3] = src[3] & ~dst[3];
		break;
	case GL_AND_INVERTED:
		dst[0] = ~src[0] & dst[0];
		dst[1] = ~src[1] & dst[1];
		dst[2] = ~src[2] & dst[2];
		dst[3] = ~src[3] & dst[3];
		break;
	case GL_OR_REVERSE:
		dst[0] = src[0] | ~dst[0];
		dst[1] = src[1] | ~dst[1];
		dst[2] = src[2] | ~dst[2];
		dst[3] = src[3] | ~dst[3];
		break;
	case GL_OR_INVERTED:
		dst[0] = ~src[0] | dst[0];
		dst[1] = ~src[1] | dst[1];
		dst[2] = ~src[2] | dst[2];
		dst[3] = ~src[3] | dst[3];
		break;
	default:
		abort();  /* implementation error */
	}
}

static void
make_image(GLuint *name, GLubyte *data)
{
	glGenTextures(1, name);
	glBindTexture(GL_TEXTURE_2D, *name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_width, img_height, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, data);
}

static enum piglit_result
test_logicop(void * data)
{
	GLenum logicop = (GLenum)(intptr_t)data;
	bool pass = true;
	int x, y;
	GLuint dst_name;
	GLubyte* dst_data = random_image_data();
	GLuint src_name;
	GLubyte* src_data = random_image_data();
	GLubyte* exp_data = color_fill_data(0, 0, 0, 0);

	glDisable(GL_DITHER);
	glClearColor(0.5, 0.5, 0.5, 0.5);  /* transparent gray */
	glClear(GL_COLOR_BUFFER_BIT);

	/* Make our random texture */
	make_image(&dst_name, dst_data);

	/* Draw dst to the framebuffer */
	glDisable(GL_COLOR_LOGIC_OP);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, dst_name);
	piglit_draw_rect_tex(0, 0, img_width, img_height, 0, 0, 1, 1);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/*
	 * Read back the contents of the framebuffer, and measure any
	 * difference from what was actually written.  We can't tell
	 * whether errors occurred when writing or when reading back,
	 * but at least we can report anything unusual.
	 */
	pass &= piglit_probe_image_ubyte(0, 0, img_width, img_height, GL_RGBA,
					 dst_data);

	/*
	 * Now generate random source pixels and apply the logicop
	 * operation to both the framebuffer and a copy in the image
	 * ``expected''.  Save the source pixels in the image ``src''
	 * so we can diagnose any problems we find later.
	 *
	 * Note: src_data was generated above.
	 */
	glLogicOp(logicop);
	glEnable(GL_COLOR_LOGIC_OP);

	/* Make src */
	make_image(&src_name, src_data);
	glBindTexture(GL_TEXTURE_2D, src_name);
	piglit_draw_rect_tex(0, 0, img_width, img_height, 0, 0, 1, 1);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* Make exp */
	for (y = 0; y < drawing_size; ++y) {
		for (x = 0; x < drawing_size; ++x) {
			int idx = 4*(img_width*y + x);

			/* Initialize expected with dst data. */
			exp_data[idx + 0] = dst_data[idx + 0];
			exp_data[idx + 1] = dst_data[idx + 1];
			exp_data[idx + 2] = dst_data[idx + 2];
			exp_data[idx + 3] = dst_data[idx + 3];

			apply_logicop(logicop, exp_data + idx,
					       src_data + idx);
		}
	}

	/*
	 * Compare the image in the framebuffer to the
	 * computed image (``expected'') to see if any pixels are
	 * outside the expected tolerance range.
	 */
	pass &= piglit_probe_image_ubyte(0, 0, img_width, img_height, GL_RGBA,
					 exp_data);
	if (!piglit_automatic)
		piglit_present_results();

	free(exp_data);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;

} /* test_logicop */

enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_PASS;
	result = piglit_run_selected_subtests(
		tests,
		piglit_config->selected_subtests,
		piglit_config->num_selected_subtests,
		result);

	return result;
}

void
piglit_init(int argc, char **argv)
{
	srand(0);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
