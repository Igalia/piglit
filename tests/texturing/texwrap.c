/*
 * Copyright © 2001 Brian Paul
 * Copyright © 2010 Marek Olšák <maraeo@gmail.com>
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* Based on the Mesa demo "texwrap" by Brian Paul.
 * Reworked and extended by Marek Olšák.
 *
 * This is more than just a test of wrap modes.
 *
 * Besides all the wrap modes, it tests:
 *
 * - 1D, 2D, 3D, and RECT texture targets.
 *
 * - Many formats, see the list below.
 *   Especially the border color might need to be set up differently
 *   for each format in hardware. Also, some hardware might not support
 *   clamp-to-border and clamp for some formats. We need to make sure all
 *   useful formats are appropriately covered here.
 *   The test is skipped if the format chosen by GL is not the same
 *   as the requested format.
 *
 * - Non-power-of-two textures.
 *   Some drivers have a special shader-based code path for NPOT textures.
 *
 * - Projective texture mapping.
 *   This is also useful to verify the correctness of shader-based wrap modes
 *   for some hardware.
 *
 ****************************************************************************
 *
 * Parameters:
 *   One of: 1D, 2D, 3D, RECT
 *   One of: See the list of formats below.
 *   Any of: npot border proj
 *
 * Examples:
 *   3D GL_RGBA8 border
 *   2D GL_RGBA16F npot
 *   RECT GL_RGB10_A2
 *
 * Default:
 *   2D GL_RGBA8
 */

#include "piglit-util-gl.h"
#include <limits.h>

/* Only *_ARB versions of these exist. I am lazy to add the suffix. */
#define GL_ALPHA32F                     0x8816
#define GL_INTENSITY32F                 0x8817
#define GL_LUMINANCE32F                 0x8818
#define GL_LUMINANCE_ALPHA32F           0x8819
#define GL_ALPHA16F                     0x881C
#define GL_INTENSITY16F                 0x881D
#define GL_LUMINANCE16F                 0x881E
#define GL_LUMINANCE_ALPHA16F           0x881F

/* Only *_EXT versions of these exist. I am lazy to add the suffix. */
#define GL_COMPRESSED_RGB_S3TC_DXT1 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5 0x83F3
#define GL_COMPRESSED_LUMINANCE_LATC1 0x8C70
#define GL_COMPRESSED_SIGNED_LUMINANCE_LATC1 0x8C71
#define GL_COMPRESSED_LUMINANCE_ALPHA_LATC2 0x8C72
#define GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2 0x8C73
#define GL_COMPRESSED_SRGB_S3TC_DXT1 0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5 0x8C4F

/* Only *_ATI versions of these exist. It's nicer without the suffix. */
#define GL_COMPRESSED_LUMINANCE_ALPHA_3DC 0x8837

/* Formats. */

#define FORMAT(f) #f, f

enum {
	FLOAT_TYPE,
	INT_TYPE,
	UINT_TYPE
};

struct format_desc {
	const char  *name;
	GLenum      internalformat;
	int         red, green, blue, alpha, luminance, intensity, depth, stencil;
	GLboolean   compressed, srgb;
	int         type;
};

struct test_desc {
	const struct format_desc *format;
	unsigned    num_formats;
	const char  *name;
	const char  *ext[3];
};

static const struct format_desc core[] = {
	{FORMAT(GL_RGBA8),              8, 8, 8, 8, 0, 0, 0, 0,     0},
	{FORMAT(GL_RGBA2),              2, 2, 2, 2, 0, 0, 0, 0,     0},
	{FORMAT(GL_R3_G3_B2),           3, 3, 2, 0, 0, 0, 0, 0,     0},
	{FORMAT(GL_RGB4),               4, 4, 4, 0, 0, 0, 0, 0,     0},
	{FORMAT(GL_RGBA4),              4, 4, 4, 4, 0, 0, 0, 0,     0},
	{FORMAT(GL_RGB5),               5, 5, 5, 0, 0, 0, 0, 0,     0},
	{FORMAT(GL_RGB5_A1),            5, 5, 5, 1, 0, 0, 0, 0,     0},
	{FORMAT(GL_RGB8),               8, 8, 8, 0, 0, 0, 0, 0,     0},
	{FORMAT(GL_RGB10),              10, 10, 10, 0, 0, 0, 0, 0,  0},
	{FORMAT(GL_RGB10_A2),           10, 10, 10, 2, 0, 0, 0, 0,  0},
	{FORMAT(GL_RGB12),              12, 12, 12, 0, 0, 0, 0, 0,  0},
	{FORMAT(GL_RGBA12),             12, 12, 12, 12, 0, 0, 0, 0, 0},
	{FORMAT(GL_RGB16),              16, 16, 16, 0, 0, 0, 0, 0,  0},
	{FORMAT(GL_RGBA16),             16, 16, 16, 16, 0, 0, 0, 0, 0},
	{FORMAT(GL_ALPHA4),             0, 0, 0, 4, 0, 0, 0, 0,     0},
	{FORMAT(GL_ALPHA8),             0, 0, 0, 8, 0, 0, 0, 0,     0},
	{FORMAT(GL_ALPHA12),            0, 0, 0, 12, 0, 0, 0, 0,    0},
	{FORMAT(GL_ALPHA16),            0, 0, 0, 16, 0, 0, 0, 0,    0},
	{FORMAT(GL_LUMINANCE4),         0, 0, 0, 0, 4, 0, 0, 0,     0},
	{FORMAT(GL_LUMINANCE8),         0, 0, 0, 0, 8, 0, 0, 0,     0},
	{FORMAT(GL_LUMINANCE12),        0, 0, 0, 0, 12, 0, 0, 0,    0},
	{FORMAT(GL_LUMINANCE16),        0, 0, 0, 0, 16, 0, 0, 0,    0},
	{FORMAT(GL_LUMINANCE4_ALPHA4),  0, 0, 0, 4, 4, 0, 0, 0,     0},
	{FORMAT(GL_LUMINANCE6_ALPHA2),  0, 0, 0, 2, 6, 0, 0, 0,     0},
	{FORMAT(GL_LUMINANCE8_ALPHA8),  0, 0, 0, 8, 8, 0, 0, 0,     0},
	{FORMAT(GL_LUMINANCE12_ALPHA4), 0, 0, 0, 4, 12, 0, 0, 0,    0},
	{FORMAT(GL_LUMINANCE12_ALPHA12),0, 0, 0, 12, 12, 0, 0, 0,   0},
	{FORMAT(GL_LUMINANCE16_ALPHA16),0, 0, 0, 16, 16, 0, 0, 0,   0},
	{FORMAT(GL_INTENSITY4),         0, 0, 0, 0, 0, 4, 0, 0,     0},
	{FORMAT(GL_INTENSITY8),         0, 0, 0, 0, 0, 8, 0, 0,     0},
	{FORMAT(GL_INTENSITY12),        0, 0, 0, 0, 0, 12, 0, 0,    0},
	{FORMAT(GL_INTENSITY16),        0, 0, 0, 0, 0, 16, 0, 0,    0},
};

static const struct format_desc ext_texture_srgb[] = {
	{FORMAT(GL_SRGB8_ALPHA8),       8, 8, 8, 8, 0, 0, 0, 0,     0, 1},
	{FORMAT(GL_SRGB8),              8, 8, 8, 0, 0, 0, 0, 0,     0, 1},
	{FORMAT(GL_SLUMINANCE8),        0, 0, 0, 0, 8, 0, 0, 0,     0, 1},
	{FORMAT(GL_SLUMINANCE8_ALPHA8), 0, 0, 0, 8, 8, 0, 0, 0,     0, 1}
};

static const struct format_desc arb_depth_texture[] = {
	{FORMAT(GL_DEPTH_COMPONENT16),  0, 0, 0, 0, 0, 0, 16, 0,    0},
	{FORMAT(GL_DEPTH_COMPONENT24),  0, 0, 0, 0, 0, 0, 24, 0,    0},
	{FORMAT(GL_DEPTH_COMPONENT32),  0, 0, 0, 0, 0, 0, 32, 0,    0},
};

static const struct format_desc ext_packed_depth_stencil[] = {
	{FORMAT(GL_DEPTH24_STENCIL8),  0, 0, 0, 0, 0, 0, 24, 8,    0},
};

static const struct format_desc arb_depth_buffer_float[] = {
	{FORMAT(GL_DEPTH32F_STENCIL8),  0, 0, 0, 0, 0, 0, 32, 8,    0},
	{FORMAT(GL_DEPTH_COMPONENT32F),  0, 0, 0, 0, 0, 0, 32, 0,    0},
};

static const struct format_desc arb_texture_compression[] = {
	{FORMAT(GL_COMPRESSED_ALPHA), 0, 0, 0, 4, 0, 0, 0, 0, 1},
	{FORMAT(GL_COMPRESSED_LUMINANCE), 0, 0, 0, 0, 4, 0, 0, 0, 1},
	{FORMAT(GL_COMPRESSED_LUMINANCE_ALPHA), 0, 0, 0, 4, 4, 0, 0, 0, 1},
	{FORMAT(GL_COMPRESSED_INTENSITY), 0, 0, 0, 0, 0, 4, 0, 0, 1},
	{FORMAT(GL_COMPRESSED_RGB), 4, 4, 4, 0, 0, 0, 0, 0, 1},
	{FORMAT(GL_COMPRESSED_RGBA), 4, 4, 4, 4, 0, 0, 0, 0, 1},
};

