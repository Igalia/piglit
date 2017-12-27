/*
 * Copyright 2016 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Test that glTexSubImage*D works correctly with GL_UNPACK_ALIGNMENT,
 * component mapping, and type conversions.
 */

#include "piglit-util-gl.h"
#include "../fbo/fbo-formats.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

	config.window_width = 512;
	config.window_height = 512;

PIGLIT_GL_TEST_CONFIG_END

#define TEX_WIDTH 32
#define TEX_HEIGHT 16

/* If set to true then the texture sub image upload will be read
 * from a PBO */
static bool use_pbo = false;

static bool have_ARB_texture_rg = false;
static bool have_EXT_bgra = false;
static bool have_EXT_abgr = false;

struct src_format_desc {
	GLenum format;
	int8_t data_swizzle[4];
	int8_t tex_swizzle[4];
	bool *enable;
};

static const struct src_format_desc test_src_formats[] = {
#define X (-1)
#define Y (-2)
	/* This is first because we test it with all types. */
	{ GL_RGBA,            { 0, 1, 2, 3 }, { 0, 1, 2, 3 } },

	/* Remainder is in the order of Table 8.8 of the OpenGL 4.5
	 * (Compatibility Profile) spec. */
	{ GL_RED,             { 0, },         { 0, X, X, Y } },
	{ GL_GREEN,           { 1, },         { X, 1, X, Y } },
	{ GL_BLUE,            { 2, },         { X, X, 2, Y } },
	{ GL_ALPHA,           { 3, },         { X, X, X, 3 } },
	{ GL_RG,              { 0, 1 },       { 0, 1, X, Y }, &have_ARB_texture_rg },
	{ GL_RGB,             { 0, 1, 2, },   { 0, 1, 2, Y } },
	{ GL_BGR,             { 2, 1, 0, },   { 0, 1, 2, Y }, &have_EXT_bgra },
	{ GL_BGRA,            { 2, 1, 0, 3 }, { 0, 1, 2, 3 }, &have_EXT_bgra },
	{ GL_LUMINANCE,       { 0, },         { 0, 0, 0, Y } },
	{ GL_LUMINANCE_ALPHA, { 0, 3, },      { 0, 0, 0, 3 } },
	{ GL_ABGR_EXT,        { 3, 2, 1, 0 }, { 0, 1, 2, 3 }, &have_EXT_abgr },
#undef X
#undef Y
};

struct base_internal_format_desc {
	GLenum format;
	int8_t swizzle[4];
};

static const GLenum test_types[] = {
	GL_UNSIGNED_BYTE,
	GL_BYTE,
	GL_UNSIGNED_SHORT,
	GL_SHORT,
	GL_UNSIGNED_INT,
	GL_INT,
};

/* Sources of RGBA values read from framebuffer when drawing a texture with the
 * fixed function pipeline, depending on the texture base internal format.
 */
static const struct base_internal_format_desc base_internal_formats[] = {
#define X (-1)
#define Y (-2)
	{ GL_ALPHA,           { Y, Y, Y, 3 } },
	{ GL_INTENSITY,       { 0, 0, 0, 0 } },
	{ GL_LUMINANCE,       { 0, 0, 0, Y } },
	{ GL_LUMINANCE_ALPHA, { 0, 0, 0, 3 } },
	{ GL_RED,             { 0, X, X, Y } },
	{ GL_RG,              { 0, 1, X, Y } },
	{ GL_RGB,             { 0, 1, 2, Y } },
	{ GL_RGBA,            { 0, 1, 2, 3 } },
#undef X
#undef Y
};

static const struct base_internal_format_desc *
lookup_base_internal_format(GLenum format)
{
	for (unsigned i = 0; i < ARRAY_SIZE(base_internal_formats); ++i) {
		const struct base_internal_format_desc *desc = &base_internal_formats[i];
		if (desc->format == format)
			return desc;
	}

	fprintf(stderr, "bad base internal format\n");
	piglit_report_result(PIGLIT_FAIL);
}

/**
 * Draw each image of the texture to the framebuffer and then save the
 * entire thing to a buffer with glReadPixels().
 */
