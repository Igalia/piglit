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
	char *name;
};

#define FORMAT(f) { f, #f }
static const struct format_desc core[] = {
	FORMAT(3),
	FORMAT(4),
	FORMAT(GL_RGB),
	FORMAT(GL_RGBA),
	FORMAT(GL_ALPHA),
	FORMAT(GL_LUMINANCE),
	FORMAT(GL_LUMINANCE_ALPHA),
	FORMAT(GL_INTENSITY),

	FORMAT(GL_ALPHA4),
	FORMAT(GL_ALPHA8),
	FORMAT(GL_ALPHA12),
	FORMAT(GL_ALPHA16),

	FORMAT(GL_LUMINANCE4),
	FORMAT(GL_LUMINANCE8),
	FORMAT(GL_LUMINANCE12),
	FORMAT(GL_LUMINANCE16),

	FORMAT(GL_LUMINANCE4_ALPHA4),
	FORMAT(GL_LUMINANCE8_ALPHA8),
	FORMAT(GL_LUMINANCE12_ALPHA12),
	FORMAT(GL_LUMINANCE16_ALPHA16),

	FORMAT(GL_INTENSITY4),
	FORMAT(GL_INTENSITY8),
	FORMAT(GL_INTENSITY12),
	FORMAT(GL_INTENSITY16),

	FORMAT(GL_R3_G3_B2),
	FORMAT(GL_RGB4),
	FORMAT(GL_RGB5),
	FORMAT(GL_RGB8),
	FORMAT(GL_RGB10),
	FORMAT(GL_RGB12),
	FORMAT(GL_RGB16),

	FORMAT(GL_RGBA2),
	FORMAT(GL_RGBA4),
	FORMAT(GL_RGB5_A1),
	FORMAT(GL_RGBA8),
	FORMAT(GL_RGB10_A2),
	FORMAT(GL_RGBA12),
	FORMAT(GL_RGBA16),
};

static const struct format_desc arb_depth_texture[] = {
	FORMAT(GL_DEPTH_COMPONENT),
	FORMAT(GL_DEPTH_COMPONENT16),
	FORMAT(GL_DEPTH_COMPONENT24),
	FORMAT(GL_DEPTH_COMPONENT32),
};

static const struct format_desc ext_packed_depth_stencil[] = {
	FORMAT(GL_DEPTH_STENCIL_EXT),
	FORMAT(GL_DEPTH24_STENCIL8_EXT),
};

static const struct format_desc ext_texture_srgb[] = {
	FORMAT(GL_SRGB_EXT),
	FORMAT(GL_SRGB8_EXT),
	FORMAT(GL_SRGB_ALPHA_EXT),
	FORMAT(GL_SRGB8_ALPHA8_EXT),
	FORMAT(GL_SLUMINANCE_ALPHA_EXT),
	FORMAT(GL_SLUMINANCE8_ALPHA8_EXT),
	FORMAT(GL_SLUMINANCE_EXT),
	FORMAT(GL_SLUMINANCE8_EXT),
};

static const struct format_desc ext_texture_srgb_compressed[] = {
	FORMAT(GL_COMPRESSED_SRGB_EXT),
	FORMAT(GL_COMPRESSED_SRGB_S3TC_DXT1_EXT),
	FORMAT(GL_COMPRESSED_SRGB_ALPHA_EXT),
	FORMAT(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT),
	FORMAT(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT),
	FORMAT(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT),
	FORMAT(GL_COMPRESSED_SLUMINANCE_ALPHA_EXT),
	FORMAT(GL_COMPRESSED_SLUMINANCE_EXT),
};

static const struct format_desc ext_texture_compression[] = {
	FORMAT(GL_COMPRESSED_ALPHA),
	FORMAT(GL_COMPRESSED_LUMINANCE),
	FORMAT(GL_COMPRESSED_LUMINANCE_ALPHA),
	FORMAT(GL_COMPRESSED_INTENSITY),
	FORMAT(GL_COMPRESSED_RGB),
	FORMAT(GL_COMPRESSED_RGBA),
};

static const struct format_desc tdfx_texture_compression_fxt1[] = {
	FORMAT(GL_COMPRESSED_RGB_FXT1_3DFX),
	FORMAT(GL_COMPRESSED_RGBA_FXT1_3DFX),
};

