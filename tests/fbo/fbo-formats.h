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

#ifndef GL_ARB_framebuffer_object
#define GL_DEPTH_STENCIL 0x84F9
#endif

#ifndef GL_ARB_depth_buffer_float
#define GL_DEPTH_COMPONENT32F 0x8CAC
#define GL_DEPTH32F_STENCIL8 0x8CAD
#endif

#ifndef GL_ARB_texture_rg
#define GL_RG 0x8227
#define GL_RG_INTEGER 0x8228
#define GL_R8 0x8229
#define GL_R16 0x822A
#define GL_RG8 0x822B
#define GL_RG16 0x822C
#define GL_R16F 0x822D
#define GL_R32F 0x822E
#define GL_RG16F 0x822F
#define GL_RG32F 0x8230
#define GL_R8I 0x8231
#define GL_R8UI 0x8232
#define GL_R16I 0x8233
#define GL_R16UI 0x8234
#define GL_R32I 0x8235
#define GL_R32UI 0x8236
#define GL_RG8I 0x8237
#define GL_RG8UI 0x8238
#define GL_RG16I 0x8239
#define GL_RG16UI 0x823A
#define GL_RG32I 0x823B
#define GL_RG32UI 0x823C
#endif

#ifndef GL_VERSION_3_0
#define GL_COMPRESSED_RED 0x8225
#define GL_COMPRESSED_RG 0x8226
#endif

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
	FORMAT(GL_COMPRESSED_SIGNED_RED_RGTC1_EXT),
	FORMAT(GL_COMPRESSED_RG),
	FORMAT(GL_COMPRESSED_RED_GREEN_RGTC2_EXT),
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

struct test_desc {
	const struct format_desc *format;
	unsigned num_formats;
	const char *param;
	const char *ext[3];
	GLenum base;
};

static const struct test_desc test_sets[] = {
	{
		core,
		ARRAY_SIZE(core),
		"Core formats"
	},
	{
		ext_texture_compression,
		ARRAY_SIZE(ext_texture_compression),
		"GL_ARB_texture_compression",
		{"GL_ARB_texture_compression"}
	},
	{
		tdfx_texture_compression_fxt1,
		ARRAY_SIZE(tdfx_texture_compression_fxt1),
		"GL_3DFX_texture_compression_FXT1",
		{"GL_ARB_texture_compression",
		 "GL_3DFX_texture_compression_FXT1"},
	},
	{
		ext_texture_compression_s3tc,
		ARRAY_SIZE(ext_texture_compression_s3tc),
		"GL_EXT_texture_compression_s3tc",
		{"GL_ARB_texture_compression",
		 "GL_EXT_texture_compression_s3tc"},
	},
	{
		arb_depth_texture,
		ARRAY_SIZE(arb_depth_texture),
		"GL_ARB_depth_texture",
		{"GL_ARB_depth_texture"},
		GL_DEPTH_COMPONENT,
	},
	{
		ext_packed_depth_stencil,
		ARRAY_SIZE(ext_packed_depth_stencil),
		"GL_EXT_packed_depth_stencil",
		{"GL_EXT_packed_depth_stencil"},
		GL_DEPTH_STENCIL,
	},
	{
		ext_texture_srgb,
		ARRAY_SIZE(ext_texture_srgb),
		"GL_EXT_texture_sRGB",
		{"GL_EXT_texture_sRGB"}
	},
	{
		ext_texture_srgb_compressed,
		ARRAY_SIZE(ext_texture_srgb_compressed),
		"GL_EXT_texture_sRGB-s3tc",
		{"GL_EXT_texture_sRGB",
		 "GL_ARB_texture_compression",
		 "GL_EXT_texture_compression_s3tc"},
	},
	{
		ext_texture_integer,
		ARRAY_SIZE(ext_texture_integer),
		"GL_EXT_texture_integer",
		{"GL_EXT_texture_integer"}
	},
	{
		arb_texture_rg,
		ARRAY_SIZE(arb_texture_rg),
		"GL_ARB_texture_rg",
		{"GL_ARB_texture_rg"}
	},
	{
		arb_texture_rg_int,
		ARRAY_SIZE(arb_texture_rg_int),
		"GL_ARB_texture_rg-int",
		{"GL_ARB_texture_rg",
		 "GL_EXT_texture_integer"}
	},
	{
		arb_texture_rg_float,
		ARRAY_SIZE(arb_texture_rg_float),
		"GL_ARB_texture_rg-float",
		{"GL_ARB_texture_rg",
		 "GL_ARB_texture_float"}
	},
	{
		ext_texture_shared_exponent,
		ARRAY_SIZE(ext_texture_shared_exponent),
		"GL_EXT_texture_shared_exponent",
		{"GL_EXT_texture_shared_exponent"}
	},
	{
		ext_packed_float,
		ARRAY_SIZE(ext_packed_float),
		"GL_EXT_packed_float",
		{"GL_EXT_packed_float"}
	},
	{
		arb_depth_buffer_float,
		ARRAY_SIZE(arb_depth_buffer_float),
		"GL_ARB_depth_buffer_float",
		{"GL_ARB_depth_buffer_float"},
		GL_DEPTH_COMPONENT,
	},
	{
		ext_texture_compression_rgtc,
		ARRAY_SIZE(ext_texture_compression_rgtc),
		"GL_EXT_texture_compression_rgtc",
		{"GL_EXT_texture_compression_rgtc"}
	},
	{
		arb_texture_float,
		ARRAY_SIZE(arb_texture_float),
		"GL_ARB_texture_float",
		{"GL_ARB_texture_float"}
	},
};