static void
draw_and_read_texture(GLuint w, GLuint h, GLubyte *ref)
{
	piglit_draw_rect_tex(0, 0, /* x/y */
			     w, h,
			     0.0, 0.0, /* tx/ty */
			     1.0, 1.0 /* tw/th */);

	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, ref);
}

static GLuint
create_texture(GLenum intFormat,
	       GLsizei w, GLsizei h,
	       GLenum srcFormat,
	       const GLubyte *img)
{
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, intFormat, w, h, 0,
		     srcFormat, GL_UNSIGNED_BYTE, img);

	return tex;
}

/**
 * Prepare to upload a region of update_img via glTexSubImage*D():
 *
 * - \p update_img is a straight-forward 32-bit RGBA format
 * - src_format and type describe the type and format to be used with
 *   glTexSubImage2D()
 * - alignment is the pixel unpack alignment
 *
 * - the data to be sent to the GL in \p upload will be written to upload
 * - the (straight-forward 32-bit RGBA format) \p ref_img image will be
 *   modified at the relevant pixels
 *
 * \return the number of bytes written to \p upload
 */
static unsigned
prepare_upload(const GLubyte *update_img, const GLubyte *update_ref,
	       GLuint w, GLuint h,
	       const struct base_internal_format_desc *base_format_desc,
	       const struct src_format_desc *src_format,
	       GLenum type,
	       GLint alignment,
	       GLuint tx, GLuint ty,
	       GLuint tw, GLuint th,
	       GLubyte *update_swz_ref,
	       GLubyte *upload)
{
	unsigned dst = 0;
	unsigned components = piglit_num_components(src_format->format);

	for (unsigned y = 0; y < th; ++y) {
		unsigned src = 4 * (tx + w * (ty + y));
		const GLubyte *img_tex = update_img + src;
		const GLubyte *orig_ref_tex = update_ref + src;
		GLubyte *swz_tex = update_swz_ref + src;
		for (unsigned x = 0; x < tw; ++x, img_tex += 4, orig_ref_tex += 4, swz_tex += 4) {
			for (unsigned i = 0; i < components; ++i) {
				GLubyte c = orig_ref_tex[src_format->data_swizzle[i]];
				switch (type) {
				case GL_UNSIGNED_BYTE:
					upload[dst] = c;
					++dst;
					break;
				case GL_BYTE:
					upload[dst] = c / 2;
					++dst;
					break;
				case GL_UNSIGNED_SHORT:
					*(GLushort *)(upload + dst) = (GLuint)c * 0x0101U;
					dst += 2;
					break;
				case GL_SHORT:
					*(GLshort *)(upload + dst) = ((GLuint)c * 0x0101U) / 2;
					dst += 2;
					break;
				case GL_UNSIGNED_INT:
					*(GLuint *)(upload + dst) = (GLuint)c * 0x01010101U;
					dst += 4;
					break;
				case GL_INT:
					*(GLint *)(upload + dst) = ((GLuint)c * 0x01010101U) / 2;
					dst += 4;
					break;
				default:
					assert(!"unsupported type");
				}
			}

			for (unsigned i = 0; i < 4; ++i) {
				int swz = base_format_desc->swizzle[i];
				if (swz >= 0)
					swz = src_format->tex_swizzle[swz];
				if (swz >= 0)
					swz_tex[i] = orig_ref_tex[swz];
				else if (swz == -2)
					swz_tex[i] = 255;
				else if (swz == -1)
					swz_tex[i] = 0;
			}
		}
		dst = (dst + alignment - 1) & ~(alignment - 1);
	}

	return dst;
}

