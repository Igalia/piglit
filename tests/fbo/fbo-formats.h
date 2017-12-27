/*
 * Copyright © 2009-2010 Intel Corporation
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
 *
 */


struct format_desc {
	GLenum internalformat;

	/**
	 * Base internal format corresponding to internalformat.  See
	 * for example the GL 3.0 spec, tables 3.16 through 3.19.
	 *
	 * Base internal format is always one of the following:
	 * GL_ALPHA, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL,
	 * GL_INTENSITY, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RED,
	 * GL_RG, GL_RGB, GL_RGBA.
	 */
	GLenum base_internal_format;

	/* For color formats, the number of bits in the smallest channel for
	 * the sized formats and 0 for unsized and compressed formats.
	 *
	 * For sized depth/stencil formats, the number of bits in the depth
	 * component (i.e., 0 for stencil-only formats).
	 */
	unsigned min_bits;
};

static const struct format_desc core[] = {
	{ 3, GL_RGB, 0 },
	{ 4, GL_RGBA, 0 },
	{ GL_RGB, GL_RGB, 0 },
	{ GL_RGBA, GL_RGBA, 0 },
	{ GL_ALPHA, GL_ALPHA, 0 },
	{ GL_LUMINANCE, GL_LUMINANCE, 0 },
	{ GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, 0 },
	{ GL_INTENSITY, GL_INTENSITY, 0 },

	{ GL_ALPHA4, GL_ALPHA, 4 },
	{ GL_ALPHA8, GL_ALPHA, 8 },
	{ GL_ALPHA12, GL_ALPHA, 12 },
	{ GL_ALPHA16, GL_ALPHA, 16 },

	{ GL_LUMINANCE4, GL_LUMINANCE, 4 },
	{ GL_LUMINANCE8, GL_LUMINANCE, 8 },
	{ GL_LUMINANCE12, GL_LUMINANCE, 12 },
	{ GL_LUMINANCE16, GL_LUMINANCE, 16 },

	{ GL_LUMINANCE4_ALPHA4, GL_LUMINANCE_ALPHA, 4 },
	{ GL_LUMINANCE8_ALPHA8, GL_LUMINANCE_ALPHA, 8 },
	{ GL_LUMINANCE12_ALPHA12, GL_LUMINANCE_ALPHA, 12 },
	{ GL_LUMINANCE16_ALPHA16, GL_LUMINANCE_ALPHA, 16 },

	{ GL_INTENSITY4, GL_INTENSITY, 4 },
	{ GL_INTENSITY8, GL_INTENSITY, 8 },
	{ GL_INTENSITY12, GL_INTENSITY, 12 },
	{ GL_INTENSITY16, GL_INTENSITY, 16 },

	{ GL_R3_G3_B2, GL_RGB, 2 },
	{ GL_RGB4, GL_RGB, 4 },
	{ GL_RGB5, GL_RGB, 5 },
	{ GL_RGB8, GL_RGB, 8 },
	{ GL_RGB10, GL_RGB, 10 },
	{ GL_RGB12, GL_RGB, 12 },
	{ GL_RGB16, GL_RGB, 16 },

	{ GL_RGBA2, GL_RGBA, 2 },
	{ GL_RGBA4, GL_RGBA, 4 },
	{ GL_RGB5_A1, GL_RGBA, 1 },
	{ GL_RGBA8, GL_RGBA, 8 },
	{ GL_RGB10_A2, GL_RGBA, 2 },
	{ GL_RGBA12, GL_RGBA, 12 },
	{ GL_RGBA16, GL_RGBA, 16 },
};

static const struct format_desc arb_depth_texture[] = {
	{ GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, 0 },
	{ GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, 16 },
	{ GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, 24 },
	{ GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, 32 },
};

static const struct format_desc ext_packed_depth_stencil[] = {
	{ GL_DEPTH_STENCIL_EXT, GL_DEPTH_STENCIL, 0 },
	{ GL_DEPTH24_STENCIL8_EXT, GL_DEPTH_STENCIL, 24 },
};

