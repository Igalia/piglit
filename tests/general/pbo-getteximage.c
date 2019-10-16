/*
 * Copyright © 2009 Intel Corporation
 * Copyright © 2019 Advanced Micro Devices, Inc.
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
 *    Eric Anholt <eric@anholt.net>
 *    Andrew Wesie <awesie@gmail.com>
 *    Pierre-Eric Pelloux-Prayer <pierre-eric.pelloux-prayer@amd.com>
 *
 */

/** @file pbo-getteximage.c
 *
 * Tests that using a PBO as the unpack buffer for glTexImage and
 * glTextureSubImage works correctly.
 * (pbo-teximage.c used as reference)
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static bool
probe(int x, int y, int z, float *expected, float *observed)
{
	if (expected[0] != observed[0] ||
	    expected[1] != observed[1] || expected[2] != observed[2]) {
		printf("Probe color at (%i,%i,%i)\n", x, y, z);
		printf("  Expected: b = %f  g = %f  r = %f  a = %f\n",
		       expected[0], expected[1], expected[2], expected[3]);
		printf("  Observed: b = %f  g = %f  r = %f  a = %f\n",
		       observed[0], observed[1], observed[2], observed[3]);

		return false;
	} else {
		return true;
	}
}

struct desc
{
	int x, y, z;
	int width, height, depth;
	float *pixels;
};

static bool
probe_all(struct desc *tex, struct desc *pbo)
{
	bool pass = true;
	for (int x = 0; x < pbo->width; ++x)
		for (int y = 0; y < pbo->height; ++y)
			for (int z = 0; z < pbo->depth; ++z) {
				int idx_in_tex = (x + tex->x) +
						 tex->width * (y + tex->y) +
						 tex->width * tex->height * (z + tex->z);
				int idx_in_pbo = x + pbo->width * y +
						 pbo->width * pbo->height * z;
				pass &= probe(x, y, z,
					      tex->pixels + 4 * idx_in_tex,
					      pbo->pixels + 4 * idx_in_pbo);
			}

	return pass;
}

static bool
test_getteximage(GLenum target, int width, int height, int depth,
		 float *pixels)
{
	bool pass;
	GLuint pbo, tex;

	glGenTextures(1, &tex);
	glBindTexture(target, tex);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (target == GL_TEXTURE_CUBE_MAP) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA,
			     width, height, 0, GL_RGBA, GL_FLOAT, pixels);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA,
			     width, height, 0, GL_RGBA, GL_FLOAT,
			     pixels + 4 * 2 * 2 * 1);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA,
			     width, height, 0, GL_RGBA, GL_FLOAT,
			     pixels + 4 * 2 * 2 * 2);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA,
			     width, height, 0, GL_RGBA, GL_FLOAT,
			     pixels + 4 * 2 * 2 * 3);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA,
			     width, height, 0, GL_RGBA, GL_FLOAT,
			     pixels + 4 * 2 * 2 * 4);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA,
			     width, height, 0, GL_RGBA, GL_FLOAT,
			     pixels + 4 * 2 * 2 * 5);
		depth = 6;
	} else if (depth) {
		glTexImage3D(target, 0, GL_RGBA, width, height, depth, 0,
			     GL_RGBA, GL_FLOAT, pixels);
	} else if (height) {
		glTexImage2D(target, 0, GL_RGBA, width, height, 0, GL_RGBA,
			     GL_FLOAT, pixels);
		depth = 1;
	} else {
		glTexImage1D(target, 0, GL_RGBA, width, 0, GL_RGBA, GL_FLOAT,
			     pixels);
		height = 1;
		depth = 1;
	}

	glGenBuffersARB(1, &pbo);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER, pbo);
	glBufferDataARB(GL_PIXEL_PACK_BUFFER,
			4 * width * height * depth * sizeof(float), NULL,
			GL_STREAM_DRAW_ARB);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	struct desc tex_info = {
		0, 0, 0,
		width, height, depth,
		pixels
	};

	struct desc pbo_info = {
		0, 0, 0,
		width, height, depth,
	};

	if (target == GL_TEXTURE_CUBE_MAP) {
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA,
			      GL_FLOAT, BUFFER_OFFSET(0));
		glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA,
			      GL_FLOAT,
			      BUFFER_OFFSET(sizeof(float) * 4 * 2 * 2 * 1));
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA,
			      GL_FLOAT,
			      BUFFER_OFFSET(sizeof(float) * 4 * 2 * 2 * 2));
		glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA,
			      GL_FLOAT,
			      BUFFER_OFFSET(sizeof(float) * 4 * 2 * 2 * 3));
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA,
			      GL_FLOAT,
			      BUFFER_OFFSET(sizeof(float) * 4 * 2 * 2 * 4));
		glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA,
			      GL_FLOAT,
			      BUFFER_OFFSET(sizeof(float) * 4 * 2 * 2 * 5));

		pbo_info.pixels = glMapBufferARB(GL_PIXEL_PACK_BUFFER,
						 GL_READ_ONLY_ARB);
		pass = probe_all(&tex_info, &pbo_info);
		glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);
	} else {
		glGetTexImage(target, 0, GL_RGBA, GL_FLOAT, 0);

		pbo_info.pixels = glMapBufferARB(GL_PIXEL_PACK_BUFFER,
						 GL_READ_ONLY_ARB);
		pass = probe_all(&tex_info, &pbo_info);
		glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);
	}

	glBindBufferARB(GL_PIXEL_PACK_BUFFER, 0);

	glDeleteBuffersARB(1, &pbo);
	glDeleteTextures(1, &tex);
	return pass;
}

static bool
test_gettexturesubimage(GLenum target, int width, int height, int depth,
			float *pixels)
{
	int xoffset, yoffset, zoffset;
	bool pass;
	GLuint pbo, tex;

	glGenTextures(1, &tex);
	glBindTexture(target, tex);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (target == GL_TEXTURE_CUBE_MAP) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA,
			     width, height, 0, GL_RGBA, GL_FLOAT, pixels);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA,
			     width, height, 0, GL_RGBA, GL_FLOAT,
			     pixels + 4 * 2 * 2 * 1);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA,
			     width, height, 0, GL_RGBA, GL_FLOAT,
			     pixels + 4 * 2 * 2 * 2);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA,
			     width, height, 0, GL_RGBA, GL_FLOAT,
			     pixels + 4 * 2 * 2 * 3);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA,
			     width, height, 0, GL_RGBA, GL_FLOAT,
			     pixels + 4 * 2 * 2 * 4);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA,
			     width, height, 0, GL_RGBA, GL_FLOAT,
			     pixels + 4 * 2 * 2 * 5);
		depth = 6;
	} else if (depth) {
		glTexImage3D(target, 0, GL_RGBA, width, height, depth, 0,
			     GL_RGBA, GL_FLOAT, pixels);
	} else if (height) {
		glTexImage2D(target, 0, GL_RGBA, width, height, 0, GL_RGBA,
			     GL_FLOAT, pixels);
		depth = 1;
	} else {
		glTexImage1D(target, 0, GL_RGBA, width, 0, GL_RGBA, GL_FLOAT,
			     pixels);
		height = 1;
		depth = 1;
	}

	xoffset = 1 % width;
	yoffset = 1 % height;
	zoffset = 1 % depth;

	struct desc tex_info = {
		xoffset, yoffset, zoffset,
		width, height, depth,
		pixels,
	};

	struct desc pbo_info = {
		0, 0, 0,
		width - xoffset,
		height - yoffset,
		depth - zoffset,
	};

	glGenBuffersARB(1, &pbo);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER, pbo);
	glBufferDataARB(GL_PIXEL_PACK_BUFFER,
			4 * width * height * depth * sizeof(float), NULL,
			GL_STREAM_DRAW_ARB);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glGetTextureSubImage(tex, 0, xoffset, yoffset, zoffset,
			     pbo_info.width, pbo_info.height, pbo_info.depth,
			     GL_RGBA, GL_FLOAT,
			     4 * width * height * depth * sizeof(float), 0);

	pbo_info.pixels = glMapBufferARB(GL_PIXEL_PACK_BUFFER,
					 GL_READ_ONLY_ARB);
	pass = probe_all(&tex_info, &pbo_info);
	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);

	glBindBufferARB(GL_PIXEL_PACK_BUFFER, 0);

	glDeleteBuffersARB(1, &pbo);
	glDeleteTextures(1, &tex);
	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	float pixels[4 * (2 * 2 * 12)];
	unsigned int x, y, z;

	piglit_require_extension("GL_ARB_pixel_buffer_object");
	piglit_require_extension("GL_ARB_get_texture_sub_image");

	/* Init pixels with content */
	for (x = 0; x < 2; ++x)
		for (y = 0; y < 2; ++y)
			for (z = 0; z < 12; ++z) {
				pixels[4 * (x + 2 * y + 2 * 2 * z)] = x % 2;
				pixels[4 * (x + 2 * y + 2 * 2 * z) + 1] =
					y % 2;
				pixels[4 * (x + 2 * y + 2 * 2 * z) + 2] =
					z / 6;
				pixels[4 * (x + 2 * y + 2 * 2 * z) + 3] =
					0.0f;
			}

	pass &= test_getteximage(GL_TEXTURE_1D, 2, 0, 0, pixels);
	pass &= test_getteximage(GL_TEXTURE_1D_ARRAY, 2, 2, 0, pixels);
	pass &= test_getteximage(GL_TEXTURE_2D, 2, 2, 0, pixels);
	pass &= test_getteximage(GL_TEXTURE_2D_ARRAY, 2, 2, 2, pixels);
	pass &= test_getteximage(GL_TEXTURE_3D, 2, 2, 2, pixels);
	pass &= test_getteximage(GL_TEXTURE_CUBE_MAP, 2, 2, 0, pixels);
	if (piglit_is_extension_supported("GL_ARB_texture_cube_map_array"))
		pass &= test_getteximage(GL_TEXTURE_CUBE_MAP_ARRAY, 2, 2, 12, pixels);

	pass &= test_gettexturesubimage(GL_TEXTURE_1D, 2, 0, 0, pixels);
	pass &= test_gettexturesubimage(GL_TEXTURE_1D_ARRAY, 2, 2, 0, pixels);
	pass &= test_gettexturesubimage(GL_TEXTURE_2D, 2, 2, 0, pixels);
	pass &= test_gettexturesubimage(GL_TEXTURE_2D_ARRAY, 2, 2, 2, pixels);
	pass &= test_gettexturesubimage(GL_TEXTURE_3D, 2, 2, 2, pixels);
	pass &= test_gettexturesubimage(GL_TEXTURE_CUBE_MAP, 2, 2, 0, pixels);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}