static const struct format_desc ext_texture_compression_s3tc[] = {
	FORMAT(GL_COMPRESSED_RGB_S3TC_DXT1_EXT),
	FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT),
	FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT),
	FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT),
};

static const struct format_desc ext_texture_integer[] = {
	FORMAT(GL_RGBA8UI_EXT),
	FORMAT(GL_RGBA16UI_EXT),
	FORMAT(GL_RGBA32UI_EXT),
	FORMAT(GL_RGBA8I_EXT),
	FORMAT(GL_RGBA16I_EXT),
	FORMAT(GL_RGBA32I_EXT),
	FORMAT(GL_RGB8UI_EXT),
	FORMAT(GL_RGB16UI_EXT),
	FORMAT(GL_RGB32UI_EXT),
	FORMAT(GL_RGB8I_EXT),
	FORMAT(GL_RGB16I_EXT),
	FORMAT(GL_RGB32I_EXT),
	FORMAT(GL_ALPHA8UI_EXT),
	FORMAT(GL_ALPHA16UI_EXT),
	FORMAT(GL_ALPHA32UI_EXT),
	FORMAT(GL_ALPHA8I_EXT),
	FORMAT(GL_ALPHA16I_EXT),
	FORMAT(GL_ALPHA32I_EXT),
	FORMAT(GL_INTENSITY8UI_EXT),
	FORMAT(GL_INTENSITY16UI_EXT),
	FORMAT(GL_INTENSITY32UI_EXT),
	FORMAT(GL_INTENSITY8I_EXT),
	FORMAT(GL_INTENSITY16I_EXT),
	FORMAT(GL_INTENSITY32I_EXT),
	FORMAT(GL_LUMINANCE8UI_EXT),
	FORMAT(GL_LUMINANCE16UI_EXT),
	FORMAT(GL_LUMINANCE32UI_EXT),
	FORMAT(GL_LUMINANCE8I_EXT),
	FORMAT(GL_LUMINANCE16I_EXT),
	FORMAT(GL_LUMINANCE32I_EXT),
	FORMAT(GL_LUMINANCE_ALPHA8UI_EXT),
	FORMAT(GL_LUMINANCE_ALPHA16UI_EXT),
	FORMAT(GL_LUMINANCE_ALPHA32UI_EXT),
	FORMAT(GL_LUMINANCE_ALPHA8I_EXT),
	FORMAT(GL_LUMINANCE_ALPHA16I_EXT),
	FORMAT(GL_LUMINANCE_ALPHA32I_EXT),
};

static const struct format_desc arb_texture_rg[] = {
	FORMAT(GL_R8),
	FORMAT(GL_R16),
	FORMAT(GL_RG),
	FORMAT(GL_RG8),
	FORMAT(GL_RG16),
};

static const struct format_desc arb_texture_rg_int[] = {
	FORMAT(GL_R8I),
	FORMAT(GL_R8UI),
	FORMAT(GL_R16I),
	FORMAT(GL_R16UI),
	FORMAT(GL_R32I),
	FORMAT(GL_R32UI),
	FORMAT(GL_RG_INTEGER),
	FORMAT(GL_RG8I),
	FORMAT(GL_RG8UI),
	FORMAT(GL_RG16I),
	FORMAT(GL_RG16UI),
	FORMAT(GL_RG32I),
	FORMAT(GL_RG32UI),
};

static const struct format_desc arb_texture_rg_float[] = {
	FORMAT(GL_R16F),
	FORMAT(GL_R32F),
	FORMAT(GL_RG16F),
	FORMAT(GL_RG32F),
};

static const struct format_desc ext_texture_shared_exponent[] = {
	FORMAT(GL_RGB9_E5_EXT),
};

static const struct format_desc ext_packed_float[] = {
	FORMAT(GL_R11F_G11F_B10F_EXT),
};

static const struct format_desc arb_depth_buffer_float[] = {
	FORMAT(GL_DEPTH_COMPONENT32F),
	FORMAT(GL_DEPTH32F_STENCIL8),
};

