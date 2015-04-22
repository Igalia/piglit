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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/*
 * This tests glCopySubImageData on different multisampled texture
 * formats.  One texture is created and filled with random data.  The
 * texture is then copied to a second texture, the texture is downloaded,
 * and the data verified.  Because glCopySubImageData is supposed to be a
 * direct memcpy, the copy is verified to be bit-for-bit copy of the
 * original.
 */

#include "piglit-util-gl.h"

#define TEX_SIZE 32
#define DEFAULT_SRC_LEVEL 1
#define DEFAULT_DST_LEVEL 3

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

struct texture_format {
	GLenum internal_format;
	const char *name;
	GLenum format;
	GLenum data_type;
	bool can_be_reinterpreted;
	GLuint bytes;
	GLuint block_width;
	GLuint block_height;
};

struct texture_format formats[] = {
#define FORMAT(IF, F, D, S, B, W, H) { IF, #IF, F, D, S, B, W, H }
	FORMAT(GL_RED, GL_RED, GL_UNSIGNED_BYTE, false, 1, 1, 1),
	FORMAT(GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, true, 1, 1, 1),
	FORMAT(GL_R8I, GL_RED_INTEGER, GL_BYTE, true, 1, 1, 1),
	FORMAT(GL_R8, GL_RED, GL_UNSIGNED_BYTE, true, 1, 1, 1),
	FORMAT(GL_R8_SNORM, GL_RED, GL_BYTE, true, 1, 1, 1),

	FORMAT(GL_RG, GL_RG, GL_UNSIGNED_BYTE, false, 2, 1, 1),
	FORMAT(GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE, true, 2, 1, 1),
	FORMAT(GL_RG8I, GL_RG_INTEGER, GL_BYTE, true, 2, 1, 1),
	FORMAT(GL_RG8, GL_RG, GL_UNSIGNED_BYTE, true, 2, 1, 1),
	FORMAT(GL_RG8_SNORM, GL_RG, GL_BYTE, true, 2, 1, 1),
	FORMAT(GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT, true, 2, 1, 1),
	FORMAT(GL_R16I, GL_RED_INTEGER, GL_SHORT, true, 2, 1, 1),
	FORMAT(GL_R16, GL_RED, GL_UNSIGNED_SHORT, true, 2, 1, 1),
	FORMAT(GL_R16_SNORM, GL_RED, GL_SHORT, true, 2, 1, 1),

	FORMAT(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, false, 3, 1, 1),
	FORMAT(GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE, true, 3, 1, 1),
	FORMAT(GL_RGB8I, GL_RGB_INTEGER, GL_BYTE, true, 3, 1, 1),
	FORMAT(GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, true, 3, 1, 1),
	FORMAT(GL_RGB8_SNORM, GL_RGB, GL_BYTE, true, 3, 1, 1),

	FORMAT(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, false, 4, 1, 1),
	FORMAT(GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, true, 4, 1, 1),
	FORMAT(GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE, true, 4, 1, 1),
	FORMAT(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, true, 4, 1, 1),
	FORMAT(GL_RGBA8_SNORM, GL_RGBA, GL_BYTE, true, 4, 1, 1),
	FORMAT(GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT, true, 4, 1, 1),
	FORMAT(GL_RG16I, GL_RG_INTEGER, GL_SHORT, true, 4, 1, 1),
	FORMAT(GL_RG16, GL_RG, GL_UNSIGNED_SHORT, true, 4, 1, 1),
	FORMAT(GL_RG16_SNORM, GL_RG, GL_SHORT, true, 4, 1, 1),
	FORMAT(GL_R32F, GL_RED, GL_FLOAT, true, 4, 1, 1),

	FORMAT(GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_SHORT, true, 6, 1, 1),
	FORMAT(GL_RGB16I, GL_RGB_INTEGER, GL_SHORT, true, 6, 1, 1),
	FORMAT(GL_RGB16, GL_RGB, GL_UNSIGNED_SHORT, true, 6, 1, 1),
	FORMAT(GL_RGB16_SNORM, GL_RGB, GL_SHORT, true, 6, 1, 1),