static bool
test_formats_type(const struct format_desc *intFormat,
		  GLuint w,  GLuint h,
		  const struct src_format_desc *srcFormat, GLenum type,
		  const GLubyte *original_img, const GLubyte *original_ref,
		  const GLubyte *updated_img, const GLubyte *updated_ref,
		  GLuint pbo,
		  GLubyte *updated_swz_ref, GLubyte *upload_data, GLubyte *testImg)
{
	bool pass = true;
	const struct base_internal_format_desc *base_format_desc
		= lookup_base_internal_format(intFormat->base_internal_format);
	unsigned bits = intFormat->min_bits;

	if (!bits || bits > 8)
		bits = 8;
	if (type == GL_BYTE && bits > 7)
		bits = 7;

	for (unsigned t = 0; t < 3 && pass; t++) {
		GLuint tex;
		unsigned upload_bytes;

		/* Choose random region of texture to update.
		 * Use sizes and positions that are multiples of
		 * the compressed block size.
		 */
		GLint tw = 1 + rand() % w;
		GLint th = 1 + rand() % h;
		GLint tx = rand() % (w - tw + 1);
		GLint ty = rand() % (h - th + 1);

		/* Choose a random alignment. */
		static const GLint alignments[] = { 1, 2, 4, 8 };
		GLint alignment = alignments[rand() % ARRAY_SIZE(alignments)];

		assert(tx + tw <= w);
		assert(ty + th <= h);

		/* Recreate the original texture */
		tex = create_texture(intFormat->internalformat, w, h,
				     GL_RGBA, original_img);

		upload_bytes = prepare_upload(
			updated_img, updated_ref, w, h,
			base_format_desc, srcFormat, type, alignment,
			tx, ty, tw, th,
			updated_swz_ref, upload_data);

		if (use_pbo) {
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
			glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, upload_bytes, upload_data);
		}

		/* replace texture region with data from updated image */
		glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
		glTexSubImage2D(GL_TEXTURE_2D, 0, tx, ty, tw, th,
				srcFormat->format, type,
				use_pbo ? NULL : upload_data);

		if (use_pbo)
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		/* draw test image */
		glClear(GL_COLOR_BUFFER_BIT);
		draw_and_read_texture(w, h, testImg);

		glDeleteTextures(1, &tex);

		piglit_present_results();

		if (!piglit_equal_images_update_rgba8(
				original_ref, updated_swz_ref, testImg,
				w, h, 1, tx, ty, 0, tw, th, 1,
				bits)) {
			printf("texsubimage-unpack failed\n");
			printf("  internal format: %s\n", get_format_name(intFormat->internalformat));
			printf("  format: %s\n", piglit_get_gl_enum_name(srcFormat->format));
			printf("  type: %s\n", piglit_get_gl_enum_name(type));
			printf("  alignment: %d\n", alignment);
			printf("  region: %d, %d  %d x %d\n", tx, ty, tw, th);
			pass = false;
		}
	}

	return pass;
}

/**
 * Create two different images, draw both of them to the framebuffer
 * via textures of the given \p intFormat and read them back to obtain the
 * reference data.
 *
 * Then, for various combinations of (source data) formats and types,
 * create a texture with the original image plus a random rectangle changed
 * to the updated image. Draw that texture, read it back, and compare the
 * results to what we expect.
 */
