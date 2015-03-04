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
 * @file getcompressedtextureimage-targets.c
 *
 * Adapted for testing glGetCompressedTextureImage in ARB_direct_state_access
 * by Laura Ekstrand <laura@jlekstrand.net>, November 2014.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.window_width = 216;
	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
			       PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define IMAGE_WIDTH 32
#define IMAGE_HEIGHT 32
#define IMAGE_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT * 4)
#define DISPLAY_GAP 4

static void
show_image(GLubyte *data, int num_layers, const char *title)
{
	GLuint name;
	int i;
	char junk[50];

	if (!piglit_automatic) {
		/* Create the texture handle. */
		glCreateTextures(GL_TEXTURE_2D, 1, &name);
		glTextureStorage2D(name, 1, GL_RGBA8, IMAGE_WIDTH,
			IMAGE_HEIGHT);
		glTextureParameteri(name, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(name, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glEnable(GL_TEXTURE_2D);
		glBindTextureUnit(0, name);

		/* Draw the layers, separated by some space */
		glClear(GL_COLOR_BUFFER_BIT);
		for (i = 0; i < num_layers; ++i) {
			int x = (IMAGE_WIDTH + DISPLAY_GAP) * (i % 6);
			int y = (IMAGE_HEIGHT + DISPLAY_GAP) * (i / 6);
			glTextureSubImage2D(name, 0, 0, 0,
					    IMAGE_WIDTH, IMAGE_HEIGHT,
					    GL_RGBA, GL_UNSIGNED_BYTE,
					    data + i * IMAGE_SIZE);
			piglit_draw_rect_tex(x, y, IMAGE_WIDTH, IMAGE_HEIGHT,
					     0, 0, 1, 1);
		}

		/* Make the title. */
		printf("****** %s ******\n", title);

		piglit_present_results();

		/* Pause. */
		printf("Enter any char to continue.\n>>>>>>");
		scanf("%s", junk);
		printf("\n");

		glDeleteTextures(1, &name);
	}
}

static GLubyte *
make_layer_data(int num_layers)
{
	int z;
	GLubyte *layer_data =
		malloc(num_layers * IMAGE_SIZE * sizeof(GLubyte));
	GLubyte *data = piglit_rgbw_image_ubyte(IMAGE_WIDTH,
						IMAGE_HEIGHT, true);

	for (z = 0; z < num_layers; z++) {
		memcpy(layer_data + IMAGE_SIZE * z, data, IMAGE_SIZE);
	}

	free(data);

	/* Show the first layer of the completed layer data. */
	show_image(layer_data, num_layers, "Test Data");

	return layer_data;
}

static bool
compare_layer(int layer, int num_elements, int tolerance,
			  GLubyte *data, GLubyte *expected)
{
	int i;

	for (i = 0; i < num_elements; ++i) {
		if (abs((int)data[i] - (int)expected[i]) > tolerance) {
			printf("GetCompressedTextureImage() returns incorrect"
			       " data in byte %i for layer %i\n",
			       i, layer);
			printf("    corresponding to (%i,%i), channel %i\n",
			       (i / 4) / IMAGE_WIDTH, (i / 4) % IMAGE_HEIGHT,
				i % 4);
			printf("    expected: %i\n", expected[i]);
			printf("    got: %i\n", data[i]);
			return false;
		}
	}
	return true;
}

static enum piglit_result
getTexImage(bool doPBO, GLenum target, GLubyte *data,
	    GLenum internalformat, int tolerance)
{
	int i;
	int num_layers=1, num_faces=1, layer_size;
	GLubyte *data2 = NULL;
	GLubyte *dataGet;
	GLuint packPBO;
	bool pass = true;
	GLuint name;
	GLint compressed;
	GLint comp_size;

	/* Upload the data. */
	switch (target) {

	/* These are all targets that can be compressed according to
	 * _mesa_target_can_be_compressed */

	case GL_TEXTURE_2D:
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
		return PIGLIT_FAIL;

	}

	/* Make sure the driver has compressed the image. */
	glGetTextureLevelParameteriv(name, 0, GL_TEXTURE_COMPRESSED,
		&compressed);
	printf("\tIs the texture compressed? %s.\n",
		compressed ? "yes" : "no");

	glGetTextureLevelParameteriv(name, 0,
				     GL_TEXTURE_COMPRESSED_IMAGE_SIZE,
				     &comp_size);
	/*  The OpenGL 4.5 core spec
	 *  (30.10.2014) Section 8.11 Texture Queries says:
	 *       "For GetTextureLevelParameter* only, texture may also be a
	 *       cube map texture object.  In this case the query is always
	 *       performed for face zero (the TEXTURE_CUBE_MAP_POSITIVE_X
	 *       face), since there is no way to specify another face."
	 */
	if (target == GL_TEXTURE_CUBE_MAP)
		comp_size *= num_faces;
	printf("\tThe size of the texture in bytes is %d.\n", comp_size);

	/* Show the uncompressed data. */
	show_image(data, num_layers * num_faces, "Data Before Compression");


	/* Setup the PBO or data array to read into from
	 * glGetCompressedTextureImage */
	if (doPBO) {
		glGenBuffers(1, &packPBO);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, packPBO);
		/* Make the buffer big enough to hold uncompressed data. */
		glBufferData(GL_PIXEL_PACK_BUFFER, layer_size * num_faces *
			     num_layers * sizeof(GLubyte),
			     NULL, GL_STREAM_READ);
	} else {
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		data2 = malloc(layer_size * num_faces * num_layers *
			       sizeof(GLubyte));
		memset(data2, 123, layer_size * num_faces * num_layers *
			       sizeof(GLubyte));
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	assert(num_layers * num_faces * layer_size <= 18 * IMAGE_SIZE);


	/* Download the compressed texture image. */
	if (doPBO)
		glGetCompressedTextureImage(name, 0, comp_size, NULL);
	else
		glGetCompressedTextureImage(name, 0, comp_size, data2);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	if (doPBO)
		dataGet = (GLubyte *) glMapBufferRange(
					       GL_PIXEL_PACK_BUFFER, 0,
					       comp_size,
					       GL_MAP_READ_BIT);
	else
		dataGet = data2;

	/* Re-upload the texture in compressed form. */
	switch (target) {
	case GL_TEXTURE_2D:
		glCompressedTextureSubImage2D(name, 0, 0, 0,
					      IMAGE_WIDTH, IMAGE_HEIGHT,
					      internalformat, comp_size,
					      dataGet);
		break;

	case GL_TEXTURE_CUBE_MAP:
		glCompressedTextureSubImage3D(name, 0, 0, 0, 0,
					      IMAGE_WIDTH, IMAGE_HEIGHT,
					      num_faces,
					      internalformat, comp_size,
					      dataGet);
		break;

	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		glCompressedTextureSubImage3D(name, 0, 0, 0, 0,
					      IMAGE_WIDTH, IMAGE_HEIGHT,
					      num_layers,
					      internalformat, comp_size,
					      dataGet);
		break;
	}


	/* Get the uncompressed version for comparison. */
	if (doPBO) {
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		glGetTextureImage(name, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			layer_size * num_layers * num_faces * sizeof(GLubyte),
			NULL);
	}
	else {
		glGetTextureImage(name, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			layer_size * num_layers * num_faces * sizeof(GLubyte),
			data2);
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	if (doPBO)
		dataGet = (GLubyte *) glMapBufferRange(
					       GL_PIXEL_PACK_BUFFER, 0,
					       layer_size * num_layers *
					       num_faces * sizeof(GLubyte),
					       GL_MAP_READ_BIT);
	else
		dataGet = data2;

	/* Examine the image after pulling it off the graphics card. */
	show_image(dataGet, num_layers * num_faces, "Data After Compression");

	/* Do the comparison */
	for (i = 0; i < num_faces * num_layers; i++) {
		pass = compare_layer(i, layer_size, tolerance, dataGet,
				     data + (i * layer_size)) && pass;
		dataGet += layer_size;
	}

	if (doPBO) {
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		glDeleteBuffers(1, &packPBO);
	}

	glDeleteTextures(1, &name);
	free(data2);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

struct target_and_mask {
	GLenum target;
	bool mask;
};

static struct target_and_mask targets[] = {
	{GL_TEXTURE_2D, 1},
	{GL_TEXTURE_CUBE_MAP, 1},
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

	if (!piglit_is_extension_supported("GL_ARB_texture_cube_map"))
		clear_target_mask(GL_TEXTURE_CUBE_MAP);
	if (!piglit_is_extension_supported("GL_EXT_texture_array")) {
		clear_target_mask(GL_TEXTURE_2D_ARRAY);
	}
	if (!piglit_is_extension_supported("GL_ARB_texture_cube_map_array"))
		clear_target_mask(GL_TEXTURE_CUBE_MAP_ARRAY);

	glClearColor(0.5, 0.5, 0.5, 1);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}

enum piglit_result
piglit_display(void)
{
	int i;
	GLenum internalformat = GL_COMPRESSED_RGBA_FXT1_3DFX;
	int tolerance = 8;
	GLubyte *data;
	enum piglit_result subtest;
	enum piglit_result result = PIGLIT_PASS;

	piglit_require_extension("GL_3DFX_texture_compression_FXT1");

	data = make_layer_data(18);

	for (i = 0; i < ARRAY_SIZE(targets); ++i) {
		if (!targets[i].mask)
			continue;

		printf("Testing %s into PBO\n",
			piglit_get_gl_enum_name(targets[i].target));
		subtest = getTexImage(true, targets[i].target, data,
				      internalformat, tolerance);
		piglit_report_subtest_result(subtest, "getTexImage %s PBO",
					     piglit_get_gl_enum_name(
						targets[i].target));
		if (subtest == PIGLIT_FAIL)
			result = PIGLIT_FAIL;

		printf("\n"); /* Separate tests with some white space. */

		printf("Testing %s into client array\n",
			piglit_get_gl_enum_name(targets[i].target));
		subtest = getTexImage(false, targets[i].target, data,
				      internalformat, tolerance);
		piglit_report_subtest_result(subtest, "getTexImage %s",
					     piglit_get_gl_enum_name(
						targets[i].target));
		if (subtest == PIGLIT_FAIL)
			result = PIGLIT_FAIL;

		printf("\n\n"); /* Separate targets with some white space. */

		if (!piglit_check_gl_error(GL_NO_ERROR))
			result = PIGLIT_FAIL;
	}

	/* 1D targets can't be compressed in Mesa right now,
	 * but here is a trivial test for the entry point. */
	glCompressedTextureSubImage1D(250, 0, 0, 60,
				      internalformat, 60*4*8,
				      NULL);

	if (!piglit_check_gl_error(GL_INVALID_OPERATION)) /* Bad texture */
		subtest = PIGLIT_FAIL;
	else
		subtest = PIGLIT_PASS;
	piglit_report_subtest_result(subtest, "Compressed Texture"
				     " Sub Image 1D");
	if (subtest == PIGLIT_FAIL)
		result = PIGLIT_FAIL;

	free(data);

	return result;
}