	FORMAT(GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, true, 8, 1, 1),
	FORMAT(GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT, true, 8, 1, 1),
	FORMAT(GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT, true, 8, 1, 1),
	FORMAT(GL_RGBA16_SNORM, GL_RGBA, GL_SHORT, true, 8, 1, 1),
	FORMAT(GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT, true, 8, 1, 1),
	FORMAT(GL_RG32I, GL_RG_INTEGER, GL_INT, true, 8, 1, 1),
	FORMAT(GL_RG32F, GL_RG, GL_FLOAT, true, 8, 1, 1),

	FORMAT(GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT, true, 12, 1, 1),
	FORMAT(GL_RGB32I, GL_RGB_INTEGER, GL_INT, true, 12, 1, 1),
	FORMAT(GL_RGB32F, GL_RGB, GL_FLOAT, true, 12, 1, 1),

	FORMAT(GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, true, 16, 1, 1),
	FORMAT(GL_RGBA32I, GL_RGBA_INTEGER, GL_INT, true, 16, 1, 1),
	FORMAT(GL_RGBA32F, GL_RGBA, GL_FLOAT, true, 16, 1, 1),

	FORMAT(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, false, 1, 1, 1),
	FORMAT(GL_ALPHA8, GL_ALPHA, GL_UNSIGNED_BYTE, false, 1, 1, 1),
	FORMAT(GL_ALPHA12, GL_ALPHA, GL_UNSIGNED_BYTE, false, 1, 1, 1),
	FORMAT(GL_ALPHA16, GL_ALPHA, GL_UNSIGNED_SHORT, false, 2, 1, 1),

	FORMAT(GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, false, 1, 1, 1),
	FORMAT(GL_LUMINANCE8, GL_LUMINANCE, GL_UNSIGNED_BYTE, false, 1, 1, 1),
	FORMAT(GL_LUMINANCE12, GL_LUMINANCE, GL_UNSIGNED_BYTE, false, 1, 1, 1),
	FORMAT(GL_LUMINANCE16, GL_LUMINANCE, GL_UNSIGNED_SHORT, false, 2, 1, 1),

	FORMAT(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, false, 2, 1, 1),
	FORMAT(GL_LUMINANCE8_ALPHA8, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, false, 2, 1, 1),
	FORMAT(GL_LUMINANCE12_ALPHA12, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, false, 2, 1, 1),
	FORMAT(GL_LUMINANCE16_ALPHA16, GL_LUMINANCE_ALPHA, GL_UNSIGNED_SHORT, false, 4, 1, 1),

	FORMAT(GL_INTENSITY, GL_RED, GL_UNSIGNED_BYTE, false, 1, 1, 1),
	FORMAT(GL_INTENSITY8, GL_RED, GL_UNSIGNED_BYTE, false, 1, 1, 1),
	FORMAT(GL_INTENSITY12, GL_RED, GL_UNSIGNED_BYTE, false, 1, 1, 1),
	FORMAT(GL_INTENSITY16, GL_RED, GL_UNSIGNED_SHORT, false, 2, 1, 1),

	FORMAT(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, false, 2, 1, 1),
	FORMAT(GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, false, 2, 1, 1),
	FORMAT(GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, false, 2, 1, 1),
	FORMAT(GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, false, 2, 1, 1),

	FORMAT(GL_DEPTH_STENCIL, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, false, 4, 1, 1),

	FORMAT(GL_STENCIL_INDEX8, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, false, 1, 1, 1),
#ifdef GL_EXT_texture_compression_s3tc
	FORMAT(GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_RED, GL_BYTE, true, 8, 4, 4),
	FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_RED, GL_BYTE, true, 8, 4, 4),
	FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_RED, GL_BYTE, true, 16, 4, 4),
	FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_RED, GL_BYTE, true, 16, 4, 4),
#ifdef GL_EXT_texture_sRGB
	FORMAT(GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, GL_RED, GL_BYTE, true, 8, 4, 4),
	FORMAT(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, GL_RED, GL_BYTE, true, 8, 4, 4),
	FORMAT(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, GL_RED, GL_BYTE, true, 16, 4, 4),
	FORMAT(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, GL_RED, GL_BYTE, true, 16, 4, 4),
#endif
#endif

#ifdef GL_EXT_texture_compression_rgtc
	FORMAT(GL_COMPRESSED_RED_RGTC1, GL_RED, GL_BYTE, true, 8, 4, 4),
	FORMAT(GL_COMPRESSED_SIGNED_RED_RGTC1, GL_RED, GL_BYTE, true, 8, 4, 4),