static const struct format_desc ext_texture_compression_s3tc[] = {
	{FORMAT(GL_COMPRESSED_RGB_S3TC_DXT1), 4, 4, 4, 0, 0, 0, 0, 0, 1},
	{FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT1), 4, 4, 4, 1, 0, 0, 0, 0, 1},
	{FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT3), 4, 4, 4, 4, 0, 0, 0, 0, 1},
	{FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT5), 4, 4, 4, 4, 0, 0, 0, 0, 1},
};

static const struct format_desc arb_texture_compression_bptc[] = {
	{FORMAT(GL_COMPRESSED_RGBA_BPTC_UNORM), 4, 4, 4, 4, 0, 0, 0, 0, 1},
	{FORMAT(GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM), 4, 4, 4, 4, 0, 0, 0, 0, 1, 1},
	{FORMAT(GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT), 4, 4, 4, 0, 0, 0, 0, 0, 1, 0, FLOAT_TYPE},
	{FORMAT(GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT), 4, 4, 4, 0, 0, 0, 0, 0, 1, 0, FLOAT_TYPE},
};

static const struct format_desc ext_texture_srgb_compressed[] = {
	{FORMAT(GL_COMPRESSED_SRGB), 4, 4, 4, 0, 0, 0, 0, 0, 1, 1},
	{FORMAT(GL_COMPRESSED_SRGB_ALPHA), 4, 4, 4, 4, 0, 0, 0, 0, 1, 1},
	{FORMAT(GL_COMPRESSED_SLUMINANCE), 0, 0, 0, 0, 4, 0, 0, 0, 1, 1},
	{FORMAT(GL_COMPRESSED_SLUMINANCE_ALPHA), 0, 0, 0, 4, 4, 0, 0, 0, 1, 1},
	{FORMAT(GL_COMPRESSED_SRGB_S3TC_DXT1), 4, 4, 4, 0, 0, 0, 0, 0, 1, 1},
	{FORMAT(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1), 4, 4, 4, 1, 0, 0, 0, 0, 1, 1},
	{FORMAT(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3), 4, 4, 4, 4, 0, 0, 0, 0, 1, 1},
	{FORMAT(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5), 4, 4, 4, 4, 0, 0, 0, 0, 1, 1},
};

static const struct format_desc ext_texture_compression_rgtc[] = {
	{FORMAT(GL_COMPRESSED_RED_RGTC1), 4, 0, 0, 0, 0, 0, 0, 0,   1},
	{FORMAT(GL_COMPRESSED_SIGNED_RED_RGTC1), 3, 0, 0, 0, 0, 0, 0, 0, 1},
	{FORMAT(GL_COMPRESSED_RG_RGTC2), 4, 4, 0, 0, 0, 0, 0, 0,    1},
	{FORMAT(GL_COMPRESSED_SIGNED_RG_RGTC2), 4, 4, 0, 0, 0, 0, 0, 0, 1},
};

static const struct format_desc arb_texture_float[] = {
	{FORMAT(GL_ALPHA16F),           0, 0, 0, 16, 0, 0, 0, 0,    0},
	{FORMAT(GL_LUMINANCE16F),       0, 0, 0, 0, 16, 0, 0, 0,    0},
	{FORMAT(GL_LUMINANCE_ALPHA16F), 0, 0, 0, 16, 16, 0, 0, 0,   0},
	{FORMAT(GL_INTENSITY16F),       0, 0, 0, 0, 0, 16, 0, 0,    0},
	{FORMAT(GL_RGB16F),             16, 16, 16, 0, 0, 0, 0, 0,  0},
	{FORMAT(GL_RGBA16F),            16, 16, 16, 16, 0, 0, 0, 0, 0},
	{FORMAT(GL_ALPHA32F),           0, 0, 0, 32, 0, 0, 0, 0,    0},
	{FORMAT(GL_LUMINANCE32F),       0, 0, 0, 0, 32, 0, 0, 0,    0},
	{FORMAT(GL_LUMINANCE_ALPHA32F), 0, 0, 0, 32, 32, 0, 0, 0,   0},
	{FORMAT(GL_INTENSITY32F),       0, 0, 0, 0, 0, 32, 0, 0,    0},
	{FORMAT(GL_RGB32F),             32, 32, 32, 0, 0, 0, 0, 0,  0},
	{FORMAT(GL_RGBA32F),            32, 32, 32, 32, 0, 0, 0, 0, 0},
};

static const struct format_desc ext_texture_integer[] = {
	{FORMAT(GL_RGB8I),              8, 8, 8, 0, 0, 0, 0, 0,    0, 0, INT_TYPE},
	{FORMAT(GL_RGBA8I),             8, 8, 8, 8, 0, 0, 0, 0,    0, 0, INT_TYPE},
	{FORMAT(GL_ALPHA8I_EXT),            0, 0, 0, 8, 0, 0, 0, 0,    0, 0, INT_TYPE},
	{FORMAT(GL_LUMINANCE8I_EXT),        0, 0, 0, 0, 8, 0, 0, 0,    0, 0, INT_TYPE},
	{FORMAT(GL_LUMINANCE_ALPHA8I_EXT),  0, 0, 0, 8, 8, 0, 0, 0,    0, 0, INT_TYPE},
	{FORMAT(GL_INTENSITY8I_EXT),        0, 0, 0, 0, 0, 8, 0, 0,    0, 0, INT_TYPE},

	{FORMAT(GL_RGB16I),             16, 16, 16, 0, 0, 0, 0, 0,  0, 0, INT_TYPE},
	{FORMAT(GL_RGBA16I),            16, 16, 16, 16, 0, 0, 0, 0, 0, 0, INT_TYPE},
	{FORMAT(GL_ALPHA16I_EXT),           0, 0, 0, 16, 0, 0, 0, 0,    0, 0, INT_TYPE},
	{FORMAT(GL_LUMINANCE16I_EXT),       0, 0, 0, 0, 16, 0, 0, 0,    0, 0, INT_TYPE},
	{FORMAT(GL_LUMINANCE_ALPHA16I_EXT), 0, 0, 0, 16, 16, 0, 0, 0,   0, 0, INT_TYPE},
	{FORMAT(GL_INTENSITY16I_EXT),       0, 0, 0, 0, 0, 16, 0, 0,    0, 0, INT_TYPE},

	{FORMAT(GL_RGB32I),             32, 32, 32, 0, 0, 0, 0, 0,  0, 0, INT_TYPE},
	{FORMAT(GL_RGBA32I),            32, 32, 32, 32, 0, 0, 0, 0, 0, 0, INT_TYPE},
	{FORMAT(GL_ALPHA32I_EXT),           0, 0, 0, 32, 0, 0, 0, 0,    0, 0, INT_TYPE},
	{FORMAT(GL_LUMINANCE32I_EXT),       0, 0, 0, 0, 32, 0, 0, 0,    0, 0, INT_TYPE},
	{FORMAT(GL_LUMINANCE_ALPHA32I_EXT), 0, 0, 0, 32, 32, 0, 0, 0,   0, 0, INT_TYPE},
	{FORMAT(GL_INTENSITY32I_EXT),       0, 0, 0, 0, 0, 32, 0, 0,    0, 0, INT_TYPE},

	{FORMAT(GL_RGB8UI),              8, 8, 8, 0, 0, 0, 0, 0,    0, 0, UINT_TYPE},
	{FORMAT(GL_RGBA8UI),             8, 8, 8, 8, 0, 0, 0, 0,    0, 0, UINT_TYPE},
	{FORMAT(GL_ALPHA8UI_EXT),            0, 0, 0, 8, 0, 0, 0, 0,    0, 0, UINT_TYPE},
	{FORMAT(GL_LUMINANCE8UI_EXT),        0, 0, 0, 0, 8, 0, 0, 0,    0, 0, UINT_TYPE},
	{FORMAT(GL_LUMINANCE_ALPHA8UI_EXT),  0, 0, 0, 8, 8, 0, 0, 0,    0, 0, UINT_TYPE},
	{FORMAT(GL_INTENSITY8UI_EXT),        0, 0, 0, 0, 0, 8, 0, 0,    0, 0, UINT_TYPE},

	{FORMAT(GL_RGB16UI),             16, 16, 16, 0, 0, 0, 0, 0,  0, 0, UINT_TYPE},
	{FORMAT(GL_RGBA16UI),            16, 16, 16, 16, 0, 0, 0, 0, 0, 0, UINT_TYPE},
	{FORMAT(GL_ALPHA16UI_EXT),           0, 0, 0, 16, 0, 0, 0, 0,    0, 0, UINT_TYPE},
	{FORMAT(GL_LUMINANCE16UI_EXT),       0, 0, 0, 0, 16, 0, 0, 0,    0, 0, UINT_TYPE},
	{FORMAT(GL_LUMINANCE_ALPHA16UI_EXT), 0, 0, 0, 16, 16, 0, 0, 0,   0, 0, UINT_TYPE},
	{FORMAT(GL_INTENSITY16UI_EXT),       0, 0, 0, 0, 0, 16, 0, 0,    0, 0, UINT_TYPE},

	{FORMAT(GL_RGB32UI),             32, 32, 32, 0, 0, 0, 0, 0,  0, 0, UINT_TYPE},
	{FORMAT(GL_RGBA32UI),            32, 32, 32, 32, 0, 0, 0, 0, 0, 0, UINT_TYPE},
	{FORMAT(GL_ALPHA32UI_EXT),           0, 0, 0, 32, 0, 0, 0, 0,    0, 0, UINT_TYPE},
	{FORMAT(GL_LUMINANCE32UI_EXT),       0, 0, 0, 0, 32, 0, 0, 0,    0, 0, UINT_TYPE},
	{FORMAT(GL_LUMINANCE_ALPHA32UI_EXT), 0, 0, 0, 32, 32, 0, 0, 0,   0, 0, UINT_TYPE},
	{FORMAT(GL_INTENSITY32UI_EXT),       0, 0, 0, 0, 0, 32, 0, 0,    0, 0, UINT_TYPE},
};