static bool
test_format(const struct format_desc *intFormat)
{
	const GLuint w = TEX_WIDTH;
	const GLuint h = TEX_HEIGHT;
	GLuint tex, j, k, n;
	GLubyte *original_img, *original_ref;
	GLubyte *updated_img, *updated_ref;
	GLubyte *updated_swz_ref;
	GLubyte *upload_data, *testImg;
	bool pass = true;
	GLuint pbo = 0;

	original_img = (GLubyte *) malloc(w * h * 4);
	original_ref = (GLubyte *) malloc(w * h * 4);
	updated_img = (GLubyte *) malloc(w * h * 4);
	updated_ref = (GLubyte *) malloc(w * h * 4);
	updated_swz_ref = (GLubyte *) malloc(w * h * 4);
	/* Allow generous extra space for alignment and other data types. */
	upload_data = (GLubyte *) malloc(w * h * 4 * 5);
	testImg = (GLubyte *) malloc(w * h * 4);

	/* Fill source tex images. Use only 7 bits of pseudo-randomness per
	 * component because we test GL_BYTE. */
	n = 0;
	for (j = 0; j < h; j++) {
		for (k = 0; k < w; k++) {
			/* On glibc, RAND_MAX is (1 << 31) - 1, which
			 * is good enough. */
			int data = rand();
			original_img[n + 0] = ((data >> 0) & 0x7f) * 0x81 >> 6;
			original_img[n + 1] = ((data >> 7) & 0x7f) * 0x81 >> 6;
			original_img[n + 2] = ((data >> 14) & 0x7f) * 0x81 >> 6;
			original_img[n + 3] = ((data >> 21) & 0x7f) * 0x81 >> 6;

			data = rand();
			updated_img[n + 0] = ((data >> 0) & 0x7f) * 0x81 >> 6;
			updated_img[n + 1] = ((data >> 7) & 0x7f) * 0x81 >> 6;
			updated_img[n + 2] = ((data >> 14) & 0x7f) * 0x81 >> 6;
			updated_img[n + 3] = ((data >> 21) & 0x7f) * 0x81 >> 6;

			n += 4;
		}
	}

	if (use_pbo) {
		glGenBuffers(1, &pbo);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
		glBufferData(GL_PIXEL_UNPACK_BUFFER,
			     w * h * 4 * 5,
			     NULL,
			     GL_STREAM_DRAW);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);


	/* draw original reference image */
	tex = create_texture(intFormat->internalformat, w, h,
			     GL_RGBA, original_img);
	glClear(GL_COLOR_BUFFER_BIT);
	draw_and_read_texture(w, h, original_ref);
	glDeleteTextures(1, &tex);

	/* draw updated reference image */
	tex = create_texture(intFormat->internalformat, w, h,
			     GL_RGBA, updated_img);
	glClear(GL_COLOR_BUFFER_BIT);
	draw_and_read_texture(w, h, updated_ref);
	glDeleteTextures(1, &tex);

	/* Test all source formats with type GL_UNSIGNED_BYTE. */
	for (n = 0; n < ARRAY_SIZE(test_src_formats); ++n) {
		const struct src_format_desc* srcFormat = &test_src_formats[n];
		if (srcFormat->enable && !*srcFormat->enable)
			continue;

		pass = test_formats_type(
			intFormat, w, h,
			srcFormat, GL_UNSIGNED_BYTE,
			original_img, original_ref, updated_img, updated_ref,
			pbo, updated_swz_ref, upload_data, testImg) && pass;
	}

	/* Test the GL_RGBA format with all other types. */
	for (n = 1; n < ARRAY_SIZE(test_types); ++n) {
		pass = test_formats_type(
			intFormat, w, h,
			&test_src_formats[0], test_types[n],
			original_img, original_ref, updated_img, updated_ref,
			pbo, updated_swz_ref, upload_data, testImg) && pass;
	}

	free(original_img);
	free(original_ref);
	free(updated_img);
	free(updated_ref);
	free(updated_swz_ref);
	free(upload_data);
	free(testImg);

	if (use_pbo)
		glDeleteBuffers(1, &pbo);

	return pass;
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;
	int i;

	/* Section 3.8.1 (Texture Image Specification) of the OpenGL 2.1
	 * specification says:
	 *
	 *     "For the purposes of decoding the texture image, TexImage2D is
	 *     equivalent to calling TexImage3D with corresponding arguments
	 *     and depth of 1, except that
	 *
	 *       ...
	 *
	 *      * UNPACK_SKIP_IMAGES is ignored."
	 */
	glPixelStorei(GL_UNPACK_SKIP_IMAGES, 1);

	glEnable(GL_TEXTURE_2D);

	/* loop over the format groups */
	for (i = 0; i < ARRAY_SIZE(core); ++i)
		pass = test_format(&core[i]) && pass;

	if (have_ARB_texture_rg) {
		for (i = 0; i < ARRAY_SIZE(arb_texture_rg); ++i)
			pass = test_format(&arb_texture_rg[i]) && pass;
	}

	glDisable(GL_TEXTURE_2D);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	int remaining_argc = 1;
	int i;

	srand(0); /* reproducibility of the first error */

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "pbo")) {
			piglit_require_extension("GL_ARB_pixel_buffer_object");
			use_pbo = true;
		} else {
			argv[remaining_argc++] = argv[i];
		}
	}

	fbo_formats_init(remaining_argc, argv, 0);
	(void) fbo_formats_display;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

#define CHECK_EXTENSION(name) \
	do { \
		have_ ## name = piglit_is_extension_supported("GL_" #name); \
		printf("GL_" #name " supported = %s\n", \
		       have_ ## name ? "yes" : "no"); \
	} while (false)

	CHECK_EXTENSION(ARB_texture_rg);
	CHECK_EXTENSION(EXT_bgra);
	CHECK_EXTENSION(EXT_abgr);

#undef CHECK_EXTENSION
}