#endif

#ifdef GL_EXT_texture_compression_s3tc
#endif

#ifdef GL_EXT_texture_compression_rgtc
	FORMAT(GL_COMPRESSED_RG_RGTC2, GL_RED, GL_BYTE, true, 16, 4, 4),
	FORMAT(GL_COMPRESSED_SIGNED_RG_RGTC2, GL_RED, GL_BYTE, true, 16, 4, 4),
#endif

#ifdef GL_COMPRESSED_RGBA_BPTC_UNORM
	FORMAT(GL_COMPRESSED_RGBA_BPTC_UNORM, GL_RGBA, GL_BYTE, true, 16, 4, 4),
	FORMAT(GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM, GL_RGB, GL_BYTE, true, 16, 4, 4),
	FORMAT(GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT, GL_RGB, GL_BYTE, true, 16, 4, 4),
	FORMAT(GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT, GL_RGB, GL_BYTE, true, 16, 4, 4),
#endif

#undef FORMAT
};

#define ARRAY_LENGTH(X) (sizeof(X)/sizeof(*(X)))

static struct texture_format *
find_format(const char *str)
{
	int i;
	struct texture_format *format = NULL;

	for (i = 0; i < sizeof(formats) / sizeof(*formats); ++i) {
		if (strcmp(str, formats[i].name) == 0) {
			format = &formats[i];
			break;
		}
	}

	assert(format);
	return format;
}

static bool
is_format_snorm(struct texture_format *format)
{
	switch (format->internal_format) {
	case GL_R8_SNORM:
	case GL_RG8_SNORM:
	case GL_RGB8_SNORM:
	case GL_RGBA8_SNORM:
	case GL_R16_SNORM:
	case GL_RG16_SNORM:
	case GL_RGB16_SNORM:
	case GL_RGBA16_SNORM:
		return true;
	default:
		return false;
	}
}

static bool
is_format_compressed(struct texture_format *format)
{
	return format->block_width != 1 && format->block_height != 1;
}

static bool
is_format_supported(struct texture_format *format)
{
	switch (format->internal_format) {
#ifdef GL_EXT_texture_compression_rgtc
	case GL_COMPRESSED_RED_RGTC1:
	case GL_COMPRESSED_SIGNED_RED_RGTC1:
	case GL_COMPRESSED_RG_RGTC2:
	case GL_COMPRESSED_SIGNED_RG_RGTC2:
		return piglit_is_extension_supported("GL_EXT_texture_compression_rgtc");
#endif

#ifdef GL_EXT_texture_compression_s3tc
#ifdef GL_EXT_texture_sRGB
	case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
		if (!piglit_is_extension_supported("GL_EXT_texture_sRGB"))
			return false;
#endif
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		return piglit_is_extension_supported("GL_EXT_texture_compression_s3tc");
		break;
#endif

#ifdef GL_ARB_texture_compression_bptc
	case GL_COMPRESSED_RGBA_BPTC_UNORM:
	case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
	case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
	case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
		return piglit_is_extension_supported("GL_ARB_texture_compression_bptc");
#endif
	case GL_STENCIL_INDEX8:
		return piglit_is_extension_supported("GL_ARB_texture_stencil8");
	}

	return true;
}

static bool
are_formats_compatible(struct texture_format *f1, struct texture_format *f2)
{
	if (f1 == f2)
		return true;

	if (is_format_compressed(f1)) {
		if (is_format_compressed(f2))
			/* Compressed-to-compressed copies are not supported */
			return false;

		return f1->bytes == f2->bytes;
	} else if (is_format_compressed(f2)) {
		return f1->bytes == f2->bytes;
	} else {
		return f1->can_be_reinterpreted && f2->can_be_reinterpreted &&
		       f1->bytes == f2->bytes;
	}
}

static const float green[3] = {0.0, 1.0, 0.0};

struct texture_format *src_format_arg, *dst_format_arg;
unsigned char *rand_data, *src_data, *dst_data, *res_data;
int samples = 1;

