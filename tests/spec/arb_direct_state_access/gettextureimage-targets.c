/*
 * Copyright © 2012 Marek Olšák <maraeo@gmail.com>
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * @file gettextureimage-targets.c
 *
 * Adapted for testing glGetTextureImage in ARB_direct_state_access by
 * Laura Ekstrand <laura@jlekstrand.net>, November 2014.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
			       PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define IMAGE_WIDTH 32
#define IMAGE_HEIGHT 32
#define IMAGE_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT * 4)

static void
init_layer_data(GLubyte *layer_data, int num_layers)
{
	int x, y, z, i, j;

	for (z = 0; z < num_layers; z++) {
		GLubyte *data = layer_data + IMAGE_SIZE * z;

		for (x = 0; x < IMAGE_WIDTH; x += 4) {
			for (y = 0; y < IMAGE_HEIGHT; y += 4) {
				int r = (x + 1) * 255 / (IMAGE_WIDTH - 1);
				int g = (y + 1) * 255 / (IMAGE_HEIGHT - 1);
				int b = (z + 1) * 255 / (num_layers - 1);
				int a = x ^ y ^ z;

				/* each 4x4 block constains only one color (for S3TC) */
				for (i = 0; i < 4; i++) {
					for (j = 0; j < 4; j++) {
						data[((y + j) * IMAGE_WIDTH + x
						      + i) * 4 + 0] = r;
						data[((y + j) * IMAGE_WIDTH + x
						      + i) * 4 + 1] = g;
						data[((y + j) * IMAGE_WIDTH + x
						      + i) * 4 + 2] = b;
						data[((y + j) * IMAGE_WIDTH + x
						      + i) * 4 + 3] = a;
					}
				}
			}
		}
	}
}

static bool
compare_layer(int layer, int num_elements, int tolerance,
			  GLubyte *data, GLubyte *expected)
{
	int i;

	for (i = 0; i < num_elements; ++i) {
		if (abs((int)data[i] - (int)expected[i]) > tolerance) {
			printf("GetTextureImage() returns incorrect data in byte %i for layer %i\n",
			       i, layer);
			printf("    corresponding to (%i,%i), channel %i\n",
			       (i / 4) / IMAGE_WIDTH, (i / 4) % IMAGE_HEIGHT, i % 4);
			printf("    expected: %i\n", expected[i]);
			printf("    got: %i\n", data[i]);
			return false;
		}
	}
	return true;
}

