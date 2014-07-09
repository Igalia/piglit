/*
 * Copyright © 2012 Marek Olšák <maraeo@gmail.com>
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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
			       PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}

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
			printf("GetTexImage() returns incorrect data in byte %i for layer %i\n",
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

	switch (target) {
	case GL_TEXTURE_1D:
		glTexImage1D(GL_TEXTURE_1D, 0, internalformat, IMAGE_WIDTH, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, data);
		layer_size = IMAGE_WIDTH * 4;
		break;

	case GL_TEXTURE_2D:
	case GL_TEXTURE_RECTANGLE:
		glTexImage2D(target, 0, internalformat, IMAGE_WIDTH,
			     IMAGE_HEIGHT, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, data);
		layer_size = IMAGE_SIZE;
		break;

	case GL_TEXTURE_3D:
		num_layers = 16;
		glTexImage3D(GL_TEXTURE_3D, 0, internalformat,
			     IMAGE_WIDTH, IMAGE_HEIGHT, num_layers, 0, GL_RGBA,
			     GL_UNSIGNED_BYTE, data);
		layer_size = IMAGE_SIZE;
		break;

	case GL_TEXTURE_CUBE_MAP:
		num_faces = 6;
		for (i = 0; i < num_faces; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
				     internalformat, IMAGE_WIDTH, IMAGE_HEIGHT,
				     0, GL_RGBA,
				     GL_UNSIGNED_BYTE, data[i]);
		}
		target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
		layer_size = IMAGE_SIZE;
		break;

	case GL_TEXTURE_1D_ARRAY:
		num_layers = 7;
		glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, internalformat,
			     IMAGE_WIDTH, num_layers, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, data);
		// test as a single layer 2D image
		layer_size = IMAGE_WIDTH * 4 * num_layers;
		num_layers = 1;
		break;

	case GL_TEXTURE_2D_ARRAY:
		num_layers = 7;
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalformat,
			     IMAGE_WIDTH, IMAGE_HEIGHT, num_layers, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, data);
		layer_size = IMAGE_SIZE;
		break;

	case GL_TEXTURE_CUBE_MAP_ARRAY:
		num_layers = 6 * 3;
		glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, internalformat,
			     IMAGE_WIDTH, IMAGE_HEIGHT, num_layers, 0, GL_RGBA,
			     GL_UNSIGNED_BYTE, data);
		layer_size = IMAGE_SIZE;
		break;

	default:
		puts("Invalid texture target.");
		return false;

	}

	/* Setup the PBO or data array to read into from glGetTexImage */
	if (doPBO) {
		glGenBuffers(1, &packPBO);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, packPBO);
		glBufferData(GL_PIXEL_PACK_BUFFER,
				     layer_size * MAX2(num_faces,num_layers),
				     NULL, GL_STREAM_READ);
	} else {
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		memset(data2, 123, sizeof(data2));
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	assert(num_layers * num_faces * layer_size <= sizeof(data2));

	for (i = 0; i < num_faces; i++) {
		if (doPBO) {
			glGetTexImage(target + i, 0, GL_RGBA, GL_UNSIGNED_BYTE,
				      (GLvoid *) (long) (i * layer_size));
		} else {
			glGetTexImage(target + i, 0, GL_RGBA, GL_UNSIGNED_BYTE,
				      data2[i]);
		}
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	}
	if (doPBO)
		dataGet = (GLubyte *) glMapBufferRange(
					       GL_PIXEL_PACK_BUFFER, 0,
					       layer_size *
					       MAX2(num_faces,num_layers),
					       GL_MAP_READ_BIT);
	else
		dataGet = data2[0];

	for (i = 0; i < MAX2(num_faces,num_layers); i++) {
		pass = compare_layer(i, layer_size, tolerance, dataGet,
				     data[i]) && pass;
		dataGet += layer_size;
	}

	if (doPBO) {
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		glDeleteBuffers(1, &packPBO);
	}
	return pass;
}

void
piglit_init(int argc, char **argv)
{
	int i;
	GLenum target = GL_TEXTURE_2D;
	bool pass = true;
	GLenum internalformat = GL_RGBA8;
	GLubyte data[18][IMAGE_SIZE];
	int tolerance = 0;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "1D") == 0) {
			target = GL_TEXTURE_1D;
		}
		if (strcmp(argv[i], "3D") == 0) {
			target = GL_TEXTURE_3D;
			piglit_require_gl_version(12);
		}
		if (strcmp(argv[i], "RECT") == 0) {
			target = GL_TEXTURE_RECTANGLE;
			piglit_require_extension("GL_ARB_texture_rectangle");
		}
		if (strcmp(argv[i], "CUBE") == 0) {
			target = GL_TEXTURE_CUBE_MAP;
			piglit_require_extension("GL_ARB_texture_cube_map");
		}
		if (strcmp(argv[i], "1D_ARRAY") == 0) {
			target = GL_TEXTURE_1D_ARRAY;
			piglit_require_extension("GL_EXT_texture_array");
		}
		if (strcmp(argv[i], "2D_ARRAY") == 0) {
			target = GL_TEXTURE_2D_ARRAY;
			piglit_require_extension("GL_EXT_texture_array");
		}
		if (strcmp(argv[i], "CUBE_ARRAY") == 0) {
			target = GL_TEXTURE_CUBE_MAP_ARRAY;
			piglit_require_extension("GL_ARB_texture_cube_map_array");
		}
		if (strcmp(argv[i], "S3TC") == 0) {
			internalformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			tolerance = 8;
			piglit_require_extension("GL_EXT_texture_compression_s3tc");
			puts("Testing S3TC.");
		}
	}

	init_layer_data(data[0], 18);

	printf("Testing %s into PBO\n", piglit_get_gl_enum_name(target));
	pass = getTexImage(true, target, data, internalformat, tolerance) &&
				pass;

	printf("Testing %s into client array\n", piglit_get_gl_enum_name(target));
	pass = getTexImage(false, target, data, internalformat, tolerance) &&
				pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	if (pass)
		piglit_report_result(PIGLIT_PASS);
	else
		piglit_report_result(PIGLIT_FAIL);
}
