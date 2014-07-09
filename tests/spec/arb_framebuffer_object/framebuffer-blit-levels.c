/*
 * Copyright © 2012 Intel Corporation
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

/** \file framebuffer-blit-levels.c
 *
 * This test verifies that glBlitFramebuffer operates correctly when
 * the read or draw framebuffer is bound to a nonzero miplevel of a
 * texture.
 *
 * The test can be run in two modes: "read" and "draw".  In "read"
 * mode, the layered/mipmapped texture is attached to
 * GL_READ_FRAMEBUFFER, and in "draw" mode, the layered/mipmapped
 * texture is attached to GL_DRAW_FRAMEBUFFER.
 *
 * The test operates as follows:
 *
 * - A 2D test texture is created with all miplevels present.  An
 *   auxiliary 2D texture is also created which has a single miplevel.
 *
 * - The test texture is populated with a deterministic pattern of
 *   data.  In "read" mode, this is done by simply uploading the data
 *   pattern using glTexImage2D.  In "draw" mode, this is done by
 *   first uploading the data pattern to the auxiliary texture, and
 *   then blitting it to the test texture (this checks that blits work
 *   properly when GL_DRAW_FRAMEBUFFER is the test texture).
 *
 * - The data in the test texture is then verified.  In "draw" mode,
 *   this is done by a direct call to glReadPixels().  In "read" mode,
 *   this is done by first blitting the data to the auxiliary texture,
 *   and then using glReadPixels() on the auxiliary texture (this
 *   checks that blits work properly when GL_READ_FRAMEBUFFER is the
 *   test texture).
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum {
	TEST_MODE_DRAW,
	TEST_MODE_READ,
} test_mode;

GLuint test_framebuffer;
GLuint aux_framebuffer;

GLuint test_texture;
GLuint aux_texture;

GLenum texture_internal_format;
GLenum texture_format;
GLenum readback_format;
GLenum texture_type;

GLbitfield blit_mask;
GLenum framebuffer_attachment;


#define LOG2_SIZE 7
#define SIZE (1 << LOG2_SIZE)
#define NUM_LEVELS (LOG2_SIZE + 1)

/**
 * Generate a block of test data in which each pixel has a unique RGBA
 * color.  Different values of the \c level parameter produce
 * different unique sets of pixels.
 *
 * This takes advantage of the Chinese Remainder Theorem to produce a
 * unique color for each pixel--we produce the R, G, B, and A values
 * by taking an integer mod four different primes.
 */
static void
create_test_data_rgba(GLfloat *data, unsigned level,
		      unsigned width, unsigned height)
{
	unsigned pixel;
	unsigned num_pixels = width * height;
	for (pixel = 0; pixel < num_pixels; ++pixel) {
		unsigned unique_value = level * (SIZE * SIZE) + pixel;
		data[4*pixel + 0] = (unique_value % 233) / 233.0;
		data[4*pixel + 1] = (unique_value % 239) / 239.0;
		data[4*pixel + 2] = (unique_value % 241) / 241.0;
		data[4*pixel + 3] = (unique_value % 251) / 251.0;
	}
}

/**
 * Generate a block of test data where each pixel has a unique depth value in
 * the range [0.0, 1.0).
 */
static void
create_test_data_depth(GLfloat *data, unsigned level,
		       unsigned width, unsigned height)
{
	unsigned pixel;
	unsigned num_pixels = width * height;
	double depth_delta = 0.95 / num_pixels;
	double depth_value = 0;

	for (pixel = 0; pixel < num_pixels; ++pixel) {
		data[pixel] = depth_value;
		depth_value += depth_delta;
	}
}

/**
 * Generate a block of test data appropriate for testing a stencil
 * buffer.  Different values of the \c level parameter produce
 * different unique sets of pixels.
 *
 * Since there are only 256 possible stencil values, we need to be
 * clever to generate a pattern that doesn't repeat every 256 pixels.
 * Here's how we do it: for each consecutive pair of values, we
 * compute x = (16451 * (i + 1)) % 65521 (where i is the index of the
 * pair, and starts at a different value for each miplevel).  Since
 * 16451 and 65521 are relatively prime, this pattern won't repeat for
 * 65521 pairs.  Then we set the first pixel in the pair to x / 256
 * and the second pixel in the pair to x % 256.
 */