void
piglit_init(int argc, char **argv)
{
	int seed = 0;
	int i, Bpp, rand_data_size;

	while (argc > 1) {
		if (sscanf(argv[1], "--seed=%d", &seed) > 0) {
			--argc;
			++argv;
		} else if (sscanf(argv[1], "--samples=%d", &samples) > 0) {
			--argc;
			++argv;
		} else if (strcmp(argv[1], "-h") == 0 ||
			   strcmp(argv[1], "--help") == 0) {
			printf("usage: arb_copy_image-multisample [--seed=seed] [--samples=samples] [src_format] [dst_format]\n");
			exit(0);
		} else {
			break;
		}
	}

	assert(samples > 0);

	srand(seed);

	piglit_require_extension("GL_ARB_copy_image");
	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_EXT_texture_integer");
	if (samples > 1) {
		int max_samples;

		piglit_require_extension("GL_ARB_texture_multisample");
		piglit_require_extension("GL_ARB_sample_shading");

		glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
		if (samples > max_samples) {
			printf("%d samples not supported\n", samples);
			piglit_report_result(PIGLIT_SKIP);
		}
	}

	if (argc > 1) {
		src_format_arg = find_format(argv[1]);
		assert(src_format_arg != NULL);
	}

	if (argc > 2) {
		dst_format_arg = find_format(argv[2]);
		assert(dst_format_arg != NULL);
		assert(dst_format_arg->bytes == src_format_arg->bytes);
	}

	/* We now go ahead and generate random data to copy.  If we are
	 * dealing with multisampled textures then we generate an array of
	 * images, one image per sample.
	 */

	if (src_format_arg) {
		/* Since we know the source format, we know the number of
		 * bits per texel, so we can restrict the ammount of random
		 * data we generate.
		 */
		Bpp = src_format_arg->bytes;
	} else {
		/* Allocate enough random data for all the tests */
		Bpp = 16;
	}
	rand_data_size = TEX_SIZE * TEX_SIZE * 2 * Bpp * samples;

	rand_data = malloc(rand_data_size);
	src_data = malloc(rand_data_size);
	dst_data = malloc(rand_data_size);
	res_data = malloc(rand_data_size);

	for (i = 0; i < rand_data_size; ++i)
		rand_data[i] = rand();
}

static void
memcpy_rect(void *src, int src_stride, int src_x, int src_y,
	    void *dst, int dst_stride, int dst_x, int dst_y,
	    int width, int height, int Bpp)
{
	int i;

	src = (char *)src + (src_y * src_stride) + (src_x * Bpp);
	dst = (char *)dst + (dst_y * dst_stride) + (dst_x * Bpp);

	for (i = 0; i < height; ++i) {
		memcpy(dst, src, width * Bpp);
		src = (char *)src + src_stride;
		dst = (char *)dst + dst_stride;
	}
}

static void
setup_test_data(struct texture_format *src_format,
		struct texture_format *dst_format)
{
	int i, j, stride, image_size, data_size;
	unsigned char *src_image, *res_image;
	int *rand_int;

	stride = TEX_SIZE * src_format->bytes;
	image_size = stride * TEX_SIZE;
	data_size = image_size * samples;

	if (src_format->data_type == GL_FLOAT ||
	    dst_format->data_type == GL_FLOAT) {
		/* If it's a floating-point type, let's avoid using invalid
		 * floating-point values.  That might throw things off */
		float *src_float = (float *)src_data;
		float *dst_float = (float *)dst_data;

		rand_int = (int *)rand_data;
		for (i = 0; i < data_size / sizeof(float); ++i)
			src_float[i] = rand_int[i] / (float)INT16_MAX;

		rand_int = (int *)(rand_data + data_size);
		for (i = 0; i < data_size / sizeof(float); ++i)
			dst_float[i] = rand_int[i] / (float)INT16_MAX;
	} else {
		memcpy(src_data, rand_data, data_size);
		memcpy(dst_data, rand_data + data_size, data_size);
	}

	if (is_format_snorm(src_format) || is_format_snorm(dst_format)) {
		/* In this case, certain values, namely INT_MIN are
		 * are disallowed and may be clampped.
		 */
		if (src_format->data_type == GL_BYTE ||
		    dst_format->data_type == GL_BYTE) {
			GLbyte *bytes = (GLbyte *)src_data;
			for (i = 0; i < data_size / sizeof(*bytes); ++i)
				if (bytes[i] == -128)
					bytes[i] = -127;

			bytes = (GLbyte *)dst_data;
			for (i = 0; i < data_size / sizeof(*bytes); ++i)
				if (bytes[i] == -128)
					bytes[i] = -127;
		} else if (src_format->data_type == GL_SHORT ||
			   dst_format->data_type == GL_SHORT) {
			GLshort *shorts = (GLshort *)src_data;
			for (i = 0; i < data_size / sizeof(*shorts); ++i)
				if (shorts[i] == -32768)
					shorts[i] = -32767;

			shorts = (GLshort *)dst_data;
			for (i = 0; i < data_size / sizeof(*shorts); ++i)
				if (shorts[i] == -32768)
					shorts[i] = -32767;
		} else {
			assert(!"Invalid data type for SNORM format");
		}
	}