static const struct format_desc ext_texture_srgb[] = {
	{ GL_SRGB_EXT, GL_RGB, 0 },
	{ GL_SRGB8_EXT, GL_RGB, 8 },
	{ GL_SRGB_ALPHA_EXT, GL_RGBA, 0 },
	{ GL_SRGB8_ALPHA8_EXT, GL_RGBA, 8 },
	{ GL_SLUMINANCE_ALPHA_EXT, GL_LUMINANCE_ALPHA, 0 },
	{ GL_SLUMINANCE8_ALPHA8_EXT, GL_LUMINANCE_ALPHA, 8 },
	{ GL_SLUMINANCE_EXT, GL_LUMINANCE, 0 },
	{ GL_SLUMINANCE8_EXT, GL_LUMINANCE, 8 },
};

static const struct format_desc ext_texture_srgb_compressed[] = {
	{ GL_COMPRESSED_SRGB_EXT, GL_RGB, 0 },
	{ GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, GL_RGB, 0 },
	{ GL_COMPRESSED_SRGB_ALPHA_EXT, GL_RGBA, 0 },
	{ GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, GL_RGBA, 0 },
	{ GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, GL_RGBA, 0 },
	{ GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, GL_RGBA, 0 },
	{ GL_COMPRESSED_SLUMINANCE_ALPHA_EXT, GL_LUMINANCE_ALPHA, 0 },
	{ GL_COMPRESSED_SLUMINANCE_EXT, GL_LUMINANCE, 0 },
};

static const struct format_desc ext_texture_compression[] = {
	{ GL_COMPRESSED_ALPHA, GL_ALPHA, 0 },
	{ GL_COMPRESSED_LUMINANCE, GL_LUMINANCE, 0 },
	{ GL_COMPRESSED_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, 0 },
	{ GL_COMPRESSED_INTENSITY, GL_INTENSITY, 0 },
	{ GL_COMPRESSED_RGB, GL_RGB, 0 },
	{ GL_COMPRESSED_RGBA, GL_RGBA, 0 },
};

static const struct format_desc tdfx_texture_compression_fxt1[] = {
	{ GL_COMPRESSED_RGB_FXT1_3DFX, GL_RGB, 0 },
	{ GL_COMPRESSED_RGBA_FXT1_3DFX, GL_RGBA, 0 },
};

static const struct format_desc ext_texture_compression_s3tc[] = {
	{ GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_RGB, 0 },
	{ GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_RGBA, 0 },
	{ GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_RGBA, 0 },
	{ GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_RGBA, 0 },
};

static const struct format_desc ext_texture_integer[] = {
	{ GL_RGBA8UI_EXT, GL_RGBA, 8 },
	{ GL_RGBA16UI_EXT, GL_RGBA, 16 },
	{ GL_RGBA32UI_EXT, GL_RGBA, 32 },
	{ GL_RGBA8I_EXT, GL_RGBA, 8 },
	{ GL_RGBA16I_EXT, GL_RGBA, 16 },
	{ GL_RGBA32I_EXT, GL_RGBA, 32 },
	{ GL_RGB8UI_EXT, GL_RGB, 8 },
	{ GL_RGB16UI_EXT, GL_RGB, 16 },
	{ GL_RGB32UI_EXT, GL_RGB, 32 },
	{ GL_RGB8I_EXT, GL_RGB, 8 },
	{ GL_RGB16I_EXT, GL_RGB, 16 },
	{ GL_RGB32I_EXT, GL_RGB, 32 },
	{ GL_ALPHA8UI_EXT, GL_ALPHA, 8 },
	{ GL_ALPHA16UI_EXT, GL_ALPHA, 16 },
	{ GL_ALPHA32UI_EXT, GL_ALPHA, 32 },
	{ GL_ALPHA8I_EXT, GL_ALPHA, 8 },
	{ GL_ALPHA16I_EXT, GL_ALPHA, 16 },
	{ GL_ALPHA32I_EXT, GL_ALPHA, 32 },
	{ GL_INTENSITY8UI_EXT, GL_INTENSITY, 8 },
	{ GL_INTENSITY16UI_EXT, GL_INTENSITY, 16 },
	{ GL_INTENSITY32UI_EXT, GL_INTENSITY, 32 },
	{ GL_INTENSITY8I_EXT, GL_INTENSITY, 8 },
	{ GL_INTENSITY16I_EXT, GL_INTENSITY, 16 },
	{ GL_INTENSITY32I_EXT, GL_INTENSITY, 32 },
	{ GL_LUMINANCE8UI_EXT, GL_LUMINANCE, 8 },
	{ GL_LUMINANCE16UI_EXT, GL_LUMINANCE, 16 },
	{ GL_LUMINANCE32UI_EXT, GL_LUMINANCE, 32 },
	{ GL_LUMINANCE8I_EXT, GL_LUMINANCE, 8 },
	{ GL_LUMINANCE16I_EXT, GL_LUMINANCE, 16 },
	{ GL_LUMINANCE32I_EXT, GL_LUMINANCE, 32 },
	{ GL_LUMINANCE_ALPHA8UI_EXT, GL_LUMINANCE_ALPHA, 8 },
	{ GL_LUMINANCE_ALPHA16UI_EXT, GL_LUMINANCE_ALPHA, 16 },
	{ GL_LUMINANCE_ALPHA32UI_EXT, GL_LUMINANCE_ALPHA, 32 },
	{ GL_LUMINANCE_ALPHA8I_EXT, GL_LUMINANCE_ALPHA, 8 },
	{ GL_LUMINANCE_ALPHA16I_EXT, GL_LUMINANCE_ALPHA, 16 },
	{ GL_LUMINANCE_ALPHA32I_EXT, GL_LUMINANCE_ALPHA, 32 },
};

