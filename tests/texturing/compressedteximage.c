/*
 * Copyright © 2011 Intel Corporation
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

/** @file compressedteximage.c
 *
 * Tests that fetching and uploading compressed texture data works
 * correctly.
 *
 * The other compressed texture tests are about decoding of data that
 * was uploaded from uncompressed, while this tries a round-trip after
 * the initial upload, testing glGetCompressedTexImage() and
 * glCompressedTexImage2D().
 */

#include "piglit-util-gl.h"

#define SIZE 128

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = (SIZE*2)+60;
	config.window_height = SIZE+20;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

struct format {
	const char *name;
	GLenum token;
	const char **extension;
};

static struct format *format;

const char *FXT1[] = {
	"GL_3DFX_texture_compression_FXT1",
	NULL
};

const char *S3TC[] = {
	"GL_EXT_texture_compression_s3tc",
	NULL
};

const char *S3TC_srgb[] = {
	"GL_EXT_texture_compression_s3tc",
	"GL_EXT_texture_sRGB",
	NULL
};

const char *RGTC[] = {
	"GL_ARB_texture_compression_rgtc",
	NULL
};

const char *RGTC_signed[] = {
	"GL_ARB_texture_compression_rgtc",
	"GL_EXT_texture_snorm",
	NULL
};

const char *BPTC[] = {
	"GL_ARB_texture_compression_bptc",
	NULL
};

#define FORMAT(t, ext) { #t, t, ext }
static struct format formats[] = {
	FORMAT(GL_COMPRESSED_RGB_FXT1_3DFX, FXT1),
	FORMAT(GL_COMPRESSED_RGBA_FXT1_3DFX, FXT1),

	FORMAT(GL_COMPRESSED_RGB_S3TC_DXT1_EXT, S3TC),
	FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, S3TC),
	FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, S3TC),
	FORMAT(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, S3TC),

	FORMAT(GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, S3TC_srgb),
	FORMAT(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, S3TC_srgb),
	FORMAT(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, S3TC_srgb),
	FORMAT(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, S3TC_srgb),

	FORMAT(GL_COMPRESSED_RGBA_BPTC_UNORM, BPTC),
	FORMAT(GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM, BPTC),
	FORMAT(GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT, BPTC),
	FORMAT(GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT, BPTC),

	FORMAT(GL_COMPRESSED_RED_RGTC1_EXT, RGTC),
	FORMAT(GL_COMPRESSED_SIGNED_RED_RGTC1_EXT, RGTC_signed),
	FORMAT(GL_COMPRESSED_RED_GREEN_RGTC2_EXT, RGTC),
	FORMAT(GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT, RGTC_signed),
};

static void
display_mipmaps(int x, int y)
{
	int i;

	glEnable(GL_TEXTURE_2D);

	/* Disply all the mipmap levels */
	for (i = SIZE; i > 0; i /= 2) {
		piglit_draw_rect_tex(x, y, i, i,
				     0, 0, 1, 1);

		x += i + 5;
	}
}

static GLboolean
check_resulting_mipmaps(int x, int y)
{
	GLboolean pass = GL_TRUE;
	int size;
	float red[4] =   {1.0, 0.0, 0.0, 1.0};
	float green[4] = {0.0, 1.0, 0.0, 1.0};
	float blue[4] =  {0.0, 0.0, 1.0, 1.0};
	float white[4] = {1.0, 1.0, 1.0, 1.0};

	/* for r, rg textures, overwrite what the expected colors are. */
	if (format->token == GL_COMPRESSED_RED_RGTC1_EXT ||
	    format->token == GL_COMPRESSED_SIGNED_RED_RGTC1_EXT) {
		green[1] = 0;
		blue[2] = 0;
		white[1] = 0;
		white[2] = 0;
	}
	if (format->token == GL_COMPRESSED_RED_GREEN_RGTC2_EXT ||
	    format->token == GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT) {
		blue[2] = 0;
		white[2] = 0;
	}

	for (size = SIZE; size > 0; size /= 2) {
		if (size == 4)
			pass = pass && piglit_probe_pixel_rgb(x + 2, y + 2,
							      red);
		else if (size == 2)
			pass = pass && piglit_probe_pixel_rgb(x + 1, y + 1,
							      green);
		else if (size == 1)
			pass = pass && piglit_probe_pixel_rgb(x, y,
							      blue);
		else {
			pass = pass && piglit_probe_pixel_rgb(x + size / 4,
							      y + size / 4,
							      red);
			pass = pass && piglit_probe_pixel_rgb(x + size * 3 / 4,
							      y + size / 4,
							      green);
			pass = pass && piglit_probe_pixel_rgb(x + size / 4,
							      y + size * 3 / 4,
							      blue);
			pass = pass && piglit_probe_pixel_rgb(x + size * 3 / 4,
							      y + size * 3 / 4,
							      white);
		}
		x += size + 5;
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLuint tex, tex_src;
	bool pass;
	int level;
	unsigned bw, bh, bs;

	piglit_get_compressed_block_size(format->token, &bw, &bh, &bs);
	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	tex_src = piglit_rgbw_texture(format->token, SIZE, SIZE,
				      GL_TRUE, GL_FALSE,
				      GL_UNSIGNED_NORMALIZED);
	glGenTextures(1, &tex);

	for (level = 0; (SIZE >> level) > 0; level++) {
		int w, h;
		int expected_size, size;
		void *compressed;

		w = SIZE >> level;
		h = SIZE >> level;
		expected_size = piglit_compressed_image_size(format->token, w, h);

		glBindTexture(GL_TEXTURE_2D, tex_src);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
					 GL_TEXTURE_COMPRESSED_IMAGE_SIZE,
					 &size);

		if (size != expected_size) {
			fprintf(stderr, "Format %s level %d (%dx%d) size %d "
				"doesn't match expected size %d\n",
				format->name, level, w, h, size, expected_size);
			piglit_report_result(PIGLIT_FAIL);
		}

		compressed = malloc(size);

		glGetCompressedTexImage(GL_TEXTURE_2D, level, compressed);

		glBindTexture(GL_TEXTURE_2D, tex);
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format->token,
				       w, h, 0, size, compressed);
		if (!piglit_check_gl_error(GL_NO_ERROR))
			piglit_report_result(PIGLIT_FAIL);

		free(compressed);
	}

	glDeleteTextures(1, &tex_src);
	glBindTexture(GL_TEXTURE_2D, tex);

	display_mipmaps(10, 10);
	pass = check_resulting_mipmaps(10, 10);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static void
usage(int argc, char **argv)
{
	int i;

	fprintf(stderr, "Usage: %s <format>\n", argv[0]);
	fprintf(stderr, "format is one of:\n");
	for (i = 0; i < ARRAY_SIZE(formats); i++) {
		fprintf(stderr, "  %s\n", formats[i].name);
	}
	exit(1);
}

void
piglit_init(int argc, char **argv)
{
	int i;


	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	if (argc != 2)
		usage(argc, argv);

	format = NULL;

	for (i = 0; i < ARRAY_SIZE(formats); i++) {
		if (strcmp(formats[i].name, argv[1]) == 0) {
			format = &formats[i];
			break;
		}
	}

	if (!format)
		usage(argc, argv);

	for (i = 0; format->extension[i]; i++)
		piglit_require_extension(format->extension[i]);
}