static const struct format_desc arb_texture_rg[] = {
	{FORMAT(GL_R8),                 8, 0, 0, 0, 0, 0, 0, 0,     0},
	{FORMAT(GL_RG8),                8, 8, 0, 0, 0, 0, 0, 0,     0},
	{FORMAT(GL_R16),                16, 0, 0, 0, 0, 0, 0, 0,    0},
	{FORMAT(GL_RG16),               16, 16, 0, 0, 0, 0, 0, 0,   0},
};

static const struct format_desc arb_texture_rg_float[] = {
	{FORMAT(GL_R16F),               16, 0, 0, 0, 0, 0, 0, 0,    0},
	{FORMAT(GL_RG16F),              16, 16, 0, 0, 0, 0, 0, 0,   0},
	{FORMAT(GL_R32F),               32, 0, 0, 0, 0, 0, 0, 0,    0},
	{FORMAT(GL_RG32F),              32, 32, 0, 0, 0, 0, 0, 0,   0},
};

static const struct format_desc arb_texture_rg_int[] = {
	{FORMAT(GL_R8I),                8, 0, 0, 0, 0, 0, 0, 0,     0, 0, INT_TYPE},
	{FORMAT(GL_RG8I),               8, 8, 0, 0, 0, 0, 0, 0,     0, 0, INT_TYPE},
	{FORMAT(GL_R16I),               16, 0, 0, 0, 0, 0, 0, 0,    0, 0, INT_TYPE},
	{FORMAT(GL_RG16I),              16, 16, 0, 0, 0, 0, 0, 0,   0, 0, INT_TYPE},
	{FORMAT(GL_R32I),               32, 0, 0, 0, 0, 0, 0, 0,    0, 0, INT_TYPE},
	{FORMAT(GL_RG32I),              32, 32, 0, 0, 0, 0, 0, 0,   0, 0, INT_TYPE},
	{FORMAT(GL_R8UI),               8, 0, 0, 0, 0, 0, 0, 0,     0, 0, UINT_TYPE},
	{FORMAT(GL_RG8UI),              8, 8, 0, 0, 0, 0, 0, 0,     0, 0, UINT_TYPE},
	{FORMAT(GL_R16UI),              16, 0, 0, 0, 0, 0, 0, 0,    0, 0, UINT_TYPE},
	{FORMAT(GL_RG16UI),             16, 16, 0, 0, 0, 0, 0, 0,   0, 0, UINT_TYPE},
	{FORMAT(GL_R32UI),              32, 0, 0, 0, 0, 0, 0, 0,    0, 0, UINT_TYPE},
	{FORMAT(GL_RG32UI),             32, 32, 0, 0, 0, 0, 0, 0,   0, 0, UINT_TYPE},
};

static const struct format_desc ext_packed_float[] = {
	{FORMAT(GL_R11F_G11F_B10F),     6, 6, 5, 0, 0, 0, 0, 0,  0},
};

static const struct format_desc ext_texture_compression_latc[] = {
	{FORMAT(GL_COMPRESSED_LUMINANCE_LATC1), 0, 0, 0, 0, 4, 0, 0, 0, 1},
	{FORMAT(GL_COMPRESSED_SIGNED_LUMINANCE_LATC1), 0, 0, 0, 0, 3, 0, 0, 0, 1},
	{FORMAT(GL_COMPRESSED_LUMINANCE_ALPHA_LATC2), 0, 0, 0, 4, 4, 0, 0, 0, 1},
	{FORMAT(GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2), 0, 0, 0, 4, 4, 0, 0, 0, 1},
};

static const struct format_desc ext_texture_shared_exponent[] = {
	{FORMAT(GL_RGB9_E5),            9, 9, 9, 0, 0, 0, 0, 0,     0},
};

static const struct format_desc ati_texture_compression_3dc[] = {
	{FORMAT(GL_COMPRESSED_LUMINANCE_ALPHA_3DC), 0, 0, 0, 4, 4, 0, 0, 0, 1},
};

static const struct format_desc ext_texture_snorm[] = {
	{FORMAT(GL_R8_SNORM),                 7, 0, 0, 0, 0, 0, 0, 0,     0},
	{FORMAT(GL_RG8_SNORM),                7, 7, 0, 0, 0, 0, 0, 0,     0},
	{FORMAT(GL_RGB8_SNORM),               7, 7, 7, 0, 0, 0, 0, 0,     0},
	{FORMAT(GL_RGBA8_SNORM),              7, 7, 7, 7, 0, 0, 0, 0,     0},
	{FORMAT(GL_R16_SNORM),                16, 0, 0, 0, 0, 0, 0, 0,    0},
	{FORMAT(GL_RG16_SNORM),               16, 16, 0, 0, 0, 0, 0, 0,   0},
	{FORMAT(GL_RGB16_SNORM),              16, 16, 16, 0, 0, 0, 0, 0,   0},
	{FORMAT(GL_RGBA16_SNORM),             16, 16, 16, 16, 0, 0, 0, 0,   0},
	{FORMAT(GL_ALPHA8_SNORM),             0, 0, 0, 7, 0, 0, 0, 0,     0},
	{FORMAT(GL_ALPHA16_SNORM),            0, 0, 0, 16, 0, 0, 0, 0,     0},
	{FORMAT(GL_LUMINANCE8_SNORM),         0, 0, 0, 0, 7, 0, 0, 0,     0},
	{FORMAT(GL_LUMINANCE16_SNORM),        0, 0, 0, 0, 16, 0, 0, 0,     0},
	{FORMAT(GL_LUMINANCE8_ALPHA8_SNORM),  0, 0, 0, 7, 7, 0, 0, 0,     0},
	{FORMAT(GL_LUMINANCE16_ALPHA16_SNORM),0, 0, 0, 16, 16, 0, 0, 0,     0},
	{FORMAT(GL_INTENSITY8_SNORM),         0, 0, 0, 0, 0, 8, 0, 0,     0},
	{FORMAT(GL_INTENSITY16_SNORM),        0, 0, 0, 0, 0, 16, 0, 0,     0},
};

static const struct format_desc arb_es2_compatibility[] = {
	{FORMAT(GL_RGB565),                   5, 6, 5, 0, 0, 0, 0, 0,     0}
};

static const struct format_desc arb_texture_rgb10_a2ui[] = {
	{FORMAT(GL_RGB10_A2UI),           10, 10, 10, 2, 0, 0, 0, 0,  0, 0, UINT_TYPE},
};