static const struct format_desc arb_texture_rg[] = {
	{ GL_R8, GL_RED, 8 },
	{ GL_R16, GL_RED, 16 },
	{ GL_RG, GL_RG, 0 },
	{ GL_RG8, GL_RG, 8 },
	{ GL_RG16, GL_RG, 16 },
};

static const struct format_desc arb_texture_rg_int[] = {
	{ GL_R8I, GL_RED, 8 },
	{ GL_R8UI, GL_RED, 8 },
	{ GL_R16I, GL_RED, 16 },
	{ GL_R16UI, GL_RED, 16 },
	{ GL_R32I, GL_RED, 32 },
	{ GL_R32UI, GL_RED, 32 },
	{ GL_RG_INTEGER, GL_RG, 0 },
	{ GL_RG8I, GL_RG, 8 },
	{ GL_RG8UI, GL_RG, 8 },
	{ GL_RG16I, GL_RG, 16 },
	{ GL_RG16UI, GL_RG, 16 },
	{ GL_RG32I, GL_RG, 32 },
	{ GL_RG32UI, GL_RG, 32 },
};

static const struct format_desc arb_texture_rg_float[] = {
	{ GL_R16F, GL_RED, 16 },
	{ GL_R32F, GL_RED, 32 },
	{ GL_RG16F, GL_RG, 16 },
	{ GL_RG32F, GL_RG, 32 },
};

static const struct format_desc ext_texture_shared_exponent[] = {
	{ GL_RGB9_E5_EXT, GL_RGB, 9 },
};

static const struct format_desc ext_packed_float[] = {
	{ GL_R11F_G11F_B10F_EXT, GL_RGB, 10 },
};

static const struct format_desc arb_depth_buffer_float[] = {
	{ GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, 32 },
	{ GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, 32 },
};

static const struct format_desc ext_texture_compression_rgtc[] = {
	{ GL_COMPRESSED_RED, GL_RED, 0 },
	{ GL_COMPRESSED_RED_RGTC1_EXT, GL_RED, 0 },
	{ GL_COMPRESSED_RG, GL_RG, 0 },
	{ GL_COMPRESSED_RED_GREEN_RGTC2_EXT, GL_RG, 0 },
};

static const struct format_desc ext_texture_compression_rgtc_signed[] = {
	{ GL_COMPRESSED_SIGNED_RED_RGTC1_EXT, GL_RED, 0 },
	{ GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT, GL_RG, 0 },
};