	/* Creates the expected result image from the source and
	 * destination images.  The middle TEX_SIZE/2 x TEX_SIZE/2 pixels
	 * should come from src_data while the rest should come from
	 * dest_data.  If samples > 1, then, since CopyImageSubData copies
	 * all the samples, we need to copy the center of every plane.
	 */

	/* Copy all of dst_data to result */
	memcpy(res_data, dst_data, data_size);
	for (j = 0; j < samples; ++j) {
		src_image = src_data + (j * image_size);
		res_image = res_data + (j * image_size);
		/* Copy the center TEX_SIZE/2 x TEX_SIZE/2 pixels froms
		 * src_data to res_data
		 */
		memcpy_rect(src_image, stride, TEX_SIZE/4, TEX_SIZE/4,
			    res_image, stride, TEX_SIZE/4, TEX_SIZE/4,
			    TEX_SIZE/2, TEX_SIZE/2, src_format->bytes);

		/* Copy the upper-left corner of the result to the
		 * lower-right of the result
		 */
		memcpy_rect(res_image, stride, 0, TEX_SIZE/2,
			    res_image, stride, TEX_SIZE/2, 0,
			    TEX_SIZE/2, TEX_SIZE/2, src_format->bytes);
	}
}

const char ms_compare_vs_source[] =
"#version 130\n"
"in vec2 vertex;\n"
"out vec2 tex_coords;\n"
"void main()\n"
"{\n"
"	tex_coords = vertex;\n"
"	vec2 pos = (vertex.xy * 2) - vec2(1, 1);\n"
"	gl_Position = vec4(pos, 0, 1);\n"
"}\n";

const char ms_compare_fs_source[] =
"#version 130\n"
"#extension GL_ARB_texture_multisample : enable\n"
"in vec2 tex_coords;\n"
"uniform %ssampler2DMS tex1;\n"
"uniform %ssampler2DMS tex2;\n"
"uniform ivec2 tex_size;\n"
"uniform int samples;\n"
"const vec4 red = vec4(1, 0, 0, 1);\n"
"const vec4 green = vec4(0, 1, 0, 1);\n"
"void main()\n"
"{\n"
"	int count = 0;\n"
"	ivec2 tex_px = ivec2(tex_coords * tex_size);\n"
"	for (int i = 0; i < samples; ++i) {\n"
"		%svec4 val1 = texelFetch(tex1, tex_px, i);\n"
"		%svec4 val2 = texelFetch(tex2, tex_px, i);\n"
"		if (val1 == val2)\n"
"			++count;\n"
"	}\n"
"	gl_FragColor = mix(red, green, float(count) / float(samples));\n"
"}\n";