static void
create_test_data_stencil(GLbyte *data, unsigned level,
			 unsigned width, unsigned height)
{
	unsigned pixel;
	unsigned num_pixels = width * height;
	for (pixel = 0; pixel < num_pixels; ++pixel) {
		unsigned i = (level * (SIZE * SIZE) + pixel) / 2;
		unsigned x = (16451 * (i + 1)) % 65521;
		if (pixel % 2 == 0)
			data[pixel] = x / 256;
		else
			data[pixel] = x % 256;
	}
}

/**
 * As with create_test_data_stencil(), but intersperse the stencil
 * data with depth values of 0, so that the resulting data can be
 * uploaded to a depth/stencil texture using glTexImage2D().
 */
static void
create_test_data_depthstencil(GLbyte *data, unsigned level,
			      unsigned width, unsigned height)
{
	GLbyte *stencil_data = malloc(width * height);
	unsigned i;
	create_test_data_stencil(stencil_data, level, width, height);

	for (i = 0; i < width * height; ++i) {
		data[4 * i] = stencil_data[i];
		data[4 * i + 1] = 0;
		data[4 * i + 2] = 0;
		data[4 * i + 3] = 0;
	}

	free(stencil_data);
}

static void
create_test_data(GLfloat *data, GLenum format,
		 unsigned level, unsigned width, unsigned height)
{
	switch (format) {
	case GL_RGBA:
		create_test_data_rgba(data, level, width, height);
		break;
	case GL_DEPTH_COMPONENT:
		create_test_data_depth(data, level, width, height);
		break;
	case GL_STENCIL_INDEX:
		create_test_data_stencil((GLbyte *) data, level,
					 width, height);
		break;
	case GL_DEPTH_STENCIL:
		create_test_data_depthstencil((GLbyte *) data, level,
					      width, height);
		break;
	default:
		assert(0);
		break;
	}
}

static void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <test_mode>\n"
	       "  where <test_mode> is one of:\n"
	       "    draw: test blitting *to* the given texture type\n"
	       "    read: test blitting *from* the given texture type\n"
	       "  where <format> is one of:\n"
	       "    rgba\n"
	       "    depth\n"
	       "    stencil\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	unsigned level;

	if (argc != 3) {
		print_usage_and_exit(argv[0]);
	}

	if (strcmp(argv[1], "draw") == 0) {
		test_mode = TEST_MODE_DRAW;
	} else if (strcmp(argv[1], "read") == 0) {
		test_mode = TEST_MODE_READ;
	} else {
		print_usage_and_exit(argv[0]);
	}

	if(strcmp(argv[2], "rgba") == 0) {
		texture_internal_format = GL_RGBA;
		texture_format = GL_RGBA;
		readback_format = GL_RGBA;
		texture_type = GL_FLOAT;
		framebuffer_attachment = GL_COLOR_ATTACHMENT0;
		blit_mask = GL_COLOR_BUFFER_BIT;
	} else if (strcmp(argv[2], "depth") == 0) {
		texture_internal_format = GL_DEPTH_COMPONENT;
		texture_format = GL_DEPTH_COMPONENT;
		readback_format = GL_DEPTH_COMPONENT;
		texture_type = GL_FLOAT;
		framebuffer_attachment = GL_DEPTH_ATTACHMENT;
		blit_mask = GL_DEPTH_BUFFER_BIT;
	} else if (strcmp(argv[2], "stencil") == 0) {
		texture_internal_format = GL_DEPTH_STENCIL;
		texture_format = GL_DEPTH_STENCIL;
		readback_format = GL_STENCIL_INDEX;
		texture_type = GL_UNSIGNED_INT_24_8;
		framebuffer_attachment = GL_DEPTH_STENCIL_ATTACHMENT;
		blit_mask = GL_STENCIL_BUFFER_BIT;
	} else {
		print_usage_and_exit(argv[0]);
	}

	piglit_require_extension("GL_ARB_framebuffer_object");

	/* Set up test framebuffer and test texture, but don't
	 * populate with data yet.
	 */
	glGenFramebuffers(1, &test_framebuffer);
	glGenTextures(1, &test_texture);
	glBindTexture(GL_TEXTURE_2D, test_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	for (level = 0; level < NUM_LEVELS; ++level) {
		glTexImage2D(GL_TEXTURE_2D, level,
			     texture_internal_format,
			     SIZE >> level, SIZE >> level,
			     0 /* border */,
			     texture_format,
			     texture_type,
			     NULL /* data */);
	}

	/* Set up aux framebuffer */
	glGenFramebuffers(1, &aux_framebuffer);
	glGenTextures(1, &aux_texture);
	glBindTexture(GL_TEXTURE_2D, aux_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0 /* level */,
		     texture_internal_format, SIZE, SIZE,
		     0 /* border */, texture_format,
		     texture_type, NULL /* data */);
	glBindFramebuffer(GL_FRAMEBUFFER, aux_framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, framebuffer_attachment,
			       GL_TEXTURE_2D, aux_texture, 0 /* level */);
}