static const struct format_desc arb_texture_float[] = {
	{ GL_RGB16F_ARB, GL_RGB, 16 },
	{ GL_RGBA16F_ARB, GL_RGBA, 16 },
	{ GL_ALPHA16F_ARB, GL_ALPHA, 16 },
	{ GL_LUMINANCE16F_ARB, GL_LUMINANCE, 16 },
	{ GL_LUMINANCE_ALPHA16F_ARB, GL_LUMINANCE_ALPHA, 16 },
	{ GL_INTENSITY16F_ARB, GL_INTENSITY, 16 },
	{ GL_RGB32F_ARB, GL_RGB, 32 },
	{ GL_RGBA32F_ARB, GL_RGBA, 32 },
	{ GL_ALPHA32F_ARB, GL_ALPHA, 32 },
	{ GL_LUMINANCE32F_ARB, GL_LUMINANCE, 32 },
	{ GL_LUMINANCE_ALPHA32F_ARB, GL_LUMINANCE_ALPHA, 32 },
	{ GL_INTENSITY32F_ARB, GL_INTENSITY, 32 }
};

static const struct format_desc ati_texture_compression_3dc[] = {
	{ GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI, GL_LUMINANCE_ALPHA, 0 }
};

static const struct format_desc ext_texture_compression_latc[] = {
	{ GL_COMPRESSED_LUMINANCE_LATC1_EXT, GL_LUMINANCE, 0 },
	{ GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT, GL_LUMINANCE_ALPHA, 0 },
};

static const struct format_desc ext_texture_compression_latc_signed[] = {
	{ GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT, GL_LUMINANCE, 0 },
	{ GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT, GL_LUMINANCE_ALPHA, 0 }
};

static const struct format_desc ext_texture_snorm[] = {
	{ GL_RED_SNORM, GL_RED, 0 },
	{ GL_R8_SNORM, GL_RED, 8 },
	{ GL_RG_SNORM, GL_RG, 0 },
	{ GL_RG8_SNORM, GL_RG, 8 },
	{ GL_RGB_SNORM, GL_RGB, 0 },
	{ GL_RGB8_SNORM, GL_RGB, 8 },
	{ GL_RGBA_SNORM, GL_RGBA, 0 },
	{ GL_RGBA8_SNORM, GL_RGBA, 8 },
	{ GL_ALPHA_SNORM, GL_ALPHA, 0 },
	{ GL_ALPHA8_SNORM, GL_ALPHA, 8 },
	{ GL_LUMINANCE_SNORM, GL_LUMINANCE, 0 },
	{ GL_LUMINANCE8_SNORM, GL_LUMINANCE, 8 },
	{ GL_LUMINANCE_ALPHA_SNORM, GL_LUMINANCE_ALPHA, 0 },
	{ GL_LUMINANCE8_ALPHA8_SNORM, GL_LUMINANCE_ALPHA, 8 },
	{ GL_INTENSITY_SNORM, GL_INTENSITY, 0 },
	{ GL_INTENSITY8_SNORM, GL_INTENSITY, 8 },
	{ GL_R16_SNORM, GL_RED, 16 },
	{ GL_RG16_SNORM, GL_RG, 16 },
	{ GL_RGB16_SNORM, GL_RGB, 16 },
	{ GL_RGBA16_SNORM, GL_RGBA, 16 },
	{ GL_ALPHA16_SNORM, GL_ALPHA, 16 },
	{ GL_LUMINANCE16_SNORM, GL_LUMINANCE, 16 },
	{ GL_LUMINANCE16_ALPHA16_SNORM, GL_LUMINANCE_ALPHA, 16 },
	{ GL_INTENSITY16_SNORM, GL_INTENSITY, 16 }
};

static const struct format_desc arb_texture_compression_bptc_unorm[] = {
	{ GL_COMPRESSED_RGBA_BPTC_UNORM, GL_RGBA, 0 },
	{ GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM, GL_RGBA, 0 }
};

static const struct format_desc arb_texture_compression_bptc_float[] = {
	{ GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT, GL_RGB, 0 },
	{ GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT, GL_RGB, 0 }
};

static const struct format_desc arb_es2_compatibility[] = {
	{ GL_RGB565, GL_RGB, 5 }
};

static const struct format_desc arb_texture_stencil8[] = {
	{ GL_STENCIL_INDEX8, GL_STENCIL_INDEX, 0 }
};

