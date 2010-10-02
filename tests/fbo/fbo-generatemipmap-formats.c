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

/** @file fbo-generatemipmap-formats.c
 *
 * Tests that glGenerateMipmapEXT works correctly on 2D textures of various
 * internalformats.
 */

#include "piglit-util.h"

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

#define TEX_WIDTH 256
#define TEX_HEIGHT 256
int piglit_width = 700;
int piglit_height = 300;
int piglit_window_mode = GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA;

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
	{0, 0}
};

static const struct format_desc arb_depth_texture[] = {
	FORMAT(GL_DEPTH_COMPONENT),
	FORMAT(GL_DEPTH_COMPONENT16),
	FORMAT(GL_DEPTH_COMPONENT24),
	FORMAT(GL_DEPTH_COMPONENT32),
	{0, 0}
};

static const struct format_desc ext_packed_depth_stencil[] = {
	FORMAT(GL_DEPTH_STENCIL_EXT),
	FORMAT(GL_DEPTH24_STENCIL8_EXT),
	{0, 0}
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
	{0, 0}
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
	{0, 0}
};

static const struct format_desc ext_texture_compression[] = {
	FORMAT(GL_COMPRESSED_ALPHA),
	FORMAT(GL_COMPRESSED_LUMINANCE),
	FORMAT(GL_COMPRESSED_LUMINANCE_ALPHA),
	FORMAT(GL_COMPRESSED_INTENSITY),
	FORMAT(GL_COMPRESSED_RGB),
	FORMAT(GL_COMPRESSED_RGBA),
	{0, 0}
};

static const struct format_desc tdfx_texture_compression_fxt1[] = {
	FORMAT(GL_COMPRESSED_RGB_FXT1_3DFX),
	FORMAT(GL_COMPRESSED_RGBA_FXT1_3DFX),
	{0, 0}
};

static const struct format_desc ext_texture_compression_s3tc[] = {
	FORMAT(GL_COMPRESSED_RGB_S3TC_DXT1_EXT),
	FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT),
	FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT),
	FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT),
	{0, 0}
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
	{0, 0}
};

static const struct format_desc arb_texture_rg[] = {
	FORMAT(GL_R8),
	FORMAT(GL_R16),
	FORMAT(GL_RG),
	FORMAT(GL_RG8),
	FORMAT(GL_RG16),
	{0, 0}
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
	{0, 0}
};

static const struct format_desc arb_texture_rg_float[] = {
	FORMAT(GL_R16F),
	FORMAT(GL_R32F),
	FORMAT(GL_RG16F),
	FORMAT(GL_RG32F),
	{0, 0}
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

const struct format_desc *test_set;

static GLboolean doing_depth = GL_FALSE;
static GLboolean doing_depth_stencil = GL_FALSE;

static int
create_tex(GLenum internalformat)
{
	GLuint tex;
	int i, dim;
	GLenum type, format;

	if (doing_depth || doing_depth_stencil) {
		tex = piglit_depth_texture(internalformat,
					   TEX_WIDTH, TEX_HEIGHT, GL_FALSE);
		if (doing_depth) {
			format = GL_DEPTH_COMPONENT;
			type = GL_FLOAT;
		} else {
			format = GL_DEPTH_STENCIL_EXT;
			type = GL_UNSIGNED_INT_24_8_EXT;
		}
	} else {
		tex = piglit_rgbw_texture(internalformat,
					  TEX_WIDTH, TEX_HEIGHT, GL_FALSE);
		format = GL_RGBA;
		type = GL_FLOAT;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_NEAREST);

	for (i = 1, dim = TEX_WIDTH; dim >0; i++, dim /= 2) {
		glTexImage2D(GL_TEXTURE_2D, i, internalformat,
			     dim, dim,
			     0,
			     format, type, NULL);
	}

	assert(glGetError() == 0);

	glGenerateMipmapEXT(GL_TEXTURE_2D);

	return tex;
}

static void
draw_mipmap(int x, int y, int dim)
{
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	piglit_draw_rect_tex(x, y, dim, dim,
			     0, 0, 1, 1);
}

static GLboolean
test_mipmap_drawing(int x, int y, int dim, int level)
{
	GLboolean pass = GL_TRUE;
	int half = dim / 2;
	int x1 = x, y1 = y, x2 = x + half, y2 = y + half;
	float r[] = {1, 0, 0, 1};
	float g[] = {0, 1, 0, 1};
	float b[] = {0, 0, 1, 1};
	float w[] = {1, 1, 1, 1};
	GLint r_size, g_size, b_size, l_size, a_size, d_size, i_size;
	GLint compressed;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_COMPRESSED, &compressed);
	if (compressed && dim < 8)
		return GL_TRUE;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_DEPTH_SIZE, &d_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_LUMINANCE_SIZE, &l_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_ALPHA_SIZE, &a_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_INTENSITY_SIZE, &i_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_RED_SIZE, &r_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_GREEN_SIZE, &g_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_BLUE_SIZE, &b_size);

	if (d_size) {
		for (x1 = x; x1 < x + dim; x1++) {
			float val = (x1 - x + 0.5) / (dim);
			float color[3] = {val, val, val};
			pass = pass && piglit_probe_rect_rgb(x1, y, 1, dim,
							     color);
		}
		return pass;
	}

	if (i_size || l_size) {
		r[0] = 1.0;
		r[1] = 1.0;
		r[2] = 1.0;

		g[0] = 0.0;
		g[1] = 0.0;
		g[2] = 0.0;

		b[0] = 0.0;
		b[1] = 0.0;
		b[2] = 0.0;

		if (i_size) {
			g[3] = 0.0;
			b[3] = 0.0;
		}
	} else if (a_size && !r_size && !l_size) {
		r[0] = 1.0;
		r[1] = 1.0;
		r[2] = 1.0;
		g[0] = 1.0;
		g[1] = 1.0;
		g[2] = 1.0;
		b[0] = 1.0;
		b[1] = 1.0;
		b[2] = 1.0;
	} else {
		if (!r_size) {
			r[0] = 0.0;
			w[0] = 0.0;
		}

		if (!g_size) {
			g[1] = 0.0;
			w[1] = 0.0;
		}

		if (!b_size) {
			b[2] = 0.0;
			w[2] = 0.0;
		}
	}

	/* alpha_size == 0 just means alpha reads as 1.0. */

	pass = pass && piglit_probe_rect_rgba(x1, y1, half, half, r);
	pass = pass && piglit_probe_rect_rgba(x2, y1, half, half, g);
	pass = pass && piglit_probe_rect_rgba(x1, y2, half, half, b);
	pass = pass && piglit_probe_rect_rgba(x2, y2, half, half, w);

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int i;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; test_set[i].name; i++) {
		int dim;
		GLuint tex;
		int x;
		int level;
		GLboolean test_pass = GL_TRUE;

		printf("Testing %s\n", test_set[i].name);
		tex = create_tex(test_set[i].internalformat);

		x = 1;
		for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
			draw_mipmap(x, 1, dim);
			x += dim + 1;
		}

		x = 1;
		level = 0;
		for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
			test_pass = test_pass && test_mipmap_drawing(x, 1, dim, level);
			x += dim + 1;
			level++;
		}
		pass = pass && test_pass;

		glDeleteTextures(1, &tex);
	}

	glutSwapBuffers();

	return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}