static const struct test_desc test_sets[] = {
	{
		core,
		ARRAY_SIZE(core),
		"Core formats",
	},
	{
		arb_texture_compression,
		ARRAY_SIZE(arb_texture_compression),
		"GL_ARB_texture_compression",
		{"GL_ARB_texture_compression"}
	},
	{
		ext_texture_compression_s3tc,
		ARRAY_SIZE(ext_texture_compression_s3tc),
		"GL_EXT_texture_compression_s3tc",
		{"GL_EXT_texture_compression_s3tc"},
	},
	{
		arb_texture_compression_bptc,
		ARRAY_SIZE(arb_texture_compression_bptc),
		"GL_ARB_texture_compression_bptc",
		{"GL_ARB_texture_compression_bptc"},
	},
	{
		arb_depth_texture,
		ARRAY_SIZE(arb_depth_texture),
		"GL_ARB_depth_texture",
		{"GL_ARB_depth_texture"},
	},
	{
		ext_packed_depth_stencil,
		ARRAY_SIZE(ext_packed_depth_stencil),
		"GL_EXT_packed_depth_stencil",
		{"GL_EXT_packed_depth_stencil",
		 "GL_ARB_depth_texture"},
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
		arb_texture_rgb10_a2ui,
		ARRAY_SIZE(arb_texture_rgb10_a2ui),
		"GL_ARB_texture_rgb10_a2ui",
		{"GL_ARB_texture_rgb10_a2ui"}
	},
	{
		arb_texture_rg,
		ARRAY_SIZE(arb_texture_rg),
		"GL_ARB_texture_rg",
		{"GL_ARB_texture_rg"}
	},
	{
		arb_texture_rg_float,
		ARRAY_SIZE(arb_texture_rg_float),
		"GL_ARB_texture_rg-float",
		{"GL_ARB_texture_rg",
		 "GL_ARB_texture_float"}
	},
	{
		arb_texture_rg_int,
		ARRAY_SIZE(arb_texture_rg_int),
		"GL_ARB_texture_rg-int",
		{"GL_ARB_texture_rg",
		 "GL_EXT_texture_integer"}
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
	{
		ati_texture_compression_3dc,
		ARRAY_SIZE(ati_texture_compression_3dc),
		"GL_ATI_texture_compression_3dc",
		{"GL_ATI_texture_compression_3dc"}
	},
	{
		ext_texture_compression_latc,
		ARRAY_SIZE(ext_texture_compression_latc),
		"GL_EXT_texture_compression_latc",
		{"GL_EXT_texture_compression_latc"}
	},
	{
		ext_texture_snorm,
		ARRAY_SIZE(ext_texture_snorm),
		"GL_EXT_texture_snorm",
		{"GL_EXT_texture_snorm"}
	},
	{
		arb_es2_compatibility,
		ARRAY_SIZE(arb_es2_compatibility),
		"GL_ARB_ES2_compatibility",
		{"GL_ARB_ES2_compatibility"}
	},

	{NULL}
};

/* Indexed by the channel bitdepth. */
static unsigned nearest_deltamax[33] = {
	1, 210, 128, 32, 17, 9, 9, 9,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1
};

static unsigned linear_deltamax[33] = {
	1, 210, 128, 32, 17, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10,
	10
};

/* Wrap modes. */

struct wrap_mode {
	GLenum      mode;
	const char  *name;
	GLboolean   valid_for_rect;
	const char  *extensions[3];
	GLboolean   supported;
} wrap_modes[] = {
	{GL_REPEAT,                     "REPEAT",                     GL_FALSE},
	{GL_CLAMP,                      "CLAMP",                      GL_TRUE},

	{GL_CLAMP_TO_EDGE,              "CLAMP_TO_EDGE",              GL_TRUE,
		{"GL_EXT_texture_edge_clamp", "GL_SGIS_texture_edge_clamp", NULL}},

	{GL_CLAMP_TO_BORDER,            "CLAMP_TO_BORDER",            GL_TRUE,
		{"GL_ARB_texture_border_clamp", "GL_SGIS_texture_border_clamp", NULL}},

	{GL_MIRRORED_REPEAT,            "MIRRORED_REPEAT",            GL_FALSE,
		{"GL_ARB_texture_mirrored_repeat", "GL_IBM_texture_mirrored_repeat", NULL}},

	{GL_MIRROR_CLAMP_EXT,           "MIRROR_CLAMP_EXT",           GL_FALSE,
		{"GL_EXT_texture_mirror_clamp", "GL_ATI_texture_mirror_once", NULL}},

	{GL_MIRROR_CLAMP_TO_EDGE_EXT,   "MIRROR_CLAMP_TO_EDGE_EXT",   GL_FALSE,
		{"GL_EXT_texture_mirror_clamp", "GL_ATI_texture_mirror_once",
		"GL_ARB_texture_mirror_clamp_to_edge"}},

	{GL_MIRROR_CLAMP_TO_BORDER_EXT, "MIRROR_CLAMP_TO_BORDER_EXT", GL_FALSE,
		{"GL_EXT_texture_mirror_clamp", NULL, NULL}},

	{0}
};

/* Defines. */
#define TEXEL_SIZE          3
#define TILE_SPACE          5
#define SIZE_POT            8
#define SIZE_NPOT           9
#define SIZEMAX             (SIZE_POT > SIZE_NPOT ? SIZE_POT : SIZE_NPOT)
#define TEXTURE_SIZE(npot)  ((npot) ? SIZE_NPOT : SIZE_POT)
#define BIAS_INT(npot)      (TEXTURE_SIZE(npot)+2)
#define BIAS(npot)          (BIAS_INT(npot) / (double)TEXTURE_SIZE(npot))
#define TILE_SIZE(npot)     ((BIAS_INT(npot)*2 + TEXTURE_SIZE(npot)) * TEXEL_SIZE)

/* Test parameters and state. */
static GLuint texture_id;
static GLenum texture_target;
static GLboolean texture_npot;
static GLboolean texture_proj;
static GLboolean test_border_color;
static GLboolean texture_swizzle;
static GLboolean has_texture_swizzle;
static GLboolean has_npot;
static const struct test_desc *test;
static const struct format_desc *init_format;
static int size_x = 1, size_y = 1, size_z = 1;
static GLuint prog_int, prog_uint;
static GLint int_scale_loc, uint_scale_loc;

/* Image data. */
static const int swizzle[4] = {2, 0, 1, 3};
static const float border[4] = { 0.1, 0.9, 0.5, 0.8 };
static float border_real[4];
static float image[SIZEMAX * SIZEMAX * SIZEMAX * 4];

/* Piglit stuff. */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 872;
	config.window_height = 230;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

extern int piglit_automatic;

static int get_int_format_bits(const struct format_desc *format)
{
	int maxbits = MAX2(format->red,
			   MAX2(format->green,
				MAX2(format->blue,
				     MAX2(format->alpha,
					  MAX2(format->luminance, format->intensity)))));
	return maxbits >= 32 ? 32 :
	       maxbits >= 16 ? 16 :
	       maxbits >= 10 ? 10 : 8;
}

static void sample_nearest(int x, int y, int z,
			   GLenum wrap_mode, GLenum filter,
			   unsigned char pixel[4],
			   const struct format_desc *format,
			   GLboolean npot, GLboolean texswizzle,
			   int bits)
{
	unsigned sample_border = 0;
	float border_factor = 0;
	int coords[3] = {x, y, z};
	unsigned i;
	float result[4];
	int *iresult = (int*)result;
	unsigned *uiresult = (unsigned*)result;

	/* Zero coords according to the texture target. */
	switch (texture_target) {
	case GL_TEXTURE_1D:
		coords[1] = 0;
		/* Fall through. */

	case GL_TEXTURE_2D:
	case GL_TEXTURE_RECTANGLE:
		coords[2] = 0;
	}

	/* Handle clamp mirroring. */
	switch (wrap_mode) {
	case GL_MIRROR_CLAMP_EXT:
	case GL_MIRROR_CLAMP_TO_EDGE_EXT:
	case GL_MIRROR_CLAMP_TO_BORDER_EXT:
		for (i = 0; i < 3; i++) {
			if (coords[i] < 0) {
				coords[i] = -coords[i] - 1;
			}
		}
	}

	/* Handle border sampling. */
	switch (wrap_mode) {
	case GL_CLAMP:
	case GL_MIRROR_CLAMP_EXT:
		if (filter != GL_LINEAR) {
			break;
		}

	case GL_CLAMP_TO_BORDER:
	case GL_MIRROR_CLAMP_TO_BORDER_EXT:
		sample_border = 0;
		for (i = 0; i < 3; i++) {
			if (coords[i] >= TEXTURE_SIZE(npot) || coords[i] < 0) {
				sample_border++;
			}
		}
	}

	/* Figure out what the border factor is. */
	switch (wrap_mode) {
	case GL_CLAMP:
	case GL_MIRROR_CLAMP_EXT:
		if (filter == GL_LINEAR) {
			const double factor[] = {0, 0.5, 0.75, 0.875};
			border_factor = factor[sample_border];
		}
		break;
	case GL_CLAMP_TO_BORDER:
	case GL_MIRROR_CLAMP_TO_BORDER_EXT:
		if (sample_border) {
			border_factor = 1;
		}
		break;
	}

	/* Handle wrapping. */
	switch (wrap_mode) {
	case GL_REPEAT:
		for (i = 0; i < 3; i++) {
			coords[i] = (coords[i] + TEXTURE_SIZE(npot)*10) % TEXTURE_SIZE(npot);
		}
		break;

	case GL_CLAMP:
	case GL_MIRROR_CLAMP_EXT:
	case GL_CLAMP_TO_BORDER:
	case GL_MIRROR_CLAMP_TO_BORDER_EXT:
	case GL_CLAMP_TO_EDGE:
	case GL_MIRROR_CLAMP_TO_EDGE_EXT:
		for (i = 0; i < 3; i++) {
			coords[i] = coords[i] >= TEXTURE_SIZE(npot) ? TEXTURE_SIZE(npot)-1 :
								coords[i] < 0 ? 0 : coords[i];
		}
		break;

	case GL_MIRRORED_REPEAT:
		for (i = 0; i < 3; i++) {
			coords[i] = (coords[i] + TEXTURE_SIZE(npot)*10) % (TEXTURE_SIZE(npot) * 2);
			if (coords[i] >= TEXTURE_SIZE(npot))
				coords[i] = 2*TEXTURE_SIZE(npot) - coords[i] - 1;
		}
		break;
	}

	/* Sample the pixel. */
	if (format->depth) {
		result[0] = result[1] = result[2] =
				image[(coords[2]*size_y*size_x +
				       coords[1]*size_x + coords[0])];
		result[3] = 1;
	} else {
		memcpy(result,
		       &image[(coords[2]*size_y*size_x +
			       coords[1]*size_x + coords[0])*4],
		       sizeof(result));
	}

	if (format->srgb) {
		for (i = 0; i < 3; i++) {
			result[i] = piglit_srgb_to_linear(result[i]);
		}
	}

	/* Sample the border.
	 * This is actually the only place we care about linear filtering,
	 * for CLAMP. Pixels are expected to be sampled at their center,
	 * so we don't have to take 4 samples. */
	if (border_factor == 1) {
		memcpy(result, border_real, 16);
	} else if (border_factor) {
		for (i = 0; i < 4; i++)
			result[i] = border_real[i] * border_factor +
				    result[i] * (1 - border_factor);
	}

	/* Texture swizzle. */
	if (texswizzle) {
		float orig[4];
		memcpy(orig, result, 16);

		for (i = 0; i < 4; i++) {
			result[i] = orig[swizzle[i]];
		}
	}

	/* Final conversion. */
	switch (format->type) {
	case FLOAT_TYPE:
		for (i = 0; i < 4; i++) {
			pixel[i] = result[i] * 255.1;
		}
		break;
	case INT_TYPE:
		for (i = 0; i < 4; i++) {
			pixel[i] = iresult[i] * (255.1 / ((1ull << (bits-1))-1));
		}
		break;
	case UINT_TYPE:
		for (i = 0; i < 4; i++) {
			pixel[i] = uiresult[i] * (255.1 / ((1ull << bits)-1));
		}
		if (bits == 10) {
			pixel[3] = uiresult[3] * (255.1 / 3);
		}
		break;
	}
}

GLboolean probe_pixel_rgba(unsigned char *pixels, unsigned stride,
			   unsigned *pixels_deltamax,
			   unsigned x, unsigned y, unsigned char *expected,
			   unsigned a, unsigned b,
			   const char *filter, const char *wrapmode)
{
	GLboolean pass = GL_TRUE;
	unsigned i;
	unsigned char *probe = &pixels[(y * stride + x) * 4];

	for (i = 0; i < 4; ++i) {
		int delta = abs((int)probe[i] - (int)expected[i]);
		if (delta > pixels_deltamax[i]) {
			pass = GL_FALSE;
			break;
		}
	}

	if (pass) {
		return GL_TRUE;
	}

	printf("Fail with %s and %s at (%i,%i) @ %i,%i\n", filter, wrapmode, x, y, a, b);
	printf("  Expected: %i %i %i %i\n", expected[0], expected[1], expected[2], expected[3]);
	printf("  Observed: %i %i %i %i\n", probe[0], probe[1], probe[2], probe[3]);
	return GL_FALSE;
}

static void update_swizzle(GLboolean texswizzle)
{
	GLint iden[4] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
	GLint swiz[4] = {iden[swizzle[0]], iden[swizzle[1]], iden[swizzle[2]],
			 iden[swizzle[3]]};

	glBindTexture(texture_target, texture_id);
	if (texswizzle) {
		glTexParameteriv(texture_target, GL_TEXTURE_SWIZZLE_RGBA_EXT, swiz);
	} else {
		glTexParameteriv(texture_target, GL_TEXTURE_SWIZZLE_RGBA_EXT, iden);
	}
}

static GLboolean skip_test(GLenum mode, GLenum filter)
{
	if (mode == GL_CLAMP_TO_BORDER ||
	    mode == GL_MIRROR_CLAMP_TO_BORDER_EXT ||
	    (filter == GL_LINEAR &&
	     (mode == GL_CLAMP ||
	      mode == GL_MIRROR_CLAMP_EXT))) {
		return !test_border_color;
	}

	return test_border_color;
}

/**
 * For a given wrap mode index, filter mode index and npot flag, return
 * the (x,y) position for drawing the test pattern.
 */
static void
test_to_xy(unsigned mode, unsigned filter, unsigned npot, int *x, int *y)
{
	assert(mode < ARRAY_SIZE(wrap_modes));
	assert(filter < 2);
	assert(npot < 2);
	*x = mode * (TILE_SIZE(npot) + TILE_SPACE) + 5;
	*y = filter * (TILE_SIZE(npot) + TILE_SPACE) + 35;
}


static void draw(const struct format_desc *format,
		 GLboolean npot, GLboolean texproj)
{
	unsigned i, j;
	int num_filters = format->type == FLOAT_TYPE ? 2 : 1;
	int bits = get_int_format_bits(format);
	float scale[4];

	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(texture_target, texture_id);

	switch (format->type) {
	case INT_TYPE:
		scale[0] = scale[1] = scale[2] = scale[3] = 1.0/((1ull << (bits-1))-1);
		glUseProgram(prog_int);
		glUniform4fv(int_scale_loc, 1, scale);
		break;
	case UINT_TYPE:
		scale[0] = scale[1] = scale[2] = scale[3] = 1.0/((1ull << bits)-1);
		if (bits == 10) {
			scale[3] = 1.0/3;
		}
		glUseProgram(prog_uint);
		glUniform4fv(uint_scale_loc, 1, scale);
		break;
	default:;
	}

	/* Loop over min/mag filters. */
	for (i = 0; i < num_filters; i++) {
		GLenum filter = i ? GL_LINEAR : GL_NEAREST;

		glTexParameteri(texture_target, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(texture_target, GL_TEXTURE_MAG_FILTER, filter);

		/* Loop over wrap modes. */
		for (j = 0; wrap_modes[j].mode != 0; j++) {
			int xpos, ypos;
			float x0 = 0;
			float y0 = 0;
			float x1 = TILE_SIZE(npot);
			float y1 = TILE_SIZE(npot);
			float s0 = -BIAS(npot);
			float t0 = -BIAS(npot);
			float s1 = 1 + BIAS(npot);
			float t1 = 1 + BIAS(npot);
			float q = 1;
			float ts0 = s0, ts1 = s1, tt0 = t0, tt1 = t1, tr = 0.5;

			if (!wrap_modes[j].supported)
				continue;

			if (skip_test(wrap_modes[j].mode, filter))
				continue;

			/* Projective texturing. */
			if (texproj) {
				q = 2.3;
				ts0 *= q;
				ts1 *= q;
				tt0 *= q;
				tt1 *= q;
				tr *= q;
			}

			/* Rectangles. */
			if (texture_target == GL_TEXTURE_RECTANGLE) {
				ts0 *= TEXTURE_SIZE(npot);
				ts1 *= TEXTURE_SIZE(npot);
				tt0 *= TEXTURE_SIZE(npot);
				tt1 *= TEXTURE_SIZE(npot);
			}

			glTexParameteri(texture_target, GL_TEXTURE_WRAP_S,
					wrap_modes[j].mode);
			glTexParameteri(texture_target, GL_TEXTURE_WRAP_T,
					wrap_modes[j].mode);
			glTexParameteri(texture_target, GL_TEXTURE_WRAP_R,
					wrap_modes[j].mode);

			glPushMatrix();
			test_to_xy(j, i, npot, &xpos, &ypos);
			glTranslatef(xpos, ypos, 0.0);

			glEnable(texture_target);
			glColor3f(1, 1, 1);
			glBegin(GL_POLYGON);
			glTexCoord4f(ts0, tt0, tr, q);  glVertex2f(x0, y0);
			glTexCoord4f(ts1, tt0, tr, q);  glVertex2f(x1, y0);
			glTexCoord4f(ts1, tt1, tr, q);  glVertex2f(x1, y1);
			glTexCoord4f(ts0, tt1, tr, q);  glVertex2f(x0, y1);
			glEnd();
			glDisable(texture_target);

			/* Draw red outline showing bounds of texture at s=0,1 and t=0,1. */
			if (!piglit_automatic) {
				glColor3f(1, 0, 0);
				glBegin(GL_LINE_LOOP);
				glVertex2f(x0 + BIAS(npot) * (x1-x0) / (s1-s0),
					   y0 + BIAS(npot) * (y1-y0) / (t1-t0));
				glVertex2f(x1 - BIAS(npot) * (x1-x0) / (s1-s0),
					   y0 + BIAS(npot) * (y1-y0) / (t1-t0));
				glVertex2f(x1 - BIAS(npot) * (x1-x0) / (s1-s0),
					   y1 - BIAS(npot) * (y1-y0) / (t1-t0));
				glVertex2f(x0 + BIAS(npot) * (x1-x0) / (s1-s0),
					   y1 - BIAS(npot) * (y1-y0) / (t1-t0));
				glEnd();
			}

			glPopMatrix();
		}
	}

	switch (format->type) {
	case INT_TYPE:
	case UINT_TYPE:
		glUseProgram(0);
		break;
	default:;
	}

	glDisable(texture_target);
	glColor3f(1, 1, 1);

	if (!piglit_automatic) {
		printf("modes: ");
		for (i = 0; wrap_modes[i].mode != 0; i++) {
			printf("%s, ",
			       piglit_get_gl_enum_name(wrap_modes[i].mode));
		}
		printf("\n");
	}
}

static GLboolean probe_pixels(const struct format_desc *format, GLboolean npot, GLboolean texswizzle)
{
	unsigned i, j;
	unsigned char *pixels;
	GLboolean pass = GL_TRUE;
	int num_filters = format->type == FLOAT_TYPE ? 2 : 1;
	int bits = get_int_format_bits(format);

	pixels = malloc(piglit_width * piglit_height * 4);
	glReadPixels(0, 0, piglit_width, piglit_height,
		     GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	/* make slices different for 3D textures */

	/* Loop over min/mag filters. */
	for (i = 0; i < num_filters; i++) {
		GLenum filter = i ? GL_LINEAR : GL_NEAREST;
		const char *sfilter = i ? "LINEAR" : "NEAREST";
		unsigned deltamax[4] = {0};
		unsigned deltamax_swizzled[4] = {0};
		unsigned *deltamax_lut = i ? linear_deltamax : nearest_deltamax;

		/* Get the deltamax for each channel. */
		if (format->intensity) {
			for (j = 0; j < 4; j++) {
				deltamax[j] = deltamax_lut[format->intensity];
			}
		} else {
			if (format->luminance) {
				for (j = 0; j < 3; j++) {
					deltamax[j] = deltamax_lut[format->luminance];
				}
			} else if (format->depth) {
				for (j = 0; j < 3; j++) {
					deltamax[j] = deltamax_lut[format->depth];
				}
			} else {
				deltamax[0] = deltamax_lut[format->red];
				deltamax[1] = deltamax_lut[format->green];
				deltamax[2] = deltamax_lut[format->blue];
			}
			deltamax[3] = deltamax_lut[format->alpha];
		}
		if (texswizzle) {
			for (j = 0; j < 4; j++) {
				deltamax_swizzled[j] = deltamax[swizzle[j]];
			}
		} else {
			memcpy(deltamax_swizzled, deltamax, sizeof(deltamax));
		}

		/* Loop over all wrap modes. */
		for (j = 0; wrap_modes[j].mode != 0; j++) {
			unsigned char expected[4];
			int x0, y0;
			int a, b;

			test_to_xy(j, i, npot, &x0, &y0);

			if (!wrap_modes[j].supported)
				continue;

			if (skip_test(wrap_modes[j].mode, filter))
				continue;

			for (b = 0; b < (TEXTURE_SIZE(npot) + BIAS_INT(npot)*2); b++) {
				for (a = 0; a < (TEXTURE_SIZE(npot) + BIAS_INT(npot)*2); a++) {
					double x = x0 + TEXEL_SIZE*(a+0.5);
					double y = y0 + TEXEL_SIZE*(b+0.5);

					sample_nearest(a - BIAS_INT(npot), b - BIAS_INT(npot),
						       0, /* the slices are the same */
						       wrap_modes[j].mode, filter, expected,
						       format, npot, texswizzle, bits);

					if (!probe_pixel_rgba(pixels, piglit_width, deltamax_swizzled,
							      x, y, expected, a, b,
							      sfilter, wrap_modes[j].name)) {
						pass = GL_FALSE;
						goto tile_done;
					}
				}
			}

tile_done:
			;
		}
	}

	free(pixels);
	return pass;
}

static GLboolean test_format_npot_swizzle(const struct format_desc *format,
				   GLboolean npot, GLboolean texswizzle)
{
	GLboolean pass;

	if (has_texture_swizzle) {
		update_swizzle(texswizzle);
	}

	printf("Testing %s%s%s%s%s\n", format->name,
	       npot ? ", NPOT" : "",
	       texswizzle ? ", swizzled" : "",
	       texture_proj ? ", projected" : "",
	       test_border_color ? ", border color only" : "");

	draw(format, npot, texture_proj);
	pass = probe_pixels(format, npot, texswizzle);
	piglit_present_results();

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "%s%s%s%s%s", format->name,
				     npot ? ", NPOT" : "",
				     texswizzle ? ", swizzled" : "",
				     texture_proj ? ", projected" : "",
				     test_border_color ? ", border color only" : "");
	return pass;
}

static void init_texture(const struct format_desc *format, GLboolean npot);

static GLboolean test_format_npot(const struct format_desc *format, GLboolean npot)
{
	GLboolean pass = GL_TRUE;

	init_texture(format, npot);

	if (!piglit_automatic) {
		pass = test_format_npot_swizzle(format, npot, texture_swizzle) && pass;
	} else {
		pass = test_format_npot_swizzle(format, npot, texture_swizzle) && pass;

		/* Don't test NPOT and swizzle at the same time, it's not very useful.
		 *
		 * Also don't test swizzling with the border color if swizzling is disabled.
		 * It has to be enabled on the command line.
		 */
		if (!texture_swizzle && !npot && !test_border_color && has_texture_swizzle) {
			pass = pass && test_format_npot_swizzle(format, npot, 1);
		}
	}
	return pass;
}

static GLboolean test_format(const struct format_desc *format)
{
	GLboolean pass = GL_TRUE;

	if (!piglit_automatic) {
		pass = test_format_npot(format, texture_npot);
	} else {
		pass = test_format_npot(format, 0);
		if (has_npot && !test_border_color) {
			pass = pass && test_format_npot(format, 1);
		}
	}
	return pass;
}

enum piglit_result piglit_display()
{
	GLboolean pass = GL_TRUE;

	if (!piglit_automatic) {
		pass = test_format(init_format ? init_format : &test->format[0]);
	} else {
		if (init_format) {
			pass = pass && test_format(init_format);
		} else {
			int i;
			for (i = 0; i < test->num_formats; i++) {
				pass = test_format(&test->format[i]) && pass;
			}
		}
	}
	assert(glGetError() == 0);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static void key_func(unsigned char key, int x, int y)
{
	switch (key) {
	case 'n':
		texture_npot = !texture_npot;
		break;
	case 'p':
		texture_proj = !texture_proj;
		break;
	case 's':
		texture_swizzle = !texture_swizzle;
		break;
	case 'b':
		test_border_color = !test_border_color;
		break;
	}

	piglit_escape_exit_key(key, x, y);
}

static void init_float_texture(const struct format_desc *format,
			       float *image, const float border[4], float border_real[4])
{
	int x, y, z, i;
	static float red[4] = {1, 0, 0, 0.8};
	static float cyan[4] = {0, 1, 1, 0.6};
	static float blue[4] = {0, 0, 1, 0.4};
	static float orange[4] = {1, 0.6, 0.3, 0.2};
	static float white[4] = {1, 1, 1, 1};
	static float black[4] = {0, 0, 0, 0};
	float *colors[7] = {red, cyan, blue, orange, white, black, border_real};

	memcpy(border_real, border, 16);

	/* Set the colors to match the base format. */
	if (format->intensity) {
		for (i = 0; i < 7; i++) {
			colors[i][3] = colors[i][2] = colors[i][1] = colors[i][0];
		}
	} else {
		if (format->luminance || format->depth) {
			for (i = 0; i < 7; i++) {
				colors[i][2] = colors[i][1] = colors[i][0];
			}
		} else {
			if (format->red == 0) {
				for (i = 0; i < 7; i++) {
					colors[i][0] = 0;
				}
			}
			if (format->green == 0) {
				for (i = 0; i < 7; i++) {
					colors[i][1] = 0;
				}
			}
			if (format->blue == 0) {
				for (i = 0; i < 7; i++) {
					colors[i][2] = 0;
				}
			}
		}
		if (format->alpha == 0) {
			for (i = 0; i < 7; i++) {
				colors[i][3] = 1;
			}
		}
	}

	for (z = 0; z < size_z; z++) {
		for (y = 0; y < size_y; y++) {
			for (x = 0; x < size_x; x++) {
				unsigned i = (z*size_y*size_x + y*size_x + x)*4;

				if (y == 0 && x == 0) {
					/* lower-left texel = RED */
					image[i + 0] = red[0];
					image[i + 1] = red[1];
					image[i + 2] = red[2];
					image[i + 3] = red[3];
				} else if (y == 0 && x == size_x-1) {
					/* lower-right corner = CYAN */
					image[i + 0] = cyan[0];
					image[i + 1] = cyan[1];
					image[i + 2] = cyan[2];
					image[i + 3] = cyan[3];
				} else if (y == size_y-1 && x == 0) {
					/* upper-left corner = BLUE */
					image[i + 0] = blue[0];
					image[i + 1] = blue[1];
					image[i + 2] = blue[2];
					image[i + 3] = blue[3];
				} else if (y == size_y-1 && x == size_x-1) {
					/* upper-right corner = ORANGE */
					image[i + 0] = orange[0];
					image[i + 1] = orange[1];
					image[i + 2] = orange[2];
					image[i + 3] = orange[3];
				} else if ((y + x) & 1) {
					/* white */
					image[i + 0] = white[0];
					image[i + 1] = white[1];
					image[i + 2] = white[2];
					image[i + 3] = white[3];
				} else {
					/* black */
					image[i + 0] = black[0];
					image[i + 1] = black[1];
					image[i + 2] = black[2];
					image[i + 3] = black[3];
				}
			}
		}
	}
}

static void get_int_border_color(const struct format_desc *format,
				 const float border[4], int iresult[4])
{
	int i;
	int bits = get_int_format_bits(format);
	unsigned *uresult = (unsigned*)iresult;

	if (format->type == INT_TYPE) {
		for (i = 0; i < 4; i++) {
			iresult[i] = border[i] * (double)((1ull << (bits-1))-1);
		}
	} else {
		for (i = 0; i < 4; i++) {
			uresult[i] = border[i] * (double)((1ull << bits)-1);
		}
		if (bits == 10) {
			uresult[3] = border[3] * (double)3;
		}
	}
}

static void init_int_texture(const struct format_desc *format,
			     int *image, int border_real[4])
{
	int x, y, z, i;
	int bits = get_int_format_bits(format);

	static int i32red[4] = {INT_MAX, 0, 0, INT_MAX*0.8};
	static int i32cyan[4] = {0, INT_MAX, INT_MAX, INT_MAX*0.6};
	static int i32blue[4] = {0, 0, INT_MAX, INT_MAX*0.4};
	static int i32orange[4] = {INT_MAX, INT_MAX*0.6, INT_MAX*0.3, INT_MAX*0.2};
	static int i32white[4] = {INT_MAX, INT_MAX, INT_MAX, INT_MAX};
	static int i32black[4] = {0, 0, 0, 0};
	int *i32colors[7] = {i32red, i32cyan, i32blue, i32orange, i32white, i32black, border_real};

	static int i16red[4] = {SHRT_MAX, 0, 0, SHRT_MAX*0.8};
	static int i16cyan[4] = {0, SHRT_MAX, SHRT_MAX, SHRT_MAX*0.6};
	static int i16blue[4] = {0, 0, SHRT_MAX, SHRT_MAX*0.4};
	static int i16orange[4] = {SHRT_MAX, SHRT_MAX*0.6, SHRT_MAX*0.3, SHRT_MAX*0.2};
	static int i16white[4] = {SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX};
	static int i16black[4] = {0, 0, 0, 0};
	int *i16colors[7] = {i16red, i16cyan, i16blue, i16orange, i16white, i16black, border_real};

	static int i8red[4] = {SCHAR_MAX, 0, 0, SCHAR_MAX*0.8};
	static int i8cyan[4] = {0, SCHAR_MAX, SCHAR_MAX, SCHAR_MAX*0.6};
	static int i8blue[4] = {0, 0, SCHAR_MAX, SCHAR_MAX*0.4};
	static int i8orange[4] = {SCHAR_MAX, SCHAR_MAX*0.6, SCHAR_MAX*0.3, SCHAR_MAX*0.2};
	static int i8white[4] = {SCHAR_MAX, SCHAR_MAX, SCHAR_MAX, SCHAR_MAX};
	static int i8black[4] = {0, 0, 0, 0};
	int *i8colors[7] = {i8red, i8cyan, i8blue, i8orange, i8white, i8black, border_real};

	static unsigned ui32red[4] = {UINT_MAX, 0, 0, UINT_MAX*0.8};
	static unsigned ui32cyan[4] = {0, UINT_MAX, UINT_MAX, UINT_MAX*0.6};
	static unsigned ui32blue[4] = {0, 0, UINT_MAX, UINT_MAX*0.4};
	static unsigned ui32orange[4] = {UINT_MAX, UINT_MAX*0.6, UINT_MAX*0.3, UINT_MAX*0.2};
	static unsigned ui32white[4] = {UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX};
	static unsigned ui32black[4] = {0, 0, 0, 0};
	unsigned *ui32colors[7] = {ui32red, ui32cyan, ui32blue, ui32orange, ui32white, ui32black, (unsigned*)border_real};

	static unsigned ui16red[4] = {USHRT_MAX, 0, 0, USHRT_MAX*0.8};
	static unsigned ui16cyan[4] = {0, USHRT_MAX, USHRT_MAX, USHRT_MAX*0.6};
	static unsigned ui16blue[4] = {0, 0, USHRT_MAX, USHRT_MAX*0.4};
	static unsigned ui16orange[4] = {USHRT_MAX, USHRT_MAX*0.6, USHRT_MAX*0.3, USHRT_MAX*0.2};
	static unsigned ui16white[4] = {USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX};
	static unsigned ui16black[4] = {0, 0, 0, 0};
	unsigned *ui16colors[7] = {ui16red, ui16cyan, ui16blue, ui16orange, ui16white, ui16black, (unsigned*)border_real};

	static unsigned ui8red[4] = {UCHAR_MAX, 0, 0, UCHAR_MAX*0.8};
	static unsigned ui8cyan[4] = {0, UCHAR_MAX, UCHAR_MAX, UCHAR_MAX*0.6};
	static unsigned ui8blue[4] = {0, 0, UCHAR_MAX, UCHAR_MAX*0.4};
	static unsigned ui8orange[4] = {UCHAR_MAX, UCHAR_MAX*0.6, UCHAR_MAX*0.3, UCHAR_MAX*0.2};
	static unsigned ui8white[4] = {UCHAR_MAX, UCHAR_MAX, UCHAR_MAX, UCHAR_MAX};
	static unsigned ui8black[4] = {0, 0, 0, 0};
	unsigned *ui8colors[7] = {ui8red, ui8cyan, ui8blue, ui8orange, ui8white, ui8black, (unsigned*)border_real};

	static unsigned ui1010102red[4] = {1023, 0, 0, 3*0.8};
	static unsigned ui1010102cyan[4] = {0, 1023, 1023, 3*0.6};
	static unsigned ui1010102blue[4] = {0, 0, 1023, 3*0.4};
	static unsigned ui1010102orange[4] = {1023, 1023*0.6, 1023*0.3, 3*0.2};
	static unsigned ui1010102white[4] = {1023, 1023, 1023, 3};
	static unsigned ui1010102black[4] = {0, 0, 0, 0};
	unsigned *ui1010102colors[7] = {ui1010102red, ui1010102cyan, ui1010102blue, ui1010102orange,
					ui1010102white, ui1010102black, (unsigned*)border_real};

	int **colors = { 0 };
	int *red, *cyan, *blue, *orange, *white, *black;

	switch (format->type) {
	case INT_TYPE:
		switch (bits) {
		case 8:
			colors = i8colors;
			break;
		case 16:
			colors = i16colors;
			break;
		case 32:
			colors = i32colors;
			break;
		default:
			assert(!"Unexpected number of bits");
			break;
		}
		break;
	case UINT_TYPE:
		switch (bits) {
		case 10:
			colors = (int**)ui1010102colors;
			break;
		case 8:
			colors = (int**)ui8colors;
			break;
		case 16:
			colors = (int**)ui16colors;
			break;
		case 32:
			colors = (int**)ui32colors;
			break;
		default:
			assert(!"Unexpected number of bits");
			break;
		}
		break;
	default:
		assert(0);
	}

	red = colors[0];
	cyan = colors[1];
	blue = colors[2];
	orange = colors[3];
	white = colors[4];
	black = colors[5];

	/* Set the colors to match the base format. */
	if (format->intensity) {
		for (i = 0; i < 7; i++) {
			colors[i][3] = colors[i][2] = colors[i][1] = colors[i][0];
		}
	} else {
		if (format->luminance || format->depth) {
			for (i = 0; i < 7; i++) {
				colors[i][2] = colors[i][1] = colors[i][0];
			}
		} else {
			if (format->red == 0) {
				for (i = 0; i < 7; i++) {
					colors[i][0] = 0;
				}
			}
			if (format->green == 0) {
				for (i = 0; i < 7; i++) {
					colors[i][1] = 0;
				}
			}
			if (format->blue == 0) {
				for (i = 0; i < 7; i++) {
					colors[i][2] = 0;
				}
			}
		}
		if (format->alpha == 0) {
			for (i = 0; i < 7; i++) {
				colors[i][3] = 1;
			}
		}
	}

	for (z = 0; z < size_z; z++) {
		for (y = 0; y < size_y; y++) {
			for (x = 0; x < size_x; x++) {
				unsigned i = (z*size_y*size_x + y*size_x + x)*4;

				if (y == 0 && x == 0) {
					/* lower-left texel = RED */
					image[i + 0] = red[0];
					image[i + 1] = red[1];
					image[i + 2] = red[2];
					image[i + 3] = red[3];
				} else if (y == 0 && x == size_x-1) {
					/* lower-right corner = CYAN */
					image[i + 0] = cyan[0];
					image[i + 1] = cyan[1];
					image[i + 2] = cyan[2];
					image[i + 3] = cyan[3];
				} else if (y == size_y-1 && x == 0) {
					/* upper-left corner = BLUE */
					image[i + 0] = blue[0];
					image[i + 1] = blue[1];
					image[i + 2] = blue[2];
					image[i + 3] = blue[3];
				} else if (y == size_y-1 && x == size_x-1) {
					/* upper-right corner = ORANGE */
					image[i + 0] = orange[0];
					image[i + 1] = orange[1];
					image[i + 2] = orange[2];
					image[i + 3] = orange[3];
				} else if ((y + x) & 1) {
					/* white */
					image[i + 0] = white[0];
					image[i + 1] = white[1];
					image[i + 2] = white[2];
					image[i + 3] = white[3];
				} else {
					/* black */
					image[i + 0] = black[0];
					image[i + 1] = black[1];
					image[i + 2] = black[2];
					image[i + 3] = black[3];
				}
			}
		}
	}
}

static void init_texture(const struct format_desc *format, GLboolean npot)
{
	int x, y, z;
	GLenum baseformat = format->depth ? (format->stencil ? GL_DEPTH_STENCIL : GL_DEPTH_COMPONENT) :
			    format->type == FLOAT_TYPE ? GL_RGBA : GL_RGBA_INTEGER;
	GLenum type = format->internalformat == GL_DEPTH24_STENCIL8 ? GL_UNSIGNED_INT_24_8 :
		      format->internalformat == GL_DEPTH32F_STENCIL8 ? GL_FLOAT_32_UNSIGNED_INT_24_8_REV :
		      format->type == FLOAT_TYPE ? GL_FLOAT :
		      format->type == INT_TYPE ? GL_INT : GL_UNSIGNED_INT;
	float *data;
	unsigned real_size_x, real_size_y;
	int int_border[4];

	switch (texture_target) {
	case GL_TEXTURE_3D:
		size_z = TEXTURE_SIZE(npot);
		/* Fall through. */
	case GL_TEXTURE_2D:
	case GL_TEXTURE_RECTANGLE:
		size_y = TEXTURE_SIZE(npot);
		/* Fall through. */
	case GL_TEXTURE_1D:
		size_x = TEXTURE_SIZE(npot);
	}

	if (format->type == FLOAT_TYPE) {
		init_float_texture(format, image, border, border_real);
	} else {
		get_int_border_color(format, border, int_border);
		memcpy(border_real, int_border, 16);
		init_int_texture(format, (int*)image, (int*)border_real);
	}

	/* Convert to one-channel texture. Not nice, but easy. */
	if (format->depth) {
		for (x = 1; x < size_z*size_y*size_x; x++) {
			image[x] = image[x*4];
		}
	}

	/* Expand pixels to 4x4 blocks of one color to get
	 * "lossless compression". */
	if (format->compressed) {
		data = malloc(SIZEMAX*4 * SIZEMAX*4 * SIZEMAX * sizeof(float) * 4);

		for (z = 0; z < size_z; z++) {
			for (y = 0; y < size_y; y++) {
				for (x = 0; x < size_x; x++) {
					unsigned src = (z*size_y*size_x + y*size_x + x)*4;
					unsigned dstb = (z*size_y*size_x + y*size_x)*16 + x*4;
					unsigned r, c;

					for (r = 0; r < 4; r++) {
						unsigned dstr = dstb + r*size_x*4;
						for (c = 0; c < 4; c++) {
							unsigned dst = (dstr + c)*4;
							data[dst+0] = image[src+0];
							data[dst+1] = image[src+1];
							data[dst+2] = image[src+2];
							data[dst+3] = image[src+3];
						}
					}

					if ((format->internalformat == GL_COMPRESSED_RGBA_S3TC_DXT1 ||
					     format->internalformat == GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1) &&
					    image[src+3] < 0.5) {
						/* DXT1: If the 1-bit alpha is black,
						 * the RGB color is black as well. */
						image[src+0] = 0;
						image[src+1] = 0;
						image[src+2] = 0;
						image[src+3] = 0;
					}
				}
			}
		}

		real_size_x = size_x*4;
		real_size_y = size_y*4;
	} else {
		data = image;
		real_size_x = size_x;
		real_size_y = size_y;
	}

	switch (format->internalformat) {
	case GL_DEPTH24_STENCIL8:
		/* Convert to D24X8_UNORM. */
		{
			uint32_t *p;

			if (data != image) {
				free(data);
			}

			p = (uint32_t*)(data = malloc(SIZEMAX * SIZEMAX * SIZEMAX * 4));

			for (x = 0; x < size_z*size_y*size_x; x++) {
				p[x] = (uint32_t)(image[x] * ((1<<24) - 1)) << 8;
			}
		}
		break;
	case GL_DEPTH32F_STENCIL8:
		if (data != image) {
			free(data);
		}

		/* Convert to D32F_X24X8. */
		data = malloc(SIZEMAX * SIZEMAX * SIZEMAX * 8);

		for (x = 0; x < size_z*size_y*size_x; x++) {
			data[x*2] = image[x];
		}
		break;
	}

	glBindTexture(texture_target, texture_id);
	switch (format->type) {
	case FLOAT_TYPE:
		glTexParameterfv(texture_target, GL_TEXTURE_BORDER_COLOR, border);
		break;
	case INT_TYPE:
		glTexParameterIivEXT(texture_target, GL_TEXTURE_BORDER_COLOR, int_border);
		break;
	case UINT_TYPE:
		glTexParameterIuivEXT(texture_target, GL_TEXTURE_BORDER_COLOR, (unsigned*)int_border);
		break;
	}

	switch (texture_target) {
	case GL_TEXTURE_1D:
		glTexImage1D(texture_target, 0, format->internalformat,
			     real_size_x, 0,
			     baseformat, type, (void *) data);
		break;

	case GL_TEXTURE_2D:
	case GL_TEXTURE_RECTANGLE:
		glTexImage2D(texture_target, 0, format->internalformat,
			     real_size_x, real_size_y, 0,
			     baseformat, type, (void *) data);
		break;

	case GL_TEXTURE_3D:
		if (1.2 <= atof((char *)glGetString(GL_VERSION)))
			glTexImage3D(texture_target, 0, format->internalformat,
				     real_size_x, real_size_y, size_z, 0,
				     baseformat, type, (void *) data);
		else
			glTexImage3DEXT(texture_target, 0, format->internalformat,
					real_size_x, real_size_y, size_z, 0,
					baseformat, type, (void *) data);
		break;
	}
	assert(glGetError() == 0);

	if (data != image) {
		free(data);
	}
}

static const char *fp_int =
	"#version 130 \n"
	"uniform isampler2D tex; \n"
	"uniform vec4 scale; \n"
	"void main() \n"
	"{ \n"
	"   gl_FragColor = vec4(texture(tex, gl_TexCoord[0].xy)) * scale; \n"
	"} \n";

static const char *fp_uint =
	"#version 130 \n"
	"uniform usampler2D tex; \n"
	"uniform vec4 scale; \n"
	"void main() \n"
	"{ \n"
	"   gl_FragColor = vec4(texture(tex, gl_TexCoord[0].xy)) * scale; \n"
	"} \n";

void piglit_init(int argc, char **argv)
{
	unsigned i, p, fp;

	texture_target = GL_TEXTURE_2D;
	texture_npot = 0;
	texture_proj = 0;
	texture_swizzle = 0;
	has_texture_swizzle = piglit_get_gl_version() >= 33
		|| piglit_is_extension_supported("GL_ARB_texture_swizzle")
		|| piglit_is_extension_supported("GL_EXT_texture_swizzle");
	has_npot = piglit_is_extension_supported("GL_ARB_texture_non_power_of_two");
	test = &test_sets[0];

	piglit_require_extension("GL_ARB_window_pos");

	for (p = 1; p < argc; p++) {
		/* Texture targets. */
		if (strcmp(argv[p], "1D") == 0) {
			texture_target = GL_TEXTURE_1D;
			printf("Using TEXTURE_1D.\n");
			continue;
		}
		if (strcmp(argv[p], "2D") == 0) {
			texture_target = GL_TEXTURE_2D;
			printf("Using TEXTURE_2D.\n");
			continue;
		}
		if (strcmp(argv[p], "3D") == 0) {
			piglit_require_extension("GL_EXT_texture3D");
			texture_target = GL_TEXTURE_3D;
			printf("Using TEXTURE_3D.\n");
			continue;
		}
		if (strcmp(argv[p], "RECT") == 0) {
			piglit_require_extension("GL_ARB_texture_rectangle");
			texture_target = GL_TEXTURE_RECTANGLE;
			texture_npot = GL_TRUE; /* Enforce NPOT dimensions. */
			printf("Using TEXTURE_RECTANGLE.\n");
			continue;
		}

		if (strcmp(argv[p], "proj") == 0) {
			texture_proj = 1;
			printf("Using projective mapping.\n");
			continue;
		}
		if (strcmp(argv[p], "bordercolor") == 0) {
			test_border_color = 1;
			printf("Testing the border color only.\n");
			continue;
		}
		if (strcmp(argv[p], "swizzled") == 0) {
			if (!has_texture_swizzle) {
				printf("OpenGL 3.3, GL_ARB_texture_swizzle, or "
				       "GL_EXT_texture_swizzle is required for "
				       "\"swizzled\".\n");
				piglit_report_result(PIGLIT_SKIP);
			}
			texture_swizzle = 1;
			printf("Using texture swizzling.\n");
			continue;
		}

		for (i = 0; test_sets[i].name; i++) {
			if (strcmp(argv[p], test_sets[i].name) == 0) {
				int j;
				for (j = 0; j < ARRAY_SIZE(test_sets[i].ext); j++) {
					if (test_sets[i].ext[j]) {
						piglit_require_extension(test_sets[i].ext[j]);
					}
				}
				test = &test_sets[i];
				printf("Testing %s.\n", test->name);
				goto outer_continue;
			}
		}

		if (test) {
			/* Formats. */
			for (i = 0; i < test->num_formats; i++) {
				if (strcmp(argv[p], test->format[i].name) == 0) {
					init_format = &test->format[i];
					goto outer_continue;
				}
			}
		}

		printf("Error: Unknown parameter %s\n", argv[p]);
		piglit_report_result(PIGLIT_SKIP);

outer_continue:;
	}

	/* Check wrap extensions. */
	for (i = 0 ; wrap_modes[i].mode != 0 ; i++) {
		if (texture_target == GL_TEXTURE_RECTANGLE &&
		    !wrap_modes[i].valid_for_rect) {
			wrap_modes[i].supported = GL_FALSE;
		} else if (!wrap_modes[i].extensions[0]) {
			wrap_modes[i].supported = GL_TRUE;
		} else {
			int j;
			for (j = 0; j < ARRAY_SIZE(wrap_modes[i].extensions); j++) {
				if (wrap_modes[i].extensions[j] &&
				    piglit_is_extension_supported(wrap_modes[i].extensions[j])) {
					wrap_modes[i].supported = GL_TRUE;
					break;
				}
			}
		}
	}

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glGenTextures(1, &texture_id);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,   GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

	switch (test->format[0].type) {
	case INT_TYPE:
	case UINT_TYPE:
		piglit_require_GLSL_version(130);
		fp = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fp_int);
		assert(fp);
		prog_int = piglit_link_simple_program(0, fp);
		assert(prog_int);
		int_scale_loc = glGetUniformLocation(prog_int, "scale");
		assert(int_scale_loc != -1);

		fp = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fp_uint);
		assert(fp);
		prog_uint = piglit_link_simple_program(0, fp);
		assert(prog_uint);
		uint_scale_loc = glGetUniformLocation(prog_uint, "scale");
		assert(uint_scale_loc != -1);
		break;
	default:;
	}

	assert(glGetError() == 0);

	if (!piglit_automatic) {
		piglit_set_keyboard_func(key_func);
		printf("Hotkeys in the interactive mode:\n"
		       "    n  - switch between POT and NPOT dimensions\n"
		       "    p  - use projective texturing\n"
		       "    s  - use texture swizzling (ARB_texture_swizzle)\n"
		       "    b  - switch between the normal and bordercolor tests\n");
	}
}
