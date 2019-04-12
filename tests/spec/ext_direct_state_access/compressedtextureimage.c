/*
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
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

/* Compressed formats from arb_direct_state_access/compressedtextureimage.c */
struct format {
	GLenum token;
	const char **extension;
};

static const struct format *format;

static const char *FXT1[] = {
	"GL_3DFX_texture_compression_FXT1",
	NULL
};

static const char *S3TC[] = {
	"GL_EXT_texture_compression_s3tc",
	NULL
};

static const char *S3TC_srgb[] = {
	"GL_EXT_texture_compression_s3tc",
	"GL_EXT_texture_sRGB",
	NULL
};

static const char *RGTC[] = {
	"GL_ARB_texture_compression_rgtc",
	NULL
};

static const char *RGTC_signed[] = {
	"GL_ARB_texture_compression_rgtc",
	"GL_EXT_texture_snorm",
	NULL
};

static const char *BPTC[] = {
	"GL_ARB_texture_compression_bptc",
	NULL
};

static const struct format formats[] = {
	{ GL_COMPRESSED_RGB_FXT1_3DFX, FXT1 },
	{ GL_COMPRESSED_RGBA_FXT1_3DFX, FXT1 },

	{ GL_COMPRESSED_RGB_S3TC_DXT1_EXT, S3TC },
	{ GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, S3TC },
	{ GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, S3TC },
	{ GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, S3TC },

	{ GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, S3TC_srgb },
	{ GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, S3TC_srgb },
	{ GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, S3TC_srgb },
	{ GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, S3TC_srgb },

	{ GL_COMPRESSED_RGBA_BPTC_UNORM, BPTC },
	{ GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM, BPTC },
	{ GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT, BPTC },
	{ GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT, BPTC },

	{ GL_COMPRESSED_RED_RGTC1_EXT, RGTC },
	{ GL_COMPRESSED_SIGNED_RED_RGTC1_EXT, RGTC_signed },
	{ GL_COMPRESSED_RED_GREEN_RGTC2_EXT, RGTC },
	{ GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT, RGTC_signed },
};

static void
usage(int argc, char **argv)
{
	int i;

	fprintf(stderr, "Usage: %s <format>\n", argv[0]);
	fprintf(stderr, "format is one of:\n");
	for (i = 0; i < ARRAY_SIZE(formats); i++) {
		fprintf(stderr, "  %s\n",
			piglit_get_gl_enum_name(formats[i].token));
	}
	exit(1);
}

void
piglit_init(int argc, char **argv)
{
	int i;
	GLenum arg;
	if (argc != 2)
		usage(argc, argv);

	format = NULL;

	arg = piglit_get_gl_enum_from_name(argv[1]);
	for (i = 0; i < ARRAY_SIZE(formats); i++) {
		if (formats[i].token == arg) {
			format = &formats[i];
			break;
		}
	}

	if (!format)
		usage(argc, argv);

	for (i = 0; format->extension[i]; i++)
		piglit_require_extension(format->extension[i]);

	if (format->token == GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT)
		piglit_set_tolerance_for_bits(7, 7, 7, 7);

	piglit_require_extension("GL_EXT_direct_state_access");
}

static GLenum
dimension_to_target(int n)
{
	assert(n == 1 || n == 2 || n == 3);
	switch (n) {
		case 1: return GL_TEXTURE_1D;
		case 2: return GL_TEXTURE_2D;
		case 3:
		default:
			return GL_TEXTURE_3D;
	}
}

static GLenum use_display_list = GL_NONE;
static GLuint list;

