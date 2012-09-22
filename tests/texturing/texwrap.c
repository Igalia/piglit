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
 *   Both projective and non-projective mapping is tested in the automatic
 *   mode.
 *
 * - Texture borders (marked as deprecated in GL3, removed in the Core profile).
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

#include "piglit-util-gl-common.h"

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

/* Only *_ATI versions of these exist. It's nicer without the suffix. */
#define GL_COMPRESSED_LUMINANCE_ALPHA_3DC 0x8837

/* Formats. */

#define FORMAT(f) #f, f

struct format {
    const char  *name;
    GLenum      internalformat;
    int         red, green, blue, alpha, luminance, intensity, depth, stencil;
    GLboolean   compressed;
    float       version;
    const char  *extensions[2];
    const char  *dependency_extension;
} formats[] = {
    /* GL 1.1 */
    {FORMAT(GL_RGBA8),              8, 8, 8, 8, 0, 0, 0, 0,     0, 1.1},
    {FORMAT(GL_RGBA2),              2, 2, 2, 2, 0, 0, 0, 0,     0, 1.1},
    {FORMAT(GL_R3_G3_B2),           3, 3, 2, 0, 0, 0, 0, 0,     0, 1.1},
    {FORMAT(GL_RGB4),               4, 4, 4, 0, 0, 0, 0, 0,     0, 1.1},
    {FORMAT(GL_RGBA4),              4, 4, 4, 4, 0, 0, 0, 0,     0, 1.1},
    {FORMAT(GL_RGB5),               5, 5, 5, 0, 0, 0, 0, 0,     0, 1.1},
    {FORMAT(GL_RGB5_A1),            5, 5, 5, 1, 0, 0, 0, 0,     0, 1.1},
    {FORMAT(GL_RGB8),               8, 8, 8, 0, 0, 0, 0, 0,     0, 1.1},
    {FORMAT(GL_RGB10),              10, 10, 10, 0, 0, 0, 0, 0,  0, 1.1},
    {FORMAT(GL_RGB10_A2),           10, 10, 10, 2, 0, 0, 0, 0,  0, 1.1},
    {FORMAT(GL_RGB12),              12, 12, 12, 0, 0, 0, 0, 0,  0, 1.1},
    {FORMAT(GL_RGBA12),             12, 12, 12, 12, 0, 0, 0, 0, 0, 1.1},
    {FORMAT(GL_RGB16),              16, 16, 16, 0, 0, 0, 0, 0,  0, 1.1},
    {FORMAT(GL_RGBA16),             16, 16, 16, 16, 0, 0, 0, 0, 0, 1.1},
    {FORMAT(GL_ALPHA4),             0, 0, 0, 4, 0, 0, 0, 0,     0, 1.1},
    {FORMAT(GL_ALPHA8),             0, 0, 0, 8, 0, 0, 0, 0,     0, 1.1},
    {FORMAT(GL_ALPHA12),            0, 0, 0, 12, 0, 0, 0, 0,    0, 1.1},
    {FORMAT(GL_ALPHA16),            0, 0, 0, 16, 0, 0, 0, 0,    0, 1.1},
    {FORMAT(GL_LUMINANCE4),         0, 0, 0, 0, 4, 0, 0, 0,     0, 1.1},
    {FORMAT(GL_LUMINANCE8),         0, 0, 0, 0, 8, 0, 0, 0,     0, 1.1},
    {FORMAT(GL_LUMINANCE12),        0, 0, 0, 0, 12, 0, 0, 0,    0, 1.1},
    {FORMAT(GL_LUMINANCE16),        0, 0, 0, 0, 16, 0, 0, 0,    0, 1.1},
    {FORMAT(GL_LUMINANCE4_ALPHA4),  0, 0, 0, 4, 4, 0, 0, 0,     0, 1.1},
    {FORMAT(GL_LUMINANCE6_ALPHA2),  0, 0, 0, 2, 6, 0, 0, 0,     0, 1.1},
    {FORMAT(GL_LUMINANCE8_ALPHA8),  0, 0, 0, 8, 8, 0, 0, 0,     0, 1.1},
    {FORMAT(GL_LUMINANCE12_ALPHA4), 0, 0, 0, 4, 12, 0, 0, 0,    0, 1.1},
    {FORMAT(GL_LUMINANCE12_ALPHA12),0, 0, 0, 12, 12, 0, 0, 0,   0, 1.1},
    {FORMAT(GL_LUMINANCE16_ALPHA16),0, 0, 0, 16, 16, 0, 0, 0,   0, 1.1},
    {FORMAT(GL_INTENSITY4),         0, 0, 0, 0, 0, 4, 0, 0,     0, 1.1},
    {FORMAT(GL_INTENSITY8),         0, 0, 0, 0, 0, 8, 0, 0,     0, 1.1},
    {FORMAT(GL_INTENSITY12),        0, 0, 0, 0, 0, 12, 0, 0,    0, 1.1},
    {FORMAT(GL_INTENSITY16),        0, 0, 0, 0, 0, 16, 0, 0,    0, 1.1},

    /* ARB_depth_texture */
    {FORMAT(GL_DEPTH_COMPONENT16),  0, 0, 0, 0, 0, 0, 16, 0,    0, 1.4,
     {"GL_ARB_depth_texture"}},
    {FORMAT(GL_DEPTH_COMPONENT24),  0, 0, 0, 0, 0, 0, 24, 0,    0, 1.4,
     {"GL_ARB_depth_texture"}},
    {FORMAT(GL_DEPTH_COMPONENT32),  0, 0, 0, 0, 0, 0, 32, 0,    0, 1.4,
     {"GL_ARB_depth_texture"}},

    /* ARB_depth_buffer_float */
    {FORMAT(GL_DEPTH32F_STENCIL8),  0, 0, 0, 0, 0, 0, 32, 8,    0, 3.0,
     {"GL_ARB_depth_buffer_float"}},
    {FORMAT(GL_DEPTH_COMPONENT32F),  0, 0, 0, 0, 0, 0, 32, 0,    0, 3.0,
     {"GL_ARB_depth_buffer_float"}},

    /* EXT_packed_depth_stencil */
    {FORMAT(GL_DEPTH24_STENCIL8),  0, 0, 0, 0, 0, 0, 24, 8,    0, 3.0,
     {"GL_EXT_packed_depth_stencil"}},

    /* ARB_texture_compression_rgtc */
    {FORMAT(GL_COMPRESSED_RED_RGTC1), 4, 0, 0, 0, 0, 0, 0, 0,   1, 3.0,
     {"GL_ARB_texture_compression_rgtc", "GL_EXT_texture_compression_rgtc"}},
    {FORMAT(GL_COMPRESSED_SIGNED_RED_RGTC1), 3, 0, 0, 0, 0, 0, 0, 0, 1, 3.0,
     {"GL_ARB_texture_compression_rgtc", "GL_EXT_texture_compression_rgtc"}},
    {FORMAT(GL_COMPRESSED_RG_RGTC2), 4, 4, 0, 0, 0, 0, 0, 0,    1, 3.0,
     {"GL_ARB_texture_compression_rgtc", "GL_EXT_texture_compression_rgtc"}},
    {FORMAT(GL_COMPRESSED_SIGNED_RG_RGTC2), 4, 4, 0, 0, 0, 0, 0, 0, 1, 3.0,
     {"GL_ARB_texture_compression_rgtc", "GL_EXT_texture_compression_rgtc"}},

    /* ARB_texture_float */
    {FORMAT(GL_ALPHA16F),           0, 0, 0, 16, 0, 0, 0, 0,    0, 999,
     {"GL_ARB_texture_float", "GL_ATI_texture_float"}},
    {FORMAT(GL_LUMINANCE16F),       0, 0, 0, 0, 16, 0, 0, 0,    0, 999,
     {"GL_ARB_texture_float", "GL_ATI_texture_float"}},
    {FORMAT(GL_LUMINANCE_ALPHA16F), 0, 0, 0, 16, 16, 0, 0, 0,   0, 999,
     {"GL_ARB_texture_float", "GL_ATI_texture_float"}},
    {FORMAT(GL_INTENSITY16F),       0, 0, 0, 0, 0, 16, 0, 0,    0, 999,
     {"GL_ARB_texture_float", "GL_ATI_texture_float"}},
    {FORMAT(GL_RGB16F),             16, 16, 16, 0, 0, 0, 0, 0,  0, 3.0,
     {"GL_ARB_texture_float", "GL_ATI_texture_float"}},
    {FORMAT(GL_RGBA16F),            16, 16, 16, 16, 0, 0, 0, 0, 0, 3.0,
     {"GL_ARB_texture_float", "GL_ATI_texture_float"}},
    {FORMAT(GL_ALPHA32F),           0, 0, 0, 32, 0, 0, 0, 0,    0, 999,
     {"GL_ARB_texture_float", "GL_ATI_texture_float"}},
    {FORMAT(GL_LUMINANCE32F),       0, 0, 0, 0, 32, 0, 0, 0,    0, 999,
     {"GL_ARB_texture_float", "GL_ATI_texture_float"}},
    {FORMAT(GL_LUMINANCE_ALPHA32F), 0, 0, 0, 32, 32, 0, 0, 0,   0, 999,
     {"GL_ARB_texture_float", "GL_ATI_texture_float"}},
    {FORMAT(GL_INTENSITY32F),       0, 0, 0, 0, 0, 32, 0, 0,    0, 999,
     {"GL_ARB_texture_float", "GL_ATI_texture_float"}},
    {FORMAT(GL_RGB32F),             32, 32, 32, 0, 0, 0, 0, 0,  0, 3.0,
     {"GL_ARB_texture_float", "GL_ATI_texture_float"}},
    {FORMAT(GL_RGBA32F),            32, 32, 32, 32, 0, 0, 0, 0, 0, 3.0,
     {"GL_ARB_texture_float", "GL_ATI_texture_float"}},

    /* ARB_texture_rg */
    {FORMAT(GL_R8),                 8, 0, 0, 0, 0, 0, 0, 0,     0, 3.0,
     {"GL_ARB_texture_rg"}},
    {FORMAT(GL_RG8),                8, 8, 0, 0, 0, 0, 0, 0,     0, 3.0,
     {"GL_ARB_texture_rg"}},
    {FORMAT(GL_R16),                16, 0, 0, 0, 0, 0, 0, 0,    0, 3.0,
     {"GL_ARB_texture_rg"}},
    {FORMAT(GL_RG16),               16, 16, 0, 0, 0, 0, 0, 0,   0, 3.0,
     {"GL_ARB_texture_rg"}},
    {FORMAT(GL_R16F),               16, 0, 0, 0, 0, 0, 0, 0,    0, 3.0,
     {"GL_ARB_texture_rg"}, "GL_ARB_texture_float"},
    {FORMAT(GL_RG16F),              16, 16, 0, 0, 0, 0, 0, 0,   0, 3.0,
     {"GL_ARB_texture_rg"}, "GL_ARB_texture_float"},
    {FORMAT(GL_R32F),               32, 0, 0, 0, 0, 0, 0, 0,    0, 3.0,
     {"GL_ARB_texture_rg"}, "GL_ARB_texture_float"},
    {FORMAT(GL_RG32F),              32, 32, 0, 0, 0, 0, 0, 0,   0, 3.0,
     {"GL_ARB_texture_rg"}, "GL_ARB_texture_float"},

    /* EXT_packed_float */
    {FORMAT(GL_R11F_G11F_B10F),     6, 6, 5, 0, 0, 0, 0, 0,  0, 3.0,
     {"GL_EXT_packed_float"}},

    /* EXT_texture_compression_latc */
    {FORMAT(GL_COMPRESSED_LUMINANCE_LATC1), 0, 0, 0, 0, 4, 0, 0, 0, 1, 999,
     {"GL_EXT_texture_compression_latc"}},
    {FORMAT(GL_COMPRESSED_SIGNED_LUMINANCE_LATC1), 0, 0, 0, 0, 3, 0, 0, 0, 1, 999,
     {"GL_EXT_texture_compression_latc"}},
    {FORMAT(GL_COMPRESSED_LUMINANCE_ALPHA_LATC2), 0, 0, 0, 4, 4, 0, 0, 0, 1, 999,
     {"GL_EXT_texture_compression_latc"}},
    {FORMAT(GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2), 0, 0, 0, 4, 4, 0, 0, 0, 1, 999,
     {"GL_EXT_texture_compression_latc"}},

    /* ARB_texture_compression */
    {FORMAT(GL_COMPRESSED_ALPHA), 0, 0, 0, 4, 0, 0, 0, 0, 1, 1.3,
     {"GL_ARB_texture_compression"}},
    {FORMAT(GL_COMPRESSED_LUMINANCE), 0, 0, 0, 0, 4, 0, 0, 0, 1, 1.3,
     {"GL_ARB_texture_compression"}},
    {FORMAT(GL_COMPRESSED_LUMINANCE_ALPHA), 0, 0, 0, 4, 4, 0, 0, 0, 1, 1.3,
     {"GL_ARB_texture_compression"}},
    {FORMAT(GL_COMPRESSED_INTENSITY), 0, 0, 0, 0, 0, 4, 0, 0, 1, 1.3,
     {"GL_ARB_texture_compression"}},
    {FORMAT(GL_COMPRESSED_RGB), 4, 4, 4, 0, 0, 0, 0, 0, 1, 1.3,
     {"GL_ARB_texture_compression"}},
    {FORMAT(GL_COMPRESSED_RGBA), 4, 4, 4, 4, 0, 0, 0, 0, 1, 1.3,
     {"GL_ARB_texture_compression"}},

    /* EXT_texture_compression_s3tc */
    {FORMAT(GL_COMPRESSED_RGB_S3TC_DXT1), 4, 4, 4, 0, 0, 0, 0, 0, 1, 999,
     {"GL_EXT_texture_compression_s3tc"}},
    {FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT1), 4, 4, 4, 1, 0, 0, 0, 0, 1, 999,
     {"GL_EXT_texture_compression_s3tc"}},
    {FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT3), 4, 4, 4, 4, 0, 0, 0, 0, 1, 999,
     {"GL_EXT_texture_compression_s3tc"}},
    {FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT5), 4, 4, 4, 4, 0, 0, 0, 0, 1, 999,
     {"GL_EXT_texture_compression_s3tc"}},

    /* EXT_texture_shared_exponent */
    {FORMAT(GL_RGB9_E5),            9, 9, 9, 0, 0, 0, 0, 0,     0, 3.0,
     {"GL_EXT_texture_shared_exponent"}},

    /* ATI_texture_compression_3dc */
    {FORMAT(GL_COMPRESSED_LUMINANCE_ALPHA_3DC), 0, 0, 0, 4, 4, 0, 0, 0, 1, 999,
     {"GL_ATI_texture_compression_3dc"}},

    /* EXT_texture_snorm */
    {FORMAT(GL_R8_SNORM),                 7, 0, 0, 0, 0, 0, 0, 0,     0, 3.1,
     {"GL_EXT_texture_snorm"}},
    {FORMAT(GL_RG8_SNORM),                7, 7, 0, 0, 0, 0, 0, 0,     0, 3.1,
     {"GL_EXT_texture_snorm"}},
    {FORMAT(GL_RGB8_SNORM),               7, 7, 7, 0, 0, 0, 0, 0,     0, 3.1,
     {"GL_EXT_texture_snorm"}},
    {FORMAT(GL_RGBA8_SNORM),              7, 7, 7, 7, 0, 0, 0, 0,     0, 3.1,
     {"GL_EXT_texture_snorm"}},
    {FORMAT(GL_R16_SNORM),                16, 0, 0, 0, 0, 0, 0, 0,    0, 3.1,
     {"GL_EXT_texture_snorm"}},
    {FORMAT(GL_RG16_SNORM),               16, 16, 0, 0, 0, 0, 0, 0,   0, 3.1,
     {"GL_EXT_texture_snorm"}},
    {FORMAT(GL_RGB16_SNORM),              16, 16, 16, 0, 0, 0, 0, 0,   0, 3.1,
     {"GL_EXT_texture_snorm"}},
    {FORMAT(GL_RGBA16_SNORM),             16, 16, 16, 16, 0, 0, 0, 0,   0, 3.1,
     {"GL_EXT_texture_snorm"}},
    {FORMAT(GL_ALPHA8_SNORM),             0, 0, 0, 7, 0, 0, 0, 0,     0, 3.1,
     {"GL_EXT_texture_snorm"}},
    {FORMAT(GL_ALPHA16_SNORM),            0, 0, 0, 16, 0, 0, 0, 0,     0, 3.1,
     {"GL_EXT_texture_snorm"}},
    {FORMAT(GL_LUMINANCE8_SNORM),         0, 0, 0, 0, 7, 0, 0, 0,     0, 3.1,
     {"GL_EXT_texture_snorm"}},
    {FORMAT(GL_LUMINANCE16_SNORM),        0, 0, 0, 0, 16, 0, 0, 0,     0, 3.1,
     {"GL_EXT_texture_snorm"}},
    {FORMAT(GL_LUMINANCE8_ALPHA8_SNORM),  0, 0, 0, 7, 7, 0, 0, 0,     0, 3.1,
     {"GL_EXT_texture_snorm"}},
    {FORMAT(GL_LUMINANCE16_ALPHA16_SNORM),0, 0, 0, 16, 16, 0, 0, 0,     0, 3.1,
     {"GL_EXT_texture_snorm"}},
    {FORMAT(GL_INTENSITY8_SNORM),         0, 0, 0, 0, 0, 8, 0, 0,     0, 3.1,
     {"GL_EXT_texture_snorm"}},
    {FORMAT(GL_INTENSITY16_SNORM),        0, 0, 0, 0, 0, 16, 0, 0,     0, 3.1,
     {"GL_EXT_texture_snorm"}},

    /* ARB_ES2_compatibility */
    {FORMAT(GL_RGB565),                   5, 6, 5, 0, 0, 0, 0, 0,     0, 4.1,
     {"GL_ARB_ES2_compatibility"}}
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
    float       version;
    const char  *extensions[2];
    GLboolean   supported;
} wrap_modes[] = {
    {GL_REPEAT,                     "REPEAT",                     GL_FALSE,
     1.1, {NULL, NULL}},

    {GL_CLAMP,                      "CLAMP",                      GL_TRUE,
     1.1, {NULL, NULL}},

    {GL_CLAMP_TO_EDGE,              "CLAMP_TO_EDGE",              GL_TRUE,
     1.2, {"GL_EXT_texture_edge_clamp", "GL_SGIS_texture_edge_clamp"}},

    {GL_CLAMP_TO_BORDER,            "CLAMP_TO_BORDER",            GL_TRUE,
     1.3, {"GL_ARB_texture_border_clamp", "GL_SGIS_texture_border_clamp"}},

    {GL_MIRRORED_REPEAT,            "MIRRORED_REPEAT",            GL_FALSE,
     1.4, {"GL_ARB_texture_mirrored_repeat", "GL_IBM_texture_mirrored_repeat"}},

    {GL_MIRROR_CLAMP_EXT,           "MIRROR_CLAMP_EXT",           GL_FALSE,
     999.0, {"GL_EXT_texture_mirror_clamp", "GL_ATI_texture_mirror_once"}},

    {GL_MIRROR_CLAMP_TO_EDGE_EXT,   "MIRROR_CLAMP_TO_EDGE_EXT",   GL_FALSE,
     999.0, {"GL_EXT_texture_mirror_clamp", "GL_ATI_texture_mirror_once"}},

    {GL_MIRROR_CLAMP_TO_BORDER_EXT, "MIRROR_CLAMP_TO_BORDER_EXT", GL_FALSE,
     999.0, {"GL_EXT_texture_mirror_clamp", NULL}},

    {0}
};