void
load_compare_program(struct texture_format *format)
{
	static struct {
		GLuint prog;
		GLuint tex1;
		GLuint tex2;
		GLuint tex_size;
		GLuint samples;
	} comp, ucomp, icomp, *compare;
	char *fs_src, *gtype;

	switch (format->format) {
	case GL_RED_INTEGER:
	case GL_RG_INTEGER:
	case GL_RGB_INTEGER:
	case GL_RGBA_INTEGER:
	case GL_BGRA_INTEGER:
	case GL_STENCIL_INDEX:
		switch (format->data_type) {
		case GL_BYTE:
		case GL_SHORT:
		case GL_INT:
			compare = &icomp;
			break;
		case GL_UNSIGNED_BYTE:
		case GL_UNSIGNED_SHORT:
		case GL_UNSIGNED_INT:
			compare = &ucomp;
			break;
		default:
			assert(!"Invalid data type");
		}
		break;
	case GL_RED:
	case GL_RG:
	case GL_RGB:
	case GL_RGBA:
	case GL_BGRA:
	case GL_ALPHA:
	case GL_LUMINANCE:
	case GL_LUMINANCE_ALPHA:
	case GL_INTENSITY:
	case GL_DEPTH_COMPONENT:
		compare = &comp;
		break;
	default:
		assert(!"Invalid Format");
	}

	if (!compare->prog) {
		if (compare == &comp) {
			gtype = "";
		} else if (compare == &ucomp) {
			gtype = "u";
		} else if (compare == &icomp) {
			gtype = "i";
		} else {
			assert(!"Invalid comparison fucntion");
			gtype = "";
		}

		/* The generated source will be shorter because we replace
		 * a bunch of "%s" with "u", "i", or ""
		 */
		fs_src = malloc(sizeof(ms_compare_fs_source));
		snprintf(fs_src, sizeof(ms_compare_fs_source),
			 ms_compare_fs_source, gtype, gtype, gtype, gtype);

		compare->prog = piglit_build_simple_program_unlinked(
			ms_compare_vs_source, fs_src);
		glBindAttribLocation(compare->prog, 0, "vertex");
		glLinkProgram(compare->prog);
		piglit_link_check_status(compare->prog);

		compare->tex1 = glGetUniformLocation(compare->prog, "tex1");
		compare->tex2 = glGetUniformLocation(compare->prog, "tex2");
		compare->tex_size = glGetUniformLocation(compare->prog, "tex_size");
		compare->samples = glGetUniformLocation(compare->prog, "samples");
	}

	glUseProgram(compare->prog);
	glUniform1i(compare->tex1, 0);
	glUniform1i(compare->tex2, 1);
	glUniform2i(compare->tex_size, TEX_SIZE, TEX_SIZE);
	glUniform1i(compare->samples, samples);
}

static enum piglit_result
run_multisample_test(struct texture_format *src_format,
		     struct texture_format *dst_format)
{
	bool pass = true;
	int fbo_width, fbo_height;
	GLuint fbo, rb, src_tex, dst_tex, res_tex;
	static const GLfloat verts[] = {
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
		0.0, 0.0
	};

	/* Upload the source, destination, and expected result */
	src_tex = piglit_multisample_texture(GL_TEXTURE_2D_MULTISAMPLE, 0,
					     src_format->internal_format,
					     TEX_SIZE, TEX_SIZE, 1, samples,
					     src_format->format,
					     src_format->data_type, src_data);

	dst_tex = piglit_multisample_texture(GL_TEXTURE_2D_MULTISAMPLE, 0,
					     dst_format->internal_format,
					     TEX_SIZE, TEX_SIZE, 1, samples,
					     dst_format->format,
					     dst_format->data_type, dst_data);

	res_tex = piglit_multisample_texture(GL_TEXTURE_2D_MULTISAMPLE, 0,
					     dst_format->internal_format,
					     TEX_SIZE, TEX_SIZE, 1, samples,
					     dst_format->format,
					     dst_format->data_type, res_data);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* If any of these are zero, but there was no error, then it must
	 * not be renderable, so we just skip without even reporting the
	 * subtest.
	 */
	if ((src_tex == 0 || dst_tex == 0 || res_tex == 0) && pass)
		return PIGLIT_SKIP;

	glCopyImageSubData(src_tex, GL_TEXTURE_2D_MULTISAMPLE, 0,
			   TEX_SIZE / 4, TEX_SIZE / 4, 0,
			   dst_tex, GL_TEXTURE_2D_MULTISAMPLE, 0,
			   TEX_SIZE / 4, TEX_SIZE / 4, 0,
			   TEX_SIZE / 2, TEX_SIZE / 2, 1);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	glCopyImageSubData(dst_tex, GL_TEXTURE_2D_MULTISAMPLE, 0,
			   0, TEX_SIZE / 2, 0,
			   dst_tex, GL_TEXTURE_2D_MULTISAMPLE, 0,
			   TEX_SIZE / 2, 0, 0,
			   TEX_SIZE / 2, TEX_SIZE / 2, 1);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	if (piglit_automatic) {
		fbo_width = TEX_SIZE;
		fbo_height = TEX_SIZE;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenRenderbuffers(1, &rb);
		glBindRenderbuffer(GL_RENDERBUFFER, rb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA,
				      fbo_width, fbo_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
					  GL_COLOR_ATTACHMENT0,
					  GL_RENDERBUFFER, rb);
	} else {
		fbo_width = piglit_width;
		fbo_height = piglit_height;
		glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	}
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	glViewport(0, 0, fbo_width, fbo_height);

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Now we use a comparison shader to check to see if the
	 * destination matches the expected result.
	 */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, dst_tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, res_tex);

	load_compare_program(dst_format);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, verts);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* If the destination texture matches the expected result, we
	 * should get green.  If not, we get red and this test fails.
	 */
	pass &= piglit_probe_rect_rgb(0, 0, fbo_width, fbo_height, green);

	glDeleteTextures(1, &src_tex);
	glDeleteTextures(1, &dst_tex);
	glDeleteTextures(1, &res_tex);

	if (!piglit_automatic)
		piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static bool