static GLuint
init_compressed_texture(GLenum target, int* compressed_size, void** compressed,
			float** expected_pixels)
{
	int i;
	GLuint tex;
	const int height = (target == GL_TEXTURE_1D) ? 1 : piglit_height;
	const int depth = (target == GL_TEXTURE_3D) ? 2 : 1;
	float* image = piglit_rgbw_image(GL_RGBA, piglit_width, height * depth,
					 false, GL_UNSIGNED_NORMALIZED);

	if (!expected_pixels) {
		for (i = 0; i < piglit_width * height * depth * 4; i++) {
			image[i] = (float) rand() / RAND_MAX;
		}
	}

	glGenTextures(1, &tex);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	if (target == GL_TEXTURE_1D) {
		glTextureImage1DEXT(tex, target, 0, format->token,
				    piglit_width, 0,
				    GL_RGBA, GL_FLOAT, image);
	} else if (target == GL_TEXTURE_2D) {
		glTextureImage2DEXT(tex, target, 0, format->token,
				    piglit_width, height, 0,
				    GL_RGBA, GL_FLOAT, image);
	} else {
		/* 2 layers 3D image */
		glTextureImage3DEXT(tex, target, 0, format->token,
		    piglit_width, height, depth, 0,
		    GL_RGBA, GL_FLOAT, image);
	}

	/* An error here probably means format->token isn't supported, so skip the test */
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		glDeleteTextures(1, &tex);
		return 0;
	}

	if (expected_pixels) {
		*expected_pixels = image;
	} else {
		free(image);
	}
	*compressed_size = piglit_compressed_image_size(format->token, piglit_width, height * depth);
	*compressed = malloc(*compressed_size);

	glBindTexture(target, tex);
	glGetCompressedTexImage(target, 0, *compressed);
	glBindTexture(target, 0);

	piglit_check_gl_error(GL_NO_ERROR);
	return tex;
}