void piglit_init(int argc, char **argv)
{
	int i;

	piglit_require_extension("GL_EXT_framebuffer_object");

	test_set = core;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "GL_ARB_texture_compression")) {
			piglit_require_extension("GL_ARB_texture_compression");
			test_set = ext_texture_compression;
		} else if (!strcmp(argv[i], "GL_3DFX_texture_compression_FXT1")) {
			piglit_require_extension("GL_ARB_texture_compression");
			piglit_require_extension("GL_3DFX_texture_compression_FXT1");
			test_set = tdfx_texture_compression_fxt1;
		} else if (!strcmp(argv[i], "GL_EXT_texture_compression_s3tc")) {
			piglit_require_extension("GL_ARB_texture_compression");
			piglit_require_extension("GL_EXT_texture_compression_s3tc");
			test_set = ext_texture_compression_s3tc;
		} else if (!strcmp(argv[i], "GL_ARB_depth_texture")) {
			piglit_require_extension("GL_ARB_depth_texture");
			test_set = arb_depth_texture;
			doing_depth = GL_TRUE;
		} else if (!strcmp(argv[i], "GL_EXT_packed_depth_stencil")) {
			piglit_require_extension("GL_EXT_packed_depth_stencil");
			test_set = ext_packed_depth_stencil;
			doing_depth_stencil = GL_TRUE;
		} else if (!strcmp(argv[i], "GL_EXT_texture_sRGB")) {
			piglit_require_extension("GL_EXT_texture_sRGB");
			test_set = ext_texture_srgb;
		} else if (!strcmp(argv[i], "GL_EXT_texture_sRGB-s3tc")) {
			piglit_require_extension("GL_EXT_texture_sRGB");
			piglit_require_extension("GL_ARB_texture_compression");
			piglit_require_extension("GL_EXT_texture_compression_s3tc");
			test_set = ext_texture_srgb_compressed;
		} else if (!strcmp(argv[i], "GL_EXT_texture_integer")) {
			piglit_require_extension("GL_EXT_texture_integer");
			test_set = ext_texture_integer;
		} else if (!strcmp(argv[i], "GL_ARB_texture_rg")) {
			piglit_require_extension("GL_ARB_texture_rg");
			test_set = arb_texture_rg;
		} else if (!strcmp(argv[i], "GL_ARB_texture_rg-int")) {
			piglit_require_extension("GL_ARB_texture_rg");
			piglit_require_extension("GL_EXT_texture_integer");
			test_set = arb_texture_rg_int;
		} else if (!strcmp(argv[i], "GL_ARB_texture_rg-float")) {
			piglit_require_extension("GL_ARB_texture_rg");
			piglit_require_extension("GL_ARB_texture_float");
			test_set = arb_texture_rg_float;
		} else if (!strcmp(argv[i], "GL_EXT_texture_shared_exponent")) {
			piglit_require_extension("GL_EXT_texture_shared_exponent");
			test_set = ext_texture_shared_exponent;
		} else if (!strcmp(argv[i], "GL_EXT_texture_packed_float")) {
			piglit_require_extension("GL_EXT_texture_packed_float");
			test_set = ext_texture_shared_exponent;
		} else if (!strcmp(argv[i], "GL_ARB_depth_buffer_float")) {
			piglit_require_extension("GL_ARB_depth_buffer_float");
			test_set = arb_depth_buffer_float;
			doing_depth = GL_TRUE;
		} else if (!strcmp(argv[i], "GL_EXT_texture_compression_rgtc")) {
			piglit_require_extension("GL_EXT_texture_compression_rgtc");
			test_set = ext_texture_compression_rgtc;
		} else if (!strcmp(argv[i], "-auto")) {
		} else {
			fprintf(stderr, "Unknown argument: %s\n", argv[i]);
			exit(1);
		}
	}
}