check_texture(GLuint texture, unsigned level,
	      const struct texture_format *format, const unsigned char *data)
{
	int i, j, k;
	bool pass = true;
	unsigned char *tex_data;
	float passrate;

	tex_data = malloc(TEX_SIZE * TEX_SIZE * format->bytes);

	glBindTexture(GL_TEXTURE_2D, texture);
	if (format->block_width != 1 || format->block_height != 1) {
		/* Compressed */
		glGetCompressedTexImage(GL_TEXTURE_2D, level, tex_data);
	} else {
		glGetTexImage(GL_TEXTURE_2D, level, format->format,
			      format->data_type, tex_data);
	}

	passrate = 0;
	for (j = 0; j < TEX_SIZE; ++j) {
	        for (i = 0; i < TEX_SIZE; ++i) {
			if (memcmp(tex_data + ((j * TEX_SIZE) + i) * format->bytes,
				   data + ((j * TEX_SIZE) + i) * format->bytes,
				   format->bytes) == 0) {
				passrate += 1;
			} else {
				fprintf(stdout, "texel mismatch at (%d, %d); expected 0x",
					i, j);
				for (k = format->bytes - 1; k >= 0; --k)
					fprintf(stdout, "%02x", data[((j * TEX_SIZE) + i) * format->bytes + k]);

				fprintf(stdout, ", received 0x");
				for (k = format->bytes - 1; k >= 0; --k)
					fprintf(stdout, "%02x", tex_data[((j * TEX_SIZE) + i) * format->bytes + k]);
				fprintf(stdout, ".\n");

				pass = false;
			}
		}
	}
	passrate /= TEX_SIZE * TEX_SIZE;
	printf("%0.1f%% of pixels match\n", passrate * 100);

	free(tex_data);

	return pass;
}