enum piglit_result
test_CompressedTextureImageNDEXT(void* data)
{
	const int n = (int)(intptr_t) data;
	const GLenum target = dimension_to_target(n);
	void *expected_compressed;
	float *compressed, *expected_pixels;
	int compressed_size;
	GLuint tex, tex_src;
	bool pass = true;
	const int depth = (target == GL_TEXTURE_3D) ? 2 : 1;
	const int height = (target == GL_TEXTURE_1D) ? 1 : piglit_height;

	tex_src = init_compressed_texture(target,
					  &compressed_size,
					  &expected_compressed,
					  &expected_pixels);

	if (tex_src == 0) {
		return PIGLIT_SKIP;
	}

	glGenTextures(1, &tex);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	if (use_display_list != GL_NONE)
		glNewList(list, use_display_list);

	if (target == GL_TEXTURE_1D) {
		glCompressedTextureImage1DEXT(tex, GL_TEXTURE_1D, 0, format->token,
					      piglit_width, 0,
					      compressed_size, expected_compressed);
	} else if (target == GL_TEXTURE_2D) {
		glCompressedTextureImage2DEXT(tex, GL_TEXTURE_2D, 0, format->token,
					      piglit_width, height, 0,
					      compressed_size, expected_compressed);
	} else {
		glCompressedTextureImage3DEXT(tex, GL_TEXTURE_3D, 0, format->token,
					      piglit_width, height, depth, 0,
					      compressed_size, expected_compressed);
	}

	if (use_display_list != GL_NONE)
		glEndList(list);

	/* Test glGetCompressedTextureImageEXT */
	compressed = (float*) malloc(compressed_size);
	glGetCompressedTextureImageEXT(tex, target, 0, compressed);

	if (use_display_list == GL_COMPILE) {
		piglit_check_gl_error(GL_INVALID_OPERATION);
		glCallList(list);
		glGetCompressedTextureImageEXT(tex, target, 0, compressed);
	}
	pass = memcmp(compressed, expected_compressed, compressed_size) == 0 && pass;
	free(compressed);

	/* Draw texture */
	glEnable(target);
	glBindTexture(target, tex);
	piglit_draw_rect_tex(-1.0, -1.0, 2.0, 2.0, 0.0, 0.0, 1.0, 1.0);
	glDisable(target);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Check to make sure the image was drawn correctly */
	pass = piglit_probe_image_rgba(0, 0, piglit_width, (n == 1) ? 1 : piglit_height, expected_pixels) && pass;

	glDeleteTextures(1, &tex);
	glDeleteTextures(1, &tex_src);

	free(expected_pixels);
	free(expected_compressed);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
test_CompressedTextureSubImageNDEXT(void* data)
{
	const int n = (int)(intptr_t) data;
	const GLenum target = dimension_to_target(n);
	void *expected_compressed;
	float *expected_pixels;
	int compressed_size;
	GLuint tex, tex_src, tex_src2;
	bool pass = true;
	const int depth = (target == GL_TEXTURE_3D) ? 2 : 1;
	const int height = (target == GL_TEXTURE_1D) ? 1 : piglit_height;

	/* Create a first source texture, with random content */
	tex_src = init_compressed_texture(target,
					  &compressed_size,
					  &expected_compressed,
					  NULL /* random content */);

	if (tex_src == 0) {
		return PIGLIT_SKIP;
	}

	glGenTextures(1, &tex);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	if (target == GL_TEXTURE_1D) {
		glCompressedTextureImage1DEXT(tex, GL_TEXTURE_1D, 0, format->token,
					      piglit_width, 0,
					      compressed_size, expected_compressed);
	} else if (target == GL_TEXTURE_2D) {
		glCompressedTextureImage2DEXT(tex, GL_TEXTURE_2D, 0, format->token,
					      piglit_width, height, 0,
					      compressed_size, expected_compressed);
	} else {
		glCompressedTextureImage3DEXT(tex, GL_TEXTURE_3D, 0, format->token,
					      piglit_width, height, depth, 0,
					      compressed_size, expected_compressed);
	}

	free(expected_compressed);

	/* Then create a second one with a different content */
	tex_src2 = init_compressed_texture(target,
					   &compressed_size,
					   &expected_compressed,
					   &expected_pixels);

	if (use_display_list != GL_NONE)
		glNewList(list, use_display_list);

	/* And update our compressed texture using glCompressedTextureSubImage */
	if (target == GL_TEXTURE_1D) {
		glCompressedTextureSubImage1DEXT(tex, GL_TEXTURE_1D, 0, 0,
						 piglit_width, format->token,
						 compressed_size, expected_compressed);
	} else if (target == GL_TEXTURE_2D) {
		glCompressedTextureSubImage2DEXT(tex, GL_TEXTURE_2D, 0, 0, 0,
					      piglit_width, height, format->token,
					      compressed_size, expected_compressed);
	} else {
		glCompressedTextureSubImage3DEXT(tex, GL_TEXTURE_3D, 0, 0, 0, 0,
					      piglit_width, height, depth, format->token,
					      compressed_size, expected_compressed);
	}

	if (use_display_list != GL_NONE)
		glEndList(list);
	if (use_display_list == GL_COMPILE)
		glCallList(list);

	/* Draw texture */
	glEnable(target);
	glBindTexture(target, tex);
	piglit_draw_rect_tex(-1.0, -1.0, 2.0, 2.0, 0.0, 0.0, 1.0, 1.0);
	glDisable(target);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Check to make sure the image was drawn correctly */
	pass = piglit_probe_image_rgba(0, 0, piglit_width, (n == 1) ? 1 : piglit_height, expected_pixels) && pass;

	glDeleteTextures(1, &tex);
	glDeleteTextures(1, &tex_src);
	glDeleteTextures(1, &tex_src2);

	free(expected_pixels);
	free(expected_compressed);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
piglit_display(void)
{
	int i;
	/* Not testing 1D variants since no compression format
	   supports 1D textures (see _mesa_target_can_be_compressed) */
	struct piglit_subtest tests[] = {
		{
			"CompressedTextureImage3DEXT",
			NULL,
			test_CompressedTextureImageNDEXT,
			(void*) 3
		},
		{
			"CompressedTextureImage2DEXT",
			NULL,
			test_CompressedTextureImageNDEXT,
			(void*) 2
		},
		{
			"CompressedTextureSubImage3DEXT",
			NULL,
			test_CompressedTextureSubImageNDEXT,
			(void*) 3
		},
		{
			"CompressedTextureSubImage2DEXT",
			NULL,
			test_CompressedTextureSubImageNDEXT,
			(void*) 2
		},
		{
			NULL
		}
	};

	enum piglit_result result = piglit_run_selected_subtests(tests, NULL, 0, PIGLIT_PASS);
	list = glGenLists(1);

	/* Re-run the same test but using display list GL_COMPILE */
	for (i = 0; tests[i].name; i++) {
		char* test_name_display_list;
		asprintf(&test_name_display_list, "%s + display list GL_COMPILE", tests[i].name);
		tests[i].name = test_name_display_list;
	}
	use_display_list = GL_COMPILE;
	result = piglit_run_selected_subtests(tests, NULL, 0, result);

	/* Re-run the same test but using display list GL_COMPILE_AND_EXECUTE */
	for (i = 0; tests[i].name; i++) {
		char* test_name_display_list;
		asprintf(&test_name_display_list, "%s_AND_EXECUTE", tests[i].name);
		tests[i].name = test_name_display_list;
	}
	use_display_list = GL_COMPILE_AND_EXECUTE;
	result = piglit_run_selected_subtests(tests, NULL, 0, result);

	glDeleteLists(list, 1);

	return result;
}