struct test_desc {
	const struct format_desc *format;
	unsigned num_formats;
	const char *param;
	GLenum basetype;
	const char *ext[3];
};

static const struct test_desc test_sets[] = {
	{
		core,
		ARRAY_SIZE(core),
		"Core formats",
		GL_UNSIGNED_NORMALIZED,
	},
	{
		ext_texture_compression,
		ARRAY_SIZE(ext_texture_compression),
		"GL_ARB_texture_compression",
		GL_UNSIGNED_NORMALIZED,
		{"GL_ARB_texture_compression"}
	},
	{
		tdfx_texture_compression_fxt1,
		ARRAY_SIZE(tdfx_texture_compression_fxt1),
		"GL_3DFX_texture_compression_FXT1",
		GL_UNSIGNED_NORMALIZED,
		{"GL_ARB_texture_compression",
		 "GL_3DFX_texture_compression_FXT1"},
	},
	{
		ext_texture_compression_s3tc,
		ARRAY_SIZE(ext_texture_compression_s3tc),
		"GL_EXT_texture_compression_s3tc",
		GL_UNSIGNED_NORMALIZED,
		{"GL_ARB_texture_compression",
		 "GL_EXT_texture_compression_s3tc"},
	},
	{
		arb_depth_texture,
		ARRAY_SIZE(arb_depth_texture),
		"GL_ARB_depth_texture",
		GL_UNSIGNED_NORMALIZED,
		{"GL_ARB_depth_texture"},
	},
	{
		ext_packed_depth_stencil,
		ARRAY_SIZE(ext_packed_depth_stencil),
		"GL_EXT_packed_depth_stencil",
		GL_UNSIGNED_NORMALIZED,
		{"GL_EXT_packed_depth_stencil"},
	},
	{
		ext_texture_srgb,
		ARRAY_SIZE(ext_texture_srgb),
		"GL_EXT_texture_sRGB",
		GL_UNSIGNED_NORMALIZED,
		{"GL_EXT_texture_sRGB"}
	},
	{
		ext_texture_srgb_compressed,
		ARRAY_SIZE(ext_texture_srgb_compressed),
		"GL_EXT_texture_sRGB-s3tc",
		GL_UNSIGNED_NORMALIZED,
		{"GL_EXT_texture_sRGB",
		 "GL_ARB_texture_compression",
		 "GL_EXT_texture_compression_s3tc"},
	},
	{
		ext_texture_integer,
		ARRAY_SIZE(ext_texture_integer),
		"GL_EXT_texture_integer",
		GL_INT,
		{"GL_EXT_texture_integer"}
	},
	{
		arb_texture_rg,
		ARRAY_SIZE(arb_texture_rg),
		"GL_ARB_texture_rg",
		GL_UNSIGNED_NORMALIZED,
		{"GL_ARB_texture_rg"}
	},
	{
		arb_texture_rg_int,
		ARRAY_SIZE(arb_texture_rg_int),
		"GL_ARB_texture_rg-int",
		GL_INT,
		{"GL_ARB_texture_rg",
		 "GL_EXT_texture_integer"}
	},
	{
		arb_texture_rg_float,
		ARRAY_SIZE(arb_texture_rg_float),
		"GL_ARB_texture_rg-float",
		GL_FLOAT,
		{"GL_ARB_texture_rg",
		 "GL_ARB_texture_float"}
	},
	{
		ext_texture_shared_exponent,
		ARRAY_SIZE(ext_texture_shared_exponent),
		"GL_EXT_texture_shared_exponent",
		GL_UNSIGNED_NORMALIZED, /* XXX UNSIGNED_FLOAT */
		{"GL_EXT_texture_shared_exponent"}
	},
	{
		ext_packed_float,
		ARRAY_SIZE(ext_packed_float),
		"GL_EXT_packed_float",
		GL_UNSIGNED_NORMALIZED, /* XXX UNSIGNED_FLOAT */
		{"GL_EXT_packed_float"}
	},
	{
		arb_depth_buffer_float,
		ARRAY_SIZE(arb_depth_buffer_float),
		"GL_ARB_depth_buffer_float",
		GL_FLOAT,
		{"GL_ARB_depth_buffer_float"},
	},
	{
		ext_texture_compression_rgtc,
		ARRAY_SIZE(ext_texture_compression_rgtc),
		"GL_EXT_texture_compression_rgtc",
		GL_UNSIGNED_NORMALIZED,
		{"GL_EXT_texture_compression_rgtc"}
	},
	{
		ext_texture_compression_rgtc_signed,
		ARRAY_SIZE(ext_texture_compression_rgtc_signed),
		"GL_EXT_texture_compression_rgtc-signed",
		GL_SIGNED_NORMALIZED,
		{"GL_EXT_texture_compression_rgtc"}
	},
	{
		arb_texture_float,
		ARRAY_SIZE(arb_texture_float),
		"GL_ARB_texture_float",
		GL_FLOAT,
		{"GL_ARB_texture_float"}
	},
	{
		ati_texture_compression_3dc,
		ARRAY_SIZE(ati_texture_compression_3dc),
		"GL_ATI_texture_compression_3dc",
		GL_UNSIGNED_NORMALIZED,
		{"GL_ATI_texture_compression_3dc"}
	},
	{
		ext_texture_compression_latc,
		ARRAY_SIZE(ext_texture_compression_latc),
		"GL_EXT_texture_compression_latc",
		GL_UNSIGNED_NORMALIZED,
		{"GL_EXT_texture_compression_latc"}
	},
	{
		ext_texture_compression_latc_signed,
		ARRAY_SIZE(ext_texture_compression_latc_signed),
		"GL_EXT_texture_compression_latc-signed",
		GL_SIGNED_NORMALIZED,
		{"GL_EXT_texture_compression_latc"}
	},
	{
		ext_texture_snorm,
		ARRAY_SIZE(ext_texture_snorm),
		"GL_EXT_texture_snorm",
		GL_SIGNED_NORMALIZED,
		{"GL_EXT_texture_snorm"}
	},
	{
		arb_es2_compatibility,
		ARRAY_SIZE(arb_es2_compatibility),
		"GL_ARB_ES2_compatibility",
		GL_UNSIGNED_NORMALIZED,
		{"GL_ARB_ES2_compatibility"}
	},
	{
		arb_texture_compression_bptc_unorm,
		ARRAY_SIZE(arb_texture_compression_bptc_unorm),
		"GL_ARB_texture_compression_bptc-unorm",
		GL_UNSIGNED_NORMALIZED,
		{"GL_ARB_texture_compression_bptc"}
	},
	{
		arb_texture_compression_bptc_float,
		ARRAY_SIZE(arb_texture_compression_bptc_float),
		"GL_ARB_texture_compression_bptc-float",
		GL_FLOAT,
		{"GL_ARB_texture_compression_bptc"}
	},
	{
		arb_texture_stencil8,
		ARRAY_SIZE(arb_texture_stencil8),
		"GL_ARB_texture_stencil8",
		GL_UNSIGNED_BYTE,
		{"GL_ARB_texture_stencil8"}
	},
};