/* Defines. */
#define BORDER_TEXTURE      1
#define NO_BORDER_TEXTURE   2
#define BIAS_INT            (texture_size+2)
#define BIAS                (BIAS_INT / (double)texture_size)
#define TEXEL_SIZE          3
#define TILE_SIZE           ((BIAS_INT*2 + texture_size) * TEXEL_SIZE)
#define TILE_SPACE          5
#define SIZE_POT            8
#define SIZE_NPOT           9
#define SIZEMAX             (SIZE_POT > SIZE_NPOT ? SIZE_POT : SIZE_NPOT)

/* Test parameters and state. */
static GLuint texture_id;
static GLenum texture_target;
static GLboolean texture_npot;
static GLboolean texture_proj;
static GLboolean test_border_color;
static GLboolean texture_swizzle;
static int texture_size;
static struct format *texture_format;
static GLboolean has_texture_swizzle;

/* Image data. */
static const int swizzle[4] = {2, 0, 1, 3};
static const float borderf[4] = { 0.1, 0.9, 0.5, 0.8 };
static float borderf_real[4];
static float border_image[(SIZEMAX+2) * (SIZEMAX+2) * (SIZEMAX+2) * 4];
static float no_border_image[SIZEMAX * SIZEMAX * SIZEMAX * 4];