static enum piglit_result
run_test(struct texture_format *src_format, struct texture_format *dst_format)
{
	bool pass = true, warn = false;
	unsigned src_width, src_height, dst_width, dst_height;
	unsigned src_level, dst_level;
	GLuint texture[2];

	glEnable(GL_TEXTURE_2D);

	glGenTextures(2, texture);

	src_width = TEX_SIZE * src_format->block_width;
	src_height = TEX_SIZE * src_format->block_height;

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	if (src_format->can_be_reinterpreted) {
		src_level = DEFAULT_SRC_LEVEL;
		glTexStorage2D(GL_TEXTURE_2D, src_level + 2,
			       src_format->internal_format,
			       src_width << src_level, src_height << src_level);
		if (src_format->block_width != 1 ||
		    src_format->block_height != 1) {
			/* Compressed */
			glCompressedTexSubImage2D(GL_TEXTURE_2D, src_level,
						  0, 0,
						  src_width, src_height,
						  src_format->internal_format,
						  TEX_SIZE * TEX_SIZE * src_format->bytes,
						  src_data);
		} else {
			glTexSubImage2D(GL_TEXTURE_2D, src_level, 0, 0,
				     src_width, src_height, src_format->format,
				     src_format->data_type, src_data);
		}
	} else {
		src_level = 0;
		/* All non-reintepretable textures are uncompressed */
		glTexImage2D(GL_TEXTURE_2D, 0, src_format->internal_format,
			     src_width, src_height, 0, src_format->format,
			     src_format->data_type, src_data);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	if (!pass) goto cleanup;
	warn |= !check_texture(texture[0], src_level, src_format, src_data);

	dst_width = TEX_SIZE * dst_format->block_width;
	dst_height = TEX_SIZE * dst_format->block_height;

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	if (dst_format->can_be_reinterpreted) {
		dst_level = DEFAULT_DST_LEVEL;
		glTexStorage2D(GL_TEXTURE_2D, dst_level + 2,
			       dst_format->internal_format,
			       dst_width << dst_level, dst_height << dst_level);
		if (dst_format->block_width != 1 ||
		    dst_format->block_height != 1) {
			/* Compressed */
			glCompressedTexSubImage2D(GL_TEXTURE_2D, dst_level,
						  0, 0,
						  dst_width, dst_height,
						  dst_format->internal_format,
						  TEX_SIZE * TEX_SIZE * dst_format->bytes,
						  dst_data);
		} else {
			glTexSubImage2D(GL_TEXTURE_2D, dst_level, 0, 0,
				     dst_width, dst_height, dst_format->format,
				     dst_format->data_type, dst_data);
		}
	} else {
		dst_level = 0;
		/* All non-reintepritable textures are uncompressed */
		glTexImage2D(GL_TEXTURE_2D, 0, dst_format->internal_format,
			     dst_width, dst_height, 0, dst_format->format,
			     dst_format->data_type, dst_data);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	pass &= piglit_check_gl_error(GL_NO_ERROR);
	if (!pass) goto cleanup;
	warn |= !check_texture(texture[1], dst_level, dst_format, dst_data);

	glCopyImageSubData(texture[0], GL_TEXTURE_2D, src_level,
			   src_width / 4, src_height / 4, 0,
			   texture[1], GL_TEXTURE_2D, dst_level,
			   dst_width / 4, dst_height / 4, 0,
			   src_width / 2, src_height / 2, 1);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	glCopyImageSubData(texture[1], GL_TEXTURE_2D, dst_level,
			   0, dst_height / 2, 0,
			   texture[1], GL_TEXTURE_2D, dst_level,
			   dst_width / 2, 0, 0,
			   dst_width / 2, dst_height / 2, 1);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	pass &= check_texture(texture[1], dst_level, dst_format, res_data);

cleanup:
	glDeleteTextures(2, texture);

	glDisable(GL_TEXTURE_2D);

	return pass ? (warn ? PIGLIT_WARN : PIGLIT_PASS) : PIGLIT_FAIL;
}

enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_PASS;
	enum piglit_result subtest;
	struct texture_format *src_format_list, *dst_format_list;
	struct texture_format *src_format, *dst_format;
	int sf, df, src_format_count, dst_format_count;

	if (src_format_arg) {
		src_format_list = src_format_arg;
		src_format_count = 1;
	} else {
		src_format_list = formats;
		src_format_count = ARRAY_LENGTH(formats);
	}

	if (dst_format_arg) {
		dst_format_list = dst_format_arg;
		dst_format_count = 1;
	} else {
		dst_format_list = formats;
		dst_format_count = ARRAY_LENGTH(formats);
	}

	for (sf = 0; sf < src_format_count; ++sf) {
		src_format = &src_format_list[sf];
		if (!is_format_supported(src_format))
			continue;

		for (df = 0; df < dst_format_count; ++df) {
			dst_format = &dst_format_list[df];
			if (!is_format_supported(dst_format))
				continue;
			if (!are_formats_compatible(src_format, dst_format))
				continue;

			setup_test_data(src_format, dst_format);
			if (samples == 1) {
				subtest = run_test(src_format,
						   dst_format);
			} else {
				if (is_format_compressed(src_format) ||
				    is_format_compressed(dst_format))
					continue;

				subtest = run_multisample_test(src_format,
							       dst_format);
			}

			if (!src_format_arg) {
				/* In this case, we're running a full suite
				 * of subtests, report accordingly.
				 */
				piglit_report_subtest_result(subtest,
					"Source: %s/Destination: %s",
					src_format->name, dst_format->name);
			} else if (!dst_format_arg) {
				/* In this case, the source format was
				 * specified but the destination was not.
				 * Report one subtest per destination.
				 */
				piglit_report_subtest_result(subtest,
					"Destination Format: %s",
					dst_format->name);
			}

			if (subtest == PIGLIT_FAIL)
				result = PIGLIT_FAIL;
			else if (subtest == PIGLIT_WARN && result == PIGLIT_PASS)
				result = PIGLIT_WARN;
		}
	}

	return result;
}