static const char *
get_format_name(GLenum format)
{
	if (format == 3)
		return "3";
	if (format == 4)
		return "4";
	return piglit_get_gl_enum_name(format);
}

static GLboolean
supported(const struct test_desc *test)
{
	unsigned i;

	for (i = 0; i < 3; i++) {
		if (test->ext[i]) {
			if (!piglit_is_extension_supported(test->ext[i])) {
				return GL_FALSE;
			}
		}
	}

	return GL_TRUE;
}

static int test_index;
static int format_index;


/**
 * If inc_dec == +1, go to  the next test set.
 * If inc_dec == -1, go to the previous test set.
 */
static void next_test_set(int inc_dec)
{
	do {
		test_index += inc_dec;
		if (test_index >= (int) ARRAY_SIZE(test_sets)) {
			test_index = 0;
		}
		else if (test_index < 0) {
			test_index = ARRAY_SIZE(test_sets) - 1;
		}
	} while (!supported(&test_sets[test_index]));
	format_index = 0;
	printf("Using test set: %s\n", test_sets[test_index].param);
}


static void fbo_formats_key_func(unsigned char key, int x, int y)
{
	switch (key) {
	case 'n': /* next test set */
		next_test_set(+1);
		break;

	case 'N': /* previous test set */
		next_test_set(-1);
		break;

	case 'm': /* next format */
		format_index++;
		if (format_index >= (int) test_sets[test_index].num_formats) {
			format_index = 0;
		}
		break;

	case 'M': /* previous format */
		format_index--;
		if (format_index < 0) {
			format_index = test_sets[test_index].num_formats - 1;
		}
		break;
	case 'f': /* next format, or next test set */
		format_index++;
		if (format_index >= (int) test_sets[test_index].num_formats) {
			next_test_set(+1);
		}
		break;
	case 'F': /* prev format, or prev test set */
		format_index--;
		if (format_index < 0) {
			next_test_set(-1);
			format_index = test_sets[test_index].num_formats - 1;
		}
		break;
	}
	piglit_escape_exit_key(key, x, y);
}