/* Derived from texture_size, texture_target, and border. */
static int size_x = 1, size_y = 1, size_z = 1;          /* size */
static int bsize_x = 1, bsize_y = 1, bsize_z = 1;       /* size + 2*border */
static int border_x = 0, border_y = 0, border_z = 0;    /* 0 or 1 */

/* Piglit stuff. */

PIGLIT_GL_TEST_MAIN(
    872 /*window_width*/,
    230 /*window_height*/,
    PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_ALPHA | PIGLIT_GL_VISUAL_DOUBLE)

extern int piglit_automatic;

static void sample_nearest(int x, int y, int z,
                           GLenum wrap_mode, GLenum filter,
                           unsigned char pixel[4])
{
    unsigned sample_border = 0;
    float border_factor = 0;
    int coords[3] = {x, y, z};
    unsigned i;
    float result[4];

    /* Zero coords according to the texture target. */
    switch (texture_target) {
    case GL_TEXTURE_1D:
        coords[1] = 0;
        /* Pass through. */

    case GL_TEXTURE_2D:
    case GL_TEXTURE_RECTANGLE_NV:
        coords[2] = 0;
    }

    /* Resolve clamp mirroring. */
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

    /* Resolve border sampling. */
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
            if (coords[i] >= texture_size || coords[i] < 0) {
                sample_border++;
            }
        }
    }

    /* Resolve wrapping. */
    switch (wrap_mode) {
    case GL_REPEAT:
        for (i = 0; i < 3; i++) {
            coords[i] = (coords[i] + texture_size*10) % texture_size;
        }
        break;

    case GL_CLAMP:
    case GL_MIRROR_CLAMP_EXT:
        if (filter == GL_LINEAR) {
            const double factor[] = {0, 0.5, 0.75, 0.875};
            border_factor = factor[sample_border];
        }
        /* Pass through. */

    case GL_CLAMP_TO_EDGE:
    case GL_MIRROR_CLAMP_TO_EDGE_EXT:
        for (i = 0; i < 3; i++) {
            coords[i] = coords[i] >= texture_size ? texture_size-1 :
                coords[i] < 0 ? 0 : coords[i];
        }
        break;

    case GL_CLAMP_TO_BORDER:
    case GL_MIRROR_CLAMP_TO_BORDER_EXT:
        if (sample_border) {
            border_factor = 1;
        }
        break;

    case GL_MIRRORED_REPEAT:
        for (i = 0; i < 3; i++) {
            coords[i] = (coords[i] + texture_size*10) % (texture_size * 2);
            if (coords[i] >= texture_size)
                coords[i] = 2*texture_size - coords[i] - 1;
        }
        break;
    }

    /* Sample the pixel. */
    if (texture_id == BORDER_TEXTURE) {
        /* Do not sample the per-pixel border. */
        switch (texture_target) {
        case GL_TEXTURE_3D:
            coords[2] += 1;
            /* Pass through. */
        case GL_TEXTURE_2D:
            coords[1] += 1;
            /* Pass through. */
        case GL_TEXTURE_1D:
            coords[0] += 1;
            break;
        default:
            assert(0);
        }

        if (texture_format->depth) {
            result[0] = result[1] = result[2] =
                border_image[(coords[2]*bsize_y*bsize_x +
                              coords[1]*bsize_x + coords[0])];
            result[3] = 1;
        } else {
            memcpy(result,
                   &border_image[(coords[2]*bsize_y*bsize_x +
                                  coords[1]*bsize_x + coords[0])*4],
                   sizeof(result));
        }
    } else {
        if (texture_format->depth) {
            result[0] = result[1] = result[2] =
                no_border_image[(coords[2]*size_y*size_x +
                                 coords[1]*size_x + coords[0])];
            result[3] = 1;
        } else {
            memcpy(result,
                   &no_border_image[(coords[2]*size_y*size_x +
                                     coords[1]*size_x + coords[0])*4],
                   sizeof(result));
        }
    }

    /* Sample the border.
     * This is actually the only place we care about linear filtering,
     * for CLAMP. Pixels are expected to be sampled at their center,
     * so we don't have to take 4 samples. */
    if (sample_border) {
        for (i = 0; i < 4; i++)
            result[i] = borderf_real[i] * border_factor +
                        result[i] * (1 - border_factor);
    }

    if (texture_swizzle) {
        for (i = 0; i < 4; i++) {
            pixel[i] = result[swizzle[i]] * 255.1;
        }
    } else {
        for (i = 0; i < 4; i++) {
            pixel[i] = result[i] * 255.1;
        }
    }
}

