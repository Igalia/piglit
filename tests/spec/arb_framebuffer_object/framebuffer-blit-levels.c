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

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_MAIN(
    32,
    32,
    PIGLIT_GL_VISUAL_RGBA);

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

static void
create_test_data(GLfloat *data, GLenum texture_format,
		 unsigned level, unsigned width, unsigned height)
{
	if (texture_format == GL_RGBA)
		create_test_data_rgba(data, level, width, height);
	else if (texture_format == GL_DEPTH_COMPONENT)
		create_test_data_depth(data, level, width, height);
	else
		assert(0);
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
	       "    depth\n",
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
		texture_type = GL_FLOAT;
		framebuffer_attachment = GL_COLOR_ATTACHMENT0;
		blit_mask = GL_COLOR_BUFFER_BIT;
	} else if (strcmp(argv[2], "depth") == 0) {
		texture_internal_format = GL_DEPTH_COMPONENT;
		texture_format = GL_DEPTH_COMPONENT;
		texture_type = GL_FLOAT;
		framebuffer_attachment = GL_DEPTH_ATTACHMENT;
		blit_mask = GL_DEPTH_BUFFER_BIT;
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
		create_test_data(data, texture_format, level, width, height);
		if (test_mode == TEST_MODE_READ) {
			/* Populate directly */
			glBindTexture(GL_TEXTURE_2D, test_texture);
			glTexImage2D(GL_TEXTURE_2D, level,
				     texture_internal_format, width, height,
				     0 /* border */, texture_format,
				     texture_type, data);
		} else {
			/* Populate via aux texture */
			glBindFramebuffer(GL_READ_FRAMEBUFFER,
					  aux_framebuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
					  test_framebuffer);
			glBindTexture(GL_TEXTURE_2D, aux_texture);
			glTexImage2D(GL_TEXTURE_2D, 0 /* level */,
				     texture_internal_format, width, height,
				     0 /* border */, texture_format,
				     texture_type, data);
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
		create_test_data(data, texture_format, level, width, height);
		if (test_mode == TEST_MODE_DRAW) {
			/* Read texture data directly using glReadPixels() */
			glBindFramebuffer(GL_READ_FRAMEBUFFER, test_texture);
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER,
					       framebuffer_attachment,
					       GL_TEXTURE_2D,
					       test_texture,
					       level);
			pass = piglit_probe_image_color(0, 0, width, height,
							texture_format,
							data) && pass;
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
			pass = piglit_probe_image_color(0, 0, width, height,
							texture_format,
							data) && pass;
		}
	}

	free(data);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