static const struct format_desc ext_texture_compression_rgtc[] = {
	FORMAT(GL_COMPRESSED_RED),
	FORMAT(GL_COMPRESSED_RED_RGTC1_EXT),
	FORMAT(GL_COMPRESSED_RG),
	FORMAT(GL_COMPRESSED_RED_GREEN_RGTC2_EXT),
};

static const struct format_desc ext_texture_compression_rgtc_signed[] = {
	FORMAT(GL_COMPRESSED_SIGNED_RED_RGTC1_EXT),
	FORMAT(GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT),
};

static const struct format_desc arb_texture_float[] = {
	FORMAT(GL_RGB16F_ARB),
	FORMAT(GL_RGBA16F_ARB),
	FORMAT(GL_ALPHA16F_ARB),
	FORMAT(GL_LUMINANCE16F_ARB),
	FORMAT(GL_LUMINANCE_ALPHA16F_ARB),
	FORMAT(GL_INTENSITY16F_ARB),
	FORMAT(GL_RGB32F_ARB),
	FORMAT(GL_RGBA32F_ARB),
	FORMAT(GL_ALPHA32F_ARB),
	FORMAT(GL_LUMINANCE32F_ARB),
	FORMAT(GL_LUMINANCE_ALPHA32F_ARB),
	FORMAT(GL_INTENSITY32F_ARB)
};

static const struct format_desc ati_texture_compression_3dc[] = {
	FORMAT(GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI)
};

static const struct format_desc ext_texture_compression_latc[] = {
	FORMAT(GL_COMPRESSED_LUMINANCE_LATC1_EXT),
	FORMAT(GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT),
};

static const struct format_desc ext_texture_compression_latc_signed[] = {
	FORMAT(GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT),
	FORMAT(GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT)
};

static const struct format_desc ext_texture_snorm[] = {
        FORMAT(GL_RED_SNORM),
        FORMAT(GL_RG_SNORM),
        FORMAT(GL_RGB_SNORM),
        FORMAT(GL_RGBA_SNORM),
        FORMAT(GL_R8_SNORM),
        FORMAT(GL_RG8_SNORM),
        FORMAT(GL_RGB8_SNORM),
        FORMAT(GL_RGBA8_SNORM),
        FORMAT(GL_R16_SNORM),
        FORMAT(GL_RG16_SNORM),
        FORMAT(GL_RGB16_SNORM),
        FORMAT(GL_RGBA16_SNORM),
        FORMAT(GL_ALPHA_SNORM),
        FORMAT(GL_LUMINANCE_SNORM),
        FORMAT(GL_LUMINANCE_ALPHA_SNORM),
        FORMAT(GL_INTENSITY_SNORM),
        FORMAT(GL_ALPHA8_SNORM),
        FORMAT(GL_LUMINANCE8_SNORM),
        FORMAT(GL_LUMINANCE8_ALPHA8_SNORM),
        FORMAT(GL_INTENSITY8_SNORM),
        FORMAT(GL_ALPHA16_SNORM),
        FORMAT(GL_LUMINANCE16_SNORM),
        FORMAT(GL_LUMINANCE16_ALPHA16_SNORM),
        FORMAT(GL_INTENSITY16_SNORM)
};

struct test_desc {
	const struct format_desc *format;
	unsigned num_formats;
	const char *param;
	GLenum basetype;
	const char *ext[3];
	GLenum base;
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
		GL_DEPTH_COMPONENT,
	},
	{
		ext_packed_depth_stencil,
		ARRAY_SIZE(ext_packed_depth_stencil),
		"GL_EXT_packed_depth_stencil",
		GL_UNSIGNED_NORMALIZED,
		{"GL_EXT_packed_depth_stencil"},
		GL_DEPTH_STENCIL,
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
		/* XXX this extension consists of both DEPTH_COMPONENT and
		 * DEPTH_STENCIL formats. */
		GL_DEPTH_COMPONENT,
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
};

static GLboolean
supported(const struct test_desc *test)
{
	unsigned i;

	for (i = 0; i < 3; i++) {
		if (test->ext[i]) {
			if (!glutExtensionSupported(test->ext[i])) {
				return GL_FALSE;
			}
		}
	}

	return GL_TRUE;
}