GLboolean probe_pixel_rgba(unsigned char *pixels, unsigned stride,
                           unsigned *pixels_deltamax,
                           unsigned x, unsigned y, unsigned char *expected,
                           unsigned a, unsigned b)
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

    printf("Probe at (%i,%i) @ %i,%i\n", x, y, a, b);
    printf("  Expected: %i %i %i %i\n", expected[0], expected[1], expected[2], expected[3]);
    printf("  Observed: %i %i %i %i\n", probe[0], probe[1], probe[2], probe[3]);
    return GL_FALSE;
}

static void update_swizzle()
{
    GLint iden[4] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
    GLint swiz[4] = {iden[swizzle[0]], iden[swizzle[1]], iden[swizzle[2]],
                     iden[swizzle[3]]};
    glBindTexture(texture_target, texture_id);
    if (texture_swizzle) {
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

static void draw()
{
    unsigned i, j;
    int offset;

    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(texture_target, texture_id);

    /* Loop over min/mag filters. */
    for (i = 0; i < 2; i++) {
        GLenum filter = i ? GL_LINEAR : GL_NEAREST;

        offset = 0;

        glTexParameteri(texture_target, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(texture_target, GL_TEXTURE_MAG_FILTER, filter);

        /* Loop over wrap modes. */
        for (j = 0; wrap_modes[j].mode != 0; j++) {
            float x0 = 0;
            float y0 = 0;
            float x1 = TILE_SIZE;
            float y1 = TILE_SIZE;
            float s0 = -BIAS;
            float t0 = -BIAS;
            float s1 = 1 + BIAS;
            float t1 = 1 + BIAS;
            float q = 1;
            float ts0 = s0, ts1 = s1, tt0 = t0, tt1 = t1, tr = 0.5;

            if (!wrap_modes[j].supported)
                continue;

            if (skip_test(wrap_modes[j].mode, filter)) {
                if (skip_test(wrap_modes[j].mode, GL_LINEAR) !=
                    skip_test(wrap_modes[j].mode, GL_NEAREST)) {
                    offset++;
                }
                continue;
            }

            /* Projective texturing. */
            if (texture_proj) {
                q = 2.3;
                ts0 *= q;
                ts1 *= q;
                tt0 *= q;
                tt1 *= q;
                tr *= q;
            }

            /* Rectangles. */
            if (texture_target == GL_TEXTURE_RECTANGLE_NV) {
                ts0 *= texture_size;
                ts1 *= texture_size;
                tt0 *= texture_size;
                tt1 *= texture_size;
            }

            glTexParameteri(texture_target, GL_TEXTURE_WRAP_S,
                            wrap_modes[j].mode);
            glTexParameteri(texture_target, GL_TEXTURE_WRAP_T,
                            wrap_modes[j].mode);
            glTexParameteri(texture_target, GL_TEXTURE_WRAP_R,
                            wrap_modes[j].mode);

            glPushMatrix();
            glTranslatef(offset * (TILE_SIZE + TILE_SPACE) + 5,
                         i * (TILE_SIZE + TILE_SPACE) + 35,
                         0);
            offset++;

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
                glVertex2f(x0 + BIAS * (x1-x0) / (s1-s0),
                           y0 + BIAS * (y1-y0) / (t1-t0));
                glVertex2f(x1 - BIAS * (x1-x0) / (s1-s0),
                           y0 + BIAS * (y1-y0) / (t1-t0));
                glVertex2f(x1 - BIAS * (x1-x0) / (s1-s0),
                           y1 - BIAS * (y1-y0) / (t1-t0));
                glVertex2f(x0 + BIAS * (x1-x0) / (s1-s0),
                           y1 - BIAS * (y1-y0) / (t1-t0));
                glEnd();
            }

            glPopMatrix();
        }
    }

    glDisable(texture_target);
    glColor3f(1, 1, 1);
    offset = 0;

    if (!piglit_automatic) {
        for (i = 0; wrap_modes[i].mode != 0; i++) {
            if (wrap_modes[i].supported) {
                if (skip_test(wrap_modes[i].mode, GL_LINEAR) &&
                    skip_test(wrap_modes[i].mode, GL_NEAREST)) {
                    continue;
                }

                glWindowPos2iARB(offset * (TILE_SIZE + TILE_SPACE) + 5,
                                 5 + ((offset & 1) * 15));
                offset++;
            }
        }
    }
}

static GLboolean probe_pixels()
{
    unsigned i, j;
    unsigned char *pixels;
    GLboolean pass = GL_TRUE;

    pixels = malloc(piglit_width * piglit_height * 4);
    glReadPixels(0, 0, piglit_width, piglit_height,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    /* make slices different for 3D textures */

    /* Loop over min/mag filters. */
    for (i = 0; i < 2; i++) {
        GLenum filter = i ? GL_LINEAR : GL_NEAREST;
        const char *sfilter = i ? "LINEAR" : "NEAREST";
        unsigned deltamax[4] = {0};
        unsigned deltamax_swizzled[4] = {0};
        unsigned *deltamax_lut = i ? linear_deltamax : nearest_deltamax;
        unsigned offset = 0;

        /* Get the deltamax for each channel. */
        if (texture_format->intensity) {
            for (j = 0; j < 4; j++) {
                deltamax[j] = deltamax_lut[texture_format->intensity];
            }
        } else {
            if (texture_format->luminance) {
                for (j = 0; j < 3; j++) {
                    deltamax[j] = deltamax_lut[texture_format->luminance];
                }
            } else if (texture_format->depth) {
                for (j = 0; j < 3; j++) {
                    deltamax[j] = deltamax_lut[texture_format->depth];
                }
            } else {
                deltamax[0] = deltamax_lut[texture_format->red];
                deltamax[1] = deltamax_lut[texture_format->green];
                deltamax[2] = deltamax_lut[texture_format->blue];
            }
            deltamax[3] = deltamax_lut[texture_format->alpha];
        }
        if (texture_swizzle) {
            for (j = 0; j < 4; j++) {
                deltamax_swizzled[j] = deltamax[swizzle[j]];
            }
        } else {
            memcpy(deltamax_swizzled, deltamax, sizeof(deltamax));
        }

        /* Loop over all wrap modes. */
        for (j = 0; wrap_modes[j].mode != 0; j++) {
            unsigned char expected[4];
            int x0 = offset * (TILE_SIZE + TILE_SPACE) + 5;
            int y0 = i * (TILE_SIZE + TILE_SPACE) + 35;
            int a, b;

            if (!wrap_modes[j].supported)
                continue;

            if (skip_test(wrap_modes[j].mode, filter)) {
                if (skip_test(wrap_modes[j].mode, GL_LINEAR) !=
                    skip_test(wrap_modes[j].mode, GL_NEAREST)) {
                    offset++;
                }
                continue;
            }

            printf("Testing %s%s%s%s: %s\n",
                   sfilter,
                   texture_proj ? " with projective mapping" : "",
                   texture_swizzle ? (texture_proj ? " and" : " with") : "",
                   texture_swizzle ? " swizzling" : "",
                   wrap_modes[j].name);

            for (b = 0; b < (texture_size + BIAS_INT*2); b++) {
                for (a = 0; a < (texture_size + BIAS_INT*2); a++) {
                    double x = x0 + TEXEL_SIZE*(a+0.5);
                    double y = y0 + TEXEL_SIZE*(b+0.5);

                    sample_nearest(a - BIAS_INT, b - BIAS_INT,
                                   0, /* the slices are the same */
                                   wrap_modes[j].mode, filter, expected);

                    if (!probe_pixel_rgba(pixels, piglit_width, deltamax_swizzled,
                                          x, y, expected, a, b)) {
                        pass = GL_FALSE;
                        goto tile_done;
                    }
                }
            }

        tile_done:
            offset++;
        }
    }

    free(pixels);
    return pass;
}

static GLboolean test_simple()
{
    GLboolean pass;
    draw();
    pass = probe_pixels();
    piglit_present_results();
    return pass;
}

static GLboolean test_swizzle()
{
    GLboolean pass = GL_TRUE;
    pass = test_simple() && pass;
    if (has_texture_swizzle) {
        texture_swizzle = 1;
        update_swizzle();
        pass = test_simple() && pass;
        texture_swizzle = 0;
        update_swizzle();
    }
    return pass;
}

enum piglit_result piglit_display()
{
    GLboolean pass = GL_TRUE;

    if (piglit_automatic) {
        pass = test_swizzle();
    } else {
        if (has_texture_swizzle) {
            update_swizzle();
        }
        pass = test_simple();
    }
    assert(glGetError() == 0);

    return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static void key_func(unsigned char key, int x, int y)
{
    switch (key) {
    case 'b':
        if (texture_target == GL_TEXTURE_RECTANGLE_NV) {
            printf("The texture border cannot be used with rectangle "
                   "textures.\n");
            return;
        }
        if (texture_format->compressed) {
            printf("The texture border cannot be used with compressed "
                   "textures.\n");
            return;
        }

        texture_id = texture_id == NO_BORDER_TEXTURE ?
                     BORDER_TEXTURE : NO_BORDER_TEXTURE;

        printf(texture_id == NO_BORDER_TEXTURE ?
               "Without the border.\n" : "With the border.\n");
        break;

    case 'p':
        texture_proj = !texture_proj;
        break;

    case 's':
        texture_swizzle = !texture_swizzle;
        printf(texture_swizzle ?
               "Texture swizzle enabled.\n" : "Texture swizzle disabled.\n");
        break;
    }

    piglit_escape_exit_key(key, x, y);
}

static GLboolean check_support(float version, const char *extensions[2],
                               const char *dep_extension)
{
    float glversion = atof((char *)glGetString(GL_VERSION));

    if (version <= glversion)
        return GL_TRUE;

    if (((extensions[0] && piglit_is_extension_supported(extensions[0])) ||
         (extensions[1] && piglit_is_extension_supported(extensions[1]))) &&
        (!dep_extension || piglit_is_extension_supported(dep_extension)))
        return GL_TRUE;

    return GL_FALSE;
}

static void init_textures()
{
    int x, y, z;
    float red[4] = {1, 0, 0, 0.8};
    float cyan[4] = {0, 1, 1, 0.6};
    float blue[4] = {0, 0, 1, 0.4};
    float orange[4] = {1, 0.6, 0.3, 0.2};
    float white[4] = {1, 1, 1, 1};
    float black[4] = {0, 0, 0, 0};
    float *colors[] = {red, cyan, blue, orange, white, black, borderf_real};
    GLenum baseformat =
        texture_format->depth ?
        texture_format->stencil ? GL_DEPTH_STENCIL : GL_DEPTH_COMPONENT :
        GL_RGBA;
    GLenum type =
        texture_format->internalformat == GL_DEPTH24_STENCIL8 ? GL_UNSIGNED_INT_24_8 :
        texture_format->internalformat == GL_DEPTH32F_STENCIL8 ? GL_FLOAT_32_UNSIGNED_INT_24_8_REV :
        GL_FLOAT;

    memcpy(borderf_real, borderf, sizeof(borderf));

    /* Set the colors to match the base format. */
    if (texture_format->intensity) {
        for (x = 0; x < sizeof(colors)/sizeof(colors[0]); x++) {
            colors[x][3] = colors[x][2] = colors[x][1] = colors[x][0];
        }
    } else {
        if (texture_format->luminance || texture_format->depth) {
            for (x = 0; x < sizeof(colors)/sizeof(colors[0]); x++) {
                colors[x][2] = colors[x][1] = colors[x][0];
            }
        } else {
            if (texture_format->red == 0) {
                if (texture_format->alpha) {
                    for (x = 0; x < sizeof(colors)/sizeof(colors[0]); x++) {
                        colors[x][0] = 1;
                    }
                } else {
                    for (x = 0; x < sizeof(colors)/sizeof(colors[0]); x++) {
                        colors[x][0] = 0;
                    }
                }
            }
            if (texture_format->green == 0) {
                if (texture_format->alpha) {
                    for (x = 0; x < sizeof(colors)/sizeof(colors[0]); x++) {
                        colors[x][1] = 1;
                    }
                } else {
                    for (x = 0; x < sizeof(colors)/sizeof(colors[0]); x++) {
                        colors[x][1] = 0;
                    }
                }
            }
            if (texture_format->blue == 0) {
                if (texture_format->alpha) {
                    for (x = 0; x < sizeof(colors)/sizeof(colors[0]); x++) {
                        colors[x][2] = 1;
                    }
                } else {
                    for (x = 0; x < sizeof(colors)/sizeof(colors[0]); x++) {
                        colors[x][2] = 0;
                    }
                }
            }
        }
        if (texture_format->alpha == 0) {
            for (x = 0; x < sizeof(colors)/sizeof(colors[0]); x++) {
                colors[x][3] = 1;
            }
        }
    }

    switch (texture_target) {
    case GL_TEXTURE_3D:
        size_z = texture_size;
        bsize_z = texture_size+2;
        border_z = 1;
        /* Pass through. */
    case GL_TEXTURE_2D:
    case GL_TEXTURE_RECTANGLE_NV:
        size_y = texture_size;
        bsize_y = texture_size+2;
        border_y = 1;
        /* Pass through. */
    case GL_TEXTURE_1D:
        size_x = texture_size;
        bsize_x = texture_size+2;
        border_x = 1;
    }

    if ((!piglit_automatic || texture_id == BORDER_TEXTURE) &&
        texture_target != GL_TEXTURE_RECTANGLE_NV &&
        !texture_format->compressed) {
        for (z = 0; z < bsize_z; z++) {
            for (y = 0; y < bsize_y; y++) {
                for (x = 0; x < bsize_x; x++) {
                    unsigned i = (z*bsize_y*bsize_x + y*bsize_x + x)*4;

                    if (y == border_y && x == border_x) {
                        /* lower-left texel = RED */
                        border_image[i + 0] = red[0];
                        border_image[i + 1] = red[1];
                        border_image[i + 2] = red[2];
                        border_image[i + 3] = red[3];
                    } else if (y == border_y && x == bsize_x-border_x-1) {
                        /* lower-right corner = CYAN */
                        border_image[i + 0] = cyan[0];
                        border_image[i + 1] = cyan[1];
                        border_image[i + 2] = cyan[2];
                        border_image[i + 3] = cyan[3];
                    } else if (y == bsize_y-border_y-1 && x == border_x) {
                        /* upper-left corner = BLUE */
                        border_image[i + 0] = blue[0];
                        border_image[i + 1] = blue[1];
                        border_image[i + 2] = blue[2];
                        border_image[i + 3] = blue[3];
                    } else if (y == bsize_y-border_y-1 &&
                               x == bsize_x-border_x-1) {
                        /* upper-right corner = ORANGE */
                        border_image[i + 0] = orange[0];
                        border_image[i + 1] = orange[1];
                        border_image[i + 2] = orange[2];
                        border_image[i + 3] = orange[3];
                    } else if ((z == 0 && border_z) ||
                               (z == bsize_z-1 && border_z) ||
                               (y == 0 && border_y) ||
                               (y == bsize_y-1 && border_y) ||
                               (x == 0 && border_x) ||
                               (x == bsize_x-1 && border_x)) {
                        /* border color */
                        border_image[i + 0] = borderf_real[0];
                        border_image[i + 1] = borderf_real[1];
                        border_image[i + 2] = borderf_real[2];
                        border_image[i + 3] = borderf_real[3];
                    } else if ((y + x - !border_y) & 1) {
                        /* white */
                        border_image[i + 0] = white[0];
                        border_image[i + 1] = white[1];
                        border_image[i + 2] = white[2];
                        border_image[i + 3] = white[3];
                    } else {
                        /* black */
                        border_image[i + 0] = black[0];
                        border_image[i + 1] = black[1];
                        border_image[i + 2] = black[2];
                        border_image[i + 3] = black[3];
                    }
                }
            }
        }

        /* Convert to one-channel texture. Not nice, but easy. */
        if (texture_format->depth) {
            for (x = 1; x < bsize_z*bsize_y*bsize_x; x++) {
                border_image[x] = border_image[x*4];
            }
        }

        glBindTexture(texture_target, BORDER_TEXTURE);
        switch (texture_target) {
        case GL_TEXTURE_1D:
            glTexImage1D(texture_target, 0, texture_format->internalformat,
                         bsize_x, 1,
                         baseformat, type, (void *) border_image);
            break;

        case GL_TEXTURE_2D:
        case GL_TEXTURE_RECTANGLE_NV:
            glTexImage2D(texture_target, 0, texture_format->internalformat,
                         bsize_x, bsize_y, 1,
                         baseformat, type, (void *) border_image);
            break;

        case GL_TEXTURE_3D:
            if (1.2 <= atof((char *)glGetString(GL_VERSION)))
                glTexImage3D(texture_target, 0, texture_format->internalformat,
                             bsize_x, bsize_y, bsize_z, 1,
                             baseformat, type, (void *) border_image);
            else
                glTexImage3DEXT(texture_target, 0, texture_format->internalformat,
                                bsize_x, bsize_y, bsize_z, 1,
                                baseformat, type, (void *) border_image);
            break;
        }
        assert(glGetError() == 0);
    }

    if (!piglit_automatic || texture_id == NO_BORDER_TEXTURE) {
        float *data;
        unsigned real_size_x, real_size_y;

        for (z = 0; z < size_z; z++) {
            for (y = 0; y < size_y; y++) {
                for (x = 0; x < size_x; x++) {
                    unsigned i = (z*size_y*size_x + y*size_x + x)*4;

                    if (y == 0 && x == 0) {
                        /* lower-left texel = RED */
                        no_border_image[i + 0] = red[0];
                        no_border_image[i + 1] = red[1];
                        no_border_image[i + 2] = red[2];
                        no_border_image[i + 3] = red[3];
                    } else if (y == 0 && x == size_x-1) {
                        /* lower-right corner = CYAN */
                        no_border_image[i + 0] = cyan[0];
                        no_border_image[i + 1] = cyan[1];
                        no_border_image[i + 2] = cyan[2];
                        no_border_image[i + 3] = cyan[3];
                    } else if (y == size_y-1 && x == 0) {
                        /* upper-left corner = BLUE */
                        no_border_image[i + 0] = blue[0];
                        no_border_image[i + 1] = blue[1];
                        no_border_image[i + 2] = blue[2];
                        no_border_image[i + 3] = blue[3];
                    } else if (y == size_y-1 && x == size_x-1) {
                        /* upper-right corner = ORANGE */
                        no_border_image[i + 0] = orange[0];
                        no_border_image[i + 1] = orange[1];
                        no_border_image[i + 2] = orange[2];
                        no_border_image[i + 3] = orange[3];
                    } else if ((y + x) & 1) {
                        /* white */
                        no_border_image[i + 0] = white[0];
                        no_border_image[i + 1] = white[1];
                        no_border_image[i + 2] = white[2];
                        no_border_image[i + 3] = white[3];
                    } else {
                        /* black */
                        no_border_image[i + 0] = black[0];
                        no_border_image[i + 1] = black[1];
                        no_border_image[i + 2] = black[2];
                        no_border_image[i + 3] = black[3];
                    }
                }
            }
        }

        /* Convert to one-channel texture. Not nice, but easy. */
        if (texture_format->depth) {
            for (x = 1; x < size_z*size_y*size_x; x++) {
                no_border_image[x] = no_border_image[x*4];
            }
        }

        /* Expand pixels to 4x4 blocks of one color to get
         * "lossless compression". */
        if (texture_format->compressed) {
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
                                data[dst+0] = no_border_image[src+0];
                                data[dst+1] = no_border_image[src+1];
                                data[dst+2] = no_border_image[src+2];
                                data[dst+3] = no_border_image[src+3];
                            }
                        }

                        if (texture_format->internalformat ==
                            GL_COMPRESSED_RGBA_S3TC_DXT1 &&
                            no_border_image[src+3] < 0.5) {
                            /* DXT1: If the 1-bit alpha is black,
                             * the RGB color is black as well. */
                            no_border_image[src+0] = 0;
                            no_border_image[src+1] = 0;
                            no_border_image[src+2] = 0;
                            no_border_image[src+3] = 0;
                        }
                    }
                }
            }

            real_size_x = size_x*4;
            real_size_y = size_y*4;
        } else {
            data = no_border_image;
            real_size_x = size_x;
            real_size_y = size_y;
        }

        switch (texture_format->internalformat) {
        case GL_DEPTH24_STENCIL8:
            /* Convert to D24X8_UNORM. */
           {
                uint32_t *p;

                if (data != no_border_image) {
                    free(data);
                }

                p = (uint32_t*)(data = malloc(SIZEMAX * SIZEMAX * SIZEMAX * 4));

                for (x = 0; x < size_z*size_y*size_x; x++) {
                    p[x] = (uint32_t)(no_border_image[x] * ((1<<24) - 1)) << 8;
                }
            }
            break;
        case GL_DEPTH32F_STENCIL8:
            if (data != no_border_image) {
                free(data);
            }

            /* Convert to D32F_X24X8. */
            data = malloc(SIZEMAX * SIZEMAX * SIZEMAX * 8);

            for (x = 0; x < size_z*size_y*size_x; x++) {
                data[x*2] = no_border_image[x];
            }
            break;
        }

        glBindTexture(texture_target, NO_BORDER_TEXTURE);
        glTexParameterfv(texture_target, GL_TEXTURE_BORDER_COLOR, borderf);
        switch (texture_target) {
        case GL_TEXTURE_1D:
            glTexImage1D(texture_target, 0, texture_format->internalformat,
                         real_size_x, 0,
                         baseformat, type, (void *) data);
            break;

        case GL_TEXTURE_2D:
        case GL_TEXTURE_RECTANGLE_NV:
            glTexImage2D(texture_target, 0, texture_format->internalformat,
                         real_size_x, real_size_y, 0,
                         baseformat, type, (void *) data);
            break;

        case GL_TEXTURE_3D:
            if (1.2 <= atof((char *)glGetString(GL_VERSION)))
                glTexImage3D(texture_target, 0, texture_format->internalformat,
                             real_size_x, real_size_y, size_z, 0,
                             baseformat, type, (void *) data);
            else
                glTexImage3DEXT(texture_target, 0, texture_format->internalformat,
                                real_size_x, real_size_y, size_z, 0,
                                baseformat, type, (void *) data);
            break;
        }
        assert(glGetError() == 0);

        if (data != no_border_image) {
            free(data);
        }
    }
}