/**
 * Look up a test set name (such as "GL_EXT_texture_sRGB").
 * \return index >= 0 if found, -1 if not found
 */
static int
fbo_lookup_test_set(const char *test_set_name)
{
	int i, j;

	for (i = 0; i < (int) ARRAY_SIZE(test_sets); i++) {
		if (!strcmp(test_set_name, test_sets[i].param)) {
			for (j = 0; j < 3; j++) {
				if (test_sets[i].ext[j]) {
					piglit_require_extension(test_sets[i].ext[j]);
				}
			}

			return i;
		}
	}

	return -1;
}

static void
fbo_formats_init_test_set(int test_set_index, GLboolean print_options)
{
	if (!piglit_automatic)
		piglit_set_keyboard_func(fbo_formats_key_func);

	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_ARB_texture_env_combine");

	test_index = test_set_index;

	if (!piglit_automatic && print_options) {
		printf("    -n   Next test set.\n"
		       "    -N   Previous test set.\n"
		       "    -m   Next format in the set.\n"
		       "    -M   Previous format in the set.\n");
	}

	printf("Using test set: %s\n", test_sets[test_index].param);
}

void
fbo_formats_init(int argc, char **argv, GLboolean print_options)
{
	int test_set_index = 0;
	if (argc == 2) {
		test_set_index = fbo_lookup_test_set(argv[1]);
		if (test_set_index < 0) {
			unsigned i;
			fprintf(stderr, "Unknown test set: %s\n", argv[1]);
			fprintf(stderr, "Test sets:\n");
			for (i = 0; i < ARRAY_SIZE(test_sets); i++) {
				fprintf(stderr, "\t%s\n", test_sets[i].param);
			}
			exit(1);
		}
	} else if (argc > 2) {
		printf("More than 1 test set specified\n");
		exit(1);
	}

	fbo_formats_init_test_set(test_set_index, print_options);
}

/**
 * Try to use the given format test set (such as "GL_EXT_texture_sRGB")
 * This is an alternative to the fbo_formats_init() function above.
 * \return true if success, false if unknown test set name
 */
bool
fbo_use_test_set(const char *test_set, bool print_options)
{
	int test_set_index = fbo_lookup_test_set(test_set);
	if (test_set_index >= 0) {
		fbo_formats_init_test_set(test_set_index,
					  (GLboolean) print_options);
		return true;
	} else {
		return false;
	}
}

static void add_result(bool *all_skip, enum piglit_result *end_result,
		       enum piglit_result new_result)
{
	if (new_result != PIGLIT_SKIP)
		*all_skip = false;

	if (new_result == PIGLIT_FAIL)
		*end_result = new_result;
}

typedef enum piglit_result (*test_func)(const struct format_desc *format);

static enum piglit_result fbo_formats_display(test_func test_format)
{
	enum piglit_result result, end_result = PIGLIT_PASS;
	bool all_skip = true;
	unsigned i;

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	if (piglit_automatic) {
		for (i = 0; i < test_sets[test_index].num_formats; i++) {
			result = test_format(&test_sets[test_index].format[i]);
			add_result(&all_skip, &end_result, result);
		}
	} else {
		result = test_format(&test_sets[test_index].format[format_index]);
		add_result(&all_skip, &end_result, result);
	}

	piglit_present_results();

	if (all_skip)
		return PIGLIT_SKIP;
	return end_result;
}