static bool
getTexImage(bool doPBO, GLenum target, GLubyte data[][IMAGE_SIZE],
	    GLenum internalformat, int tolerance)
{
	int i;
	int num_layers=1, num_faces=1, layer_size;
	GLubyte data2[18][IMAGE_SIZE];
	GLubyte *dataGet;
	GLuint packPBO;
	bool pass = true;
	GLuint name;

	switch (target) {
	case GL_TEXTURE_1D:
		glCreateTextures(target, 1, &name);
		glTextureStorage1D(name, 1, internalformat, IMAGE_WIDTH);
		glTextureSubImage1D(name, 0, 0, IMAGE_WIDTH, GL_RGBA,
				    GL_UNSIGNED_BYTE, data);
		layer_size = IMAGE_WIDTH * 4;
		break;

	case GL_TEXTURE_2D:
	case GL_TEXTURE_RECTANGLE:
		glCreateTextures(target, 1, &name);
		glTextureStorage2D(name, 1, internalformat, IMAGE_WIDTH,
				   IMAGE_HEIGHT);
		glTextureSubImage2D(name, 0, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT,
				    GL_RGBA, GL_UNSIGNED_BYTE, data);
		layer_size = IMAGE_SIZE;
		break;

	case GL_TEXTURE_CUBE_MAP:
		num_faces = 6;
		glCreateTextures(target, 1, &name);
		/* This is invalid. You must use 2D storage call for cube. */
		glTextureStorage3D(name, 1, internalformat,
				   IMAGE_WIDTH, IMAGE_HEIGHT, num_faces);
		pass &= piglit_check_gl_error(GL_INVALID_ENUM);
		glTextureStorage2D(name, 1, internalformat,
				   IMAGE_WIDTH, IMAGE_HEIGHT);
		/* This is legal. */
		glTextureSubImage3D(name, 0, 0, 0, 0, IMAGE_WIDTH,
				    IMAGE_HEIGHT, num_faces, GL_RGBA,
				    GL_UNSIGNED_BYTE, data);
		layer_size = IMAGE_SIZE;
		break;

	case GL_TEXTURE_1D_ARRAY:
		num_layers = 7;
		glCreateTextures(target, 1, &name);
		// test as a single layer 2D image
		glTextureStorage2D(name, 1, internalformat, IMAGE_WIDTH,
				   num_layers);
		glTextureSubImage2D(name, 0, 0, 0, IMAGE_WIDTH, num_layers,
			            GL_RGBA, GL_UNSIGNED_BYTE, data);
		layer_size = IMAGE_WIDTH * 4 * num_layers;
		num_layers = 1;
		break;

	case GL_TEXTURE_3D:
		num_layers = 16; /* Fall through. */
	case GL_TEXTURE_2D_ARRAY:
		num_layers = 7; /* Fall through. */
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		num_layers = 6 * 3;
		glCreateTextures(target, 1, &name);
		glTextureStorage3D(name, 1, internalformat, IMAGE_WIDTH,
				   IMAGE_HEIGHT, num_layers);
		glTextureSubImage3D(name, 0, 0, 0, 0,
				    IMAGE_WIDTH, IMAGE_HEIGHT, num_layers,
				    GL_RGBA, GL_UNSIGNED_BYTE, data);
		layer_size = IMAGE_SIZE;
		break;

	default:
		puts("Invalid texture target.");
		return false;

	}

	/* Setup the PBO or data array to read into from glGetTextureImage */
	if (doPBO) {
		glGenBuffers(1, &packPBO);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, packPBO);
		glBufferData(GL_PIXEL_PACK_BUFFER,
				     layer_size * num_faces * num_layers,
				     NULL, GL_STREAM_READ);
	} else {
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		memset(data2, 123, sizeof(data2));
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	assert(num_layers * num_faces * layer_size <= sizeof(data2));

	if (doPBO) {
		glGetTextureImage(name, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			layer_size * num_faces * num_layers, NULL);
	}
	else {
		glGetTextureImage(name, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			layer_size * num_faces * num_layers, data2);
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	if (doPBO)
		dataGet = (GLubyte *) glMapBufferRange(
					       GL_PIXEL_PACK_BUFFER, 0,
					       layer_size * num_layers *
					       num_faces,
					       GL_MAP_READ_BIT);
	else
		dataGet = data2[0];

	for (i = 0; i < num_faces * num_layers; i++) {
		pass = compare_layer(i, layer_size, tolerance, dataGet,
				     data[i]) && pass;
		dataGet += layer_size;
	}

	if (doPBO) {
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		glDeleteBuffers(1, &packPBO);
	}

	glDeleteTextures(1, &name);

	return pass;
}

struct target_and_mask {
	GLenum target;
	bool mask;
};

static struct target_and_mask targets[] = {
	{GL_TEXTURE_1D, 1},
	{GL_TEXTURE_2D, 1},
	{GL_TEXTURE_3D, 1},
	{GL_TEXTURE_RECTANGLE, 1},
	{GL_TEXTURE_CUBE_MAP, 1},
	{GL_TEXTURE_1D_ARRAY, 1},
	{GL_TEXTURE_2D_ARRAY, 1},
	{GL_TEXTURE_CUBE_MAP_ARRAY, 1},
};

static void
clear_target_mask(GLenum target)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(targets); ++i) {
		if (targets[i].target == target) {
			targets[i].mask = 0;
		}
	}
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_direct_state_access");
	piglit_require_extension("GL_ARB_texture_storage");

	if (piglit_get_gl_version() < 12)
		clear_target_mask(GL_TEXTURE_3D);
	if (!piglit_is_extension_supported("GL_ARB_texture_rectangle"))
		clear_target_mask(GL_TEXTURE_RECTANGLE);
	if (!piglit_is_extension_supported("GL_ARB_texture_cube_map"))
		clear_target_mask(GL_TEXTURE_CUBE_MAP);
	if (!piglit_is_extension_supported("GL_EXT_texture_array")) {
		clear_target_mask(GL_TEXTURE_1D_ARRAY);
		clear_target_mask(GL_TEXTURE_2D_ARRAY);
	}
	if (!piglit_is_extension_supported("GL_ARB_texture_cube_map_array"))
		clear_target_mask(GL_TEXTURE_CUBE_MAP_ARRAY);
}

enum piglit_result
piglit_display(void)
{
	int i;
	bool pass = true;
	GLenum internalformat = GL_RGBA8;
	int tolerance = 0;
	GLubyte data[18][IMAGE_SIZE];

	init_layer_data(data[0], 18);

	for (i = 0; i < ARRAY_SIZE(targets); ++i) {
		if (!targets[i].mask)
			continue;

		printf("Testing %s into PBO\n", 
			piglit_get_gl_enum_name(targets[i].target));
		pass &= getTexImage(true, targets[i].target, data,
				    internalformat, tolerance);

		printf("Testing %s into client array\n",
			piglit_get_gl_enum_name(targets[i].target));
		pass &= getTexImage(false, targets[i].target, data, 
				    internalformat, tolerance);

		pass &= piglit_check_gl_error(GL_NO_ERROR);
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