GLboolean is_format_supported(struct format *f)
{
    GLuint id;
    float p[4] = {0};
    int r, g, b, a, l, i, d, s, iformat;
    GLenum baseformat =
        f->depth ?
        f->stencil ? GL_DEPTH_STENCIL : GL_DEPTH_COMPONENT :
        GL_RGBA;
    GLenum type =
        f->internalformat == GL_DEPTH24_STENCIL8 ? GL_UNSIGNED_INT_24_8 :
        f->internalformat == GL_DEPTH32F_STENCIL8 ? GL_FLOAT_32_UNSIGNED_INT_24_8_REV :
        GL_FLOAT;

    if (!check_support(f->version, f->extensions, f->dependency_extension))
        return GL_FALSE;

    /* A quick and dirty way to check if we get the format we want. */
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, f->internalformat, 1, 1, 0, baseformat, type, &p);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &r);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &g);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &b);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &a);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_LUMINANCE_SIZE, &l);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTENSITY_SIZE, &i);

    if (f->depth)
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_DEPTH_SIZE, &d);
    else
        d = 0;

    if (f->stencil)
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_STENCIL_SIZE, &s);
    else
        s = 0;

    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT,
                             &iformat);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &id);
    assert(glGetError() == 0);

    printf("%s has bits R%iG%iB%iA%i L%i I%i D%iS%i. The internal format is 0x%04X.\n",
           f->name, r, g, b, a, l, i, d, s, iformat);

    if (r != f->red ||
        (g != f->green && g-1 != f->green) ||
        b != f->blue ||
        a != f->alpha ||
        l != f->luminance ||
        i != f->intensity ||
        d != f->depth ||
        s != f->stencil ||
        iformat != f->internalformat) {
#if 0
        fprintf(stderr,
                "The used format appears to be different "
                "from the requested format. "
                "Is the internal format unsupported/being faked?\n");
#endif
    }

    return GL_TRUE;
}