/**
 * Upload test data to the given texture.
 *
 * \param data_level is the miplevel that the data is destined to
 * ultimately end up in--this influences the contents of the test
 * data.
 *
 * \param upload_level is the miplevel that the data should be
 * uploaded to.
 */
static void
upload_test_data(GLuint texture, unsigned data_level,
		 unsigned upload_level, unsigned width, unsigned height)
{
	GLfloat *data = malloc(SIZE * SIZE * 4 * sizeof(GLfloat));

	glBindTexture(GL_TEXTURE_2D, texture);

	create_test_data(data, texture_format, data_level, width, height);

	glTexImage2D(GL_TEXTURE_2D, upload_level, texture_internal_format,
		     width, height, 0 /* border */, texture_format,
		     texture_type, data);

	free(data);
}

static bool
test_image(unsigned width, unsigned height, const GLfloat *expected)
{
	if (readback_format == GL_STENCIL_INDEX) {
		return piglit_probe_image_stencil(0, 0, width, height,
						  (const GLubyte *) expected);
	} else {
		return piglit_probe_image_color(0, 0, width, height,
						readback_format, expected);
	}
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	GLfloat *data = malloc(SIZE * SIZE * 4 * sizeof(GLfloat));
	unsigned level;

	/* Populate the test texture */
	for (level = 0; level < NUM_LEVELS; ++level) {
		unsigned width = SIZE >> level;
		unsigned height = SIZE >> level;
		if (test_mode == TEST_MODE_READ) {
			/* Populate directly */
			upload_test_data(test_texture, level, level,
					 width, height);
		} else {
			/* Populate via aux texture */
			upload_test_data(aux_texture, level, 0,
					 width, height);
			glBindFramebuffer(GL_READ_FRAMEBUFFER,
					  aux_framebuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
					  test_framebuffer);
			glBindTexture(GL_TEXTURE_2D, test_texture);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
					       framebuffer_attachment,
					       GL_TEXTURE_2D,
					       test_texture,
					       level);
			glBlitFramebuffer(0, 0, width, height,
					  0, 0, width, height,
					  blit_mask, GL_NEAREST);
		}
	}

	/* Verify the test texture */
	for (level = 0; level < NUM_LEVELS; ++level) {
		unsigned width = SIZE >> level;
		unsigned height = SIZE >> level;
		printf("Testing level %d\n", level);
		create_test_data(data, readback_format, level, width, height);
		if (test_mode == TEST_MODE_DRAW) {
			/* Read texture data directly using glReadPixels() */
			glBindFramebuffer(GL_READ_FRAMEBUFFER, test_texture);
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER,
					       framebuffer_attachment,
					       GL_TEXTURE_2D,
					       test_texture,
					       level);
			pass = test_image(width, height, data) && pass;
		} else {
			/* Read via aux texture */
			glBindFramebuffer(GL_READ_FRAMEBUFFER,
					  test_framebuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
					  aux_framebuffer);
			glBindTexture(GL_TEXTURE_2D, test_texture);
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER,
					       framebuffer_attachment,
					       GL_TEXTURE_2D,
					       test_texture,
					       level);
			glBindTexture(GL_TEXTURE_2D, aux_texture);
			glTexImage2D(GL_TEXTURE_2D, 0 /* level */,
				     texture_internal_format, width, height,
				     0 /* border */, texture_format,
				     texture_type, NULL);
			glBlitFramebuffer(0, 0, width, height,
					  0, 0, width, height,
					  blit_mask, GL_NEAREST);
			glBindFramebuffer(GL_READ_FRAMEBUFFER,
					  aux_framebuffer);
			pass = test_image(width, height, data) && pass;
		}
	}

	free(data);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