void piglit_init(int argc, char **argv)
{
    unsigned i, p;
    const char *ext_swizzle[] = {
        "GL_ARB_texture_swizzle",
        "GL_EXT_texture_swizzle"
    };

    texture_target = GL_TEXTURE_2D;
    texture_id = NO_BORDER_TEXTURE;
    texture_npot = 0;
    texture_proj = 0;
    texture_swizzle = 0;
    texture_format = &formats[0];
    has_texture_swizzle = check_support(3.3, ext_swizzle, NULL);

    piglit_require_extension("GL_ARB_window_pos");

    for (p = 1; p < argc; p++) {
        printf("Parameter: %s\n", argv[p]);

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
            const char *extensions[2] = {"GL_EXT_texture3D", NULL};
            if (!check_support(1.2, extensions, NULL))
                piglit_report_result(PIGLIT_SKIP);

            texture_target = GL_TEXTURE_3D;
            printf("Using TEXTURE_3D.\n");
            continue;
        }
        if (strcmp(argv[p], "RECT") == 0) {
            const char *extensions[2] = {"GL_ARB_texture_rectangle",
                                         "GL_NV_texture_rectangle"};
            if (!check_support(3.1, extensions, NULL))
                piglit_report_result(PIGLIT_SKIP);

            texture_target = GL_TEXTURE_RECTANGLE_NV;
            texture_npot = GL_TRUE; /* Enforce NPOT dimensions. */
            printf("Using TEXTURE_RECTANGLE.\n");
            continue;
        }

        /* Parameters: npot, border */
        if (strcmp(argv[p], "npot") == 0) {
            piglit_require_extension("GL_ARB_texture_non_power_of_two");
            texture_npot = 1;
            printf("Using NPOT dimensions.\n");
            continue;
        }
        if (strcmp(argv[p], "border") == 0) {
            assert(texture_target != GL_TEXTURE_RECTANGLE_NV);
            texture_id = BORDER_TEXTURE;
            printf("Using the border.\n");
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

        /* Formats. */
        for (i = 0; formats[i].name; i++) {
            if (strcmp(argv[p], formats[i].name) == 0) {
                if (!is_format_supported(&formats[i]))
                    piglit_report_result(PIGLIT_SKIP);

                texture_format = &formats[i];
                printf("Using %s.\n", formats[i].name);
                goto outer_continue;
            }
        }

        printf("Error: Unknown parameter\n");
        piglit_report_result(PIGLIT_SKIP);

    outer_continue:;
    }

    texture_size = texture_npot ? SIZE_NPOT : SIZE_POT;

    /* Check wrap extensions. */
    for (i = 0 ; wrap_modes[i].mode != 0 ; i++) {
        if (texture_target == GL_TEXTURE_RECTANGLE_NV &&
            !wrap_modes[i].valid_for_rect) {
            wrap_modes[i].supported = GL_FALSE;
        } else {
            wrap_modes[i].supported =
                check_support(wrap_modes[i].version,
                              wrap_modes[i].extensions, NULL);
        }
    }

    piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

    glClearColor(0.5, 0.5, 0.5, 1.0);

    assert(glGetError() == 0);
    init_textures();

    if (!piglit_automatic) {
        piglit_set_keyboard_func(key_func);
        printf("Hotkeys in the interactive mode:\n"
               "    b  - use 1-pixel texture border (deprecated in GL3)\n"
               "    p  - use projective texturing\n"
               "    s  - use texture swizzling (ARB_texture_swizzle)\n");
    }
    printf("\nHint: If you only fail CLAMP and CLAMP_TO_BORDER tests, "
           "the border color is wrong.\n\n");
}
