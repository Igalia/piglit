/*
 * Copyright © 2011 VMware, Inc.
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
 *
 */


/**
 * This should expose any errors in texel addressing within a texture image
 * when calling glTexSubImage1D/2D/3D().
 *
 * Brian Paul
 * October 2011
 */


#include "piglit-util-gl.h"
#include "../fbo/fbo-formats.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

	config.window_width = 512;
	config.window_height = 512;

PIGLIT_GL_TEST_CONFIG_END

/**
 * This is a subset of the formats in fbo-formats.h
 * We don't test non-color, float, or int/uint textures at this time.
 */
static const struct test_desc texsubimage_test_sets[] = {
	{
		core,
		ARRAY_SIZE(core),
		"Core formats",
		GL_UNSIGNED_NORMALIZED,
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
		ext_texture_compression_rgtc,
		ARRAY_SIZE(ext_texture_compression_rgtc),
		"GL_EXT_texture_compression_rgtc",
		GL_UNSIGNED_NORMALIZED,
		{"GL_EXT_texture_compression_rgtc"}
	},
	{
		ext_texture_compression_latc,
		ARRAY_SIZE(ext_texture_compression_latc),
		"GL_EXT_texture_compression_latc",
		GL_UNSIGNED_NORMALIZED,
		{"GL_EXT_texture_compression_latc"}
	}
};


/**
 * XXX add this to piglit-util if useful elsewhere.
 */
static GLvoid
piglit_draw_rect_tex3d(float x, float y, float w, float h,
		       float tx, float ty, float tw, float th,
		       float tz0, float tz1)
{
	float verts[4][4];
	float tex[4][3];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	tex[0][0] = tx;
	tex[0][1] = ty;
	tex[0][2] = tz0;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	tex[1][0] = tx + tw;
	tex[1][1] = ty;
	tex[1][2] = tz1;
	verts[2][0] = x + w;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	tex[2][0] = tx + tw;
	tex[2][1] = ty + th;
	tex[2][2] = tz1;
	verts[3][0] = x;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;
	tex[3][0] = tx;
	tex[3][1] = ty + th;
	tex[3][2] = tz0;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glTexCoordPointer(3, GL_FLOAT, 0, tex);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


static GLboolean
equal_images(const GLubyte *img1, const GLubyte *img2,
             GLuint w, GLuint h, GLuint d)
{
	return memcmp(img1, img2, w*h*d*4) == 0;
}


/**
 * Get block size for compressed format.
 * \return GL_TRUE if format is compressed, GL_FALSE otherwise
 * XXX this could be a piglit util function if useful elsewhere.
 */
static GLboolean
get_format_block_size(GLenum format, GLuint *bw, GLuint *bh)
{
	switch (format) {
	case GL_COMPRESSED_RGB_FXT1_3DFX:
	case GL_COMPRESSED_RGBA_FXT1_3DFX:
		*bw = 8;
		*bh = 4;
		return GL_TRUE;
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
	case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
		*bw = 4;
		*bh = 4;
		return GL_TRUE;
	case GL_COMPRESSED_RED:
	case GL_COMPRESSED_RED_RGTC1_EXT:
	case GL_COMPRESSED_RG:
	case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
		*bw = 4;
		*bh = 4;
		return GL_TRUE;
	case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
	case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
		*bw = 4;
		*bh = 4;
		return GL_TRUE;
	default:
		*bw = *bh = 1;
		return GL_FALSE;
	}
}

/**
 * Draw each image of the texture to the framebuffer and then save the
 * entire thing to a buffer with glReadPixels().
 */
static void
draw_and_read_texture(GLuint w, GLuint h, GLuint d, GLubyte *ref)
{
	int i;

	for (i = 0; i < d; i++) {
		float tz = (i + 0.5f) / d;
		piglit_draw_rect_tex3d(0, i * h, /* x/y */
				       w, h,
				       0.0, 0.0, /* tx/ty */
				       1.0, 1.0, /* tw/th */
				       tz, tz /* tz0/tz1 */);
	}

	glReadPixels(0, 0, w, h * d, GL_RGBA, GL_UNSIGNED_BYTE, ref);
}

/**
 * Create a texture image with reference values.  Draw a textured quad.
 * Save reference image with glReadPixels().
 * Loop:
 *    replace a sub-region of the texture image with same values
 *    draw test textured quad
 *    read test image with glReadPixels
 *    compare reference image to test image
 * \param target  GL_TEXTURE_1D/2D/3D
 * \param intFormat  the internal texture format
 */
static GLboolean
test_format(GLenum target, GLenum intFormat)
{
	const GLenum srcFormat = GL_RGBA;
	GLuint w = 128, h = 64, d = 8;
	GLuint tex, i, j, k, n, t;
	GLubyte *img, *ref, *testImg;
	GLboolean pass = GL_TRUE;
	GLuint bw, bh, wMask, hMask, dMask;
	get_format_block_size(intFormat, &bw, &bh);
	wMask = ~(bw-1);
	hMask = ~(bh-1);
	dMask = ~0;

	if (target != GL_TEXTURE_3D)
		d = 1;
	if (target == GL_TEXTURE_1D)
		h = 1;

	img = (GLubyte *) malloc(w * h * d * 4);
	ref = (GLubyte *) malloc(w * h * d * 4);
	testImg = (GLubyte *) malloc(w * h * d * 4);

	/* fill source tex image */
	n = 0;
	for (i = 0; i < d; i++) {
		for (j = 0; j < h; j++) {
			for (k = 0; k < w; k++) {
				img[n++] = j * 4;
				img[n++] = k * 2;
				img[n++] = i * 16;
				img[n++] = 255;
			}
		}
	}

	glPixelStorei(GL_UNPACK_ROW_LENGTH, w);
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, h);

	glGenTextures(1, &tex);
	glBindTexture(target, tex);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_IMAGES, 0);
	if (target == GL_TEXTURE_1D) {
		glTexImage1D(target, 0, intFormat, w, 0,
			     srcFormat, GL_UNSIGNED_BYTE, img);
	}
	else if (target == GL_TEXTURE_2D) {
		glTexImage2D(target, 0, intFormat, w, h, 0,
			     srcFormat, GL_UNSIGNED_BYTE, img);
	}
	else if (target == GL_TEXTURE_3D) {
		glTexImage3D(target, 0, intFormat, w, h, d, 0,
			     srcFormat, GL_UNSIGNED_BYTE, img);
	}

	glEnable(target);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	/* draw reference image */
	glClear(GL_COLOR_BUFFER_BIT);
	draw_and_read_texture(w, h, d, ref);

	for (t = 0; t < 10; t++) {
		/* Choose random region of texture to update.
		 * Use sizes and positions that are multiples of
		 * the compressed block size.
		 */
		GLint tw = (rand() % w) & wMask;
		GLint th = (rand() % h) & hMask;
		GLint td = (rand() % d) & dMask;
		GLint tx = (rand() % (w - tw)) & wMask;
		GLint ty = (rand() % (h - th)) & hMask;
		GLint tz = (rand() % (d - td)) & dMask;

		assert(tx + tw <= w);
		assert(ty + th <= h);
		assert(tz + td <= d);

		/* replace texture region (with same data) */
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, tx);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, ty);
		glPixelStorei(GL_UNPACK_SKIP_IMAGES, tz);
		if (target == GL_TEXTURE_1D) {
			glTexSubImage1D(target, 0, tx, tw,
					srcFormat, GL_UNSIGNED_BYTE, img);
		}
		else if (target == GL_TEXTURE_2D) {
			glTexSubImage2D(target, 0, tx, ty, tw, th,
					srcFormat, GL_UNSIGNED_BYTE, img);
		}
		else if (target == GL_TEXTURE_3D) {
			glTexSubImage3D(target, 0, tx, ty, tz, tw, th, td,
					srcFormat, GL_UNSIGNED_BYTE, img);
		}

		/* draw test image */
		glClear(GL_COLOR_BUFFER_BIT);
		draw_and_read_texture(w, h, d, testImg);

		piglit_present_results();

		if (!equal_images(ref, testImg, w, h, d)) {
			printf("texsubimage failed\n");
			printf("  target: %s\n", piglit_get_gl_enum_name(target));
			printf("  internal format: %s\n", piglit_get_gl_enum_name(intFormat));
			printf("  region: %d, %d  %d x %d\n", tx, ty, tw, th);
			pass = GL_FALSE;
			break;
		}
	}

	glDisable(target);

	free(img);
	free(ref);
	free(testImg);

	glDeleteTextures(1, &tex);
	return pass;
}


/**
 * Test all formats in texsubimage_test_sets[] for the given
 * texture target.
 */
static GLboolean
test_formats(GLenum target)
{
	GLboolean pass = GL_TRUE;
	int i, j;

	/* loop over the format groups */
	for (i = 0; i < ARRAY_SIZE(texsubimage_test_sets); i++) {
		const struct test_desc *set = &texsubimage_test_sets[i];
		GLboolean skip = GL_FALSE;

		/* only test compressed formats with 2D textures */
		if (i > 0 && target != GL_TEXTURE_2D)
			continue;

		/* skip formats for unsupported extensions */
		for (j = 0; j < ARRAY_SIZE(set->ext); j++) {
			if (set->ext[j] &&
			    !piglit_is_extension_supported(set->ext[j])) {
				/* req'd extension not supported */
				skip = GL_TRUE;
				break;
			}
		}
		if (skip)
			continue;

		/* loop over formats in the set */
		for (j = 0; j < set->num_formats; j++) {
			if (!test_format(target,
					 set->format[j].internalformat)) {
				pass = GL_FALSE;
			}
		}
	}

	return pass;
}


enum piglit_result
piglit_display(void)
{
	static const GLenum targets[] = {
		GL_TEXTURE_1D,
		GL_TEXTURE_2D,
		GL_TEXTURE_3D
	};
	GLboolean pass = GL_TRUE;
	int i;

	/* Loop over 1/2/3D texture targets */
	for (i = 0; i < ARRAY_SIZE(targets); i++) {
		pass = test_formats(targets[i]) && pass;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	fbo_formats_init(argc, argv, 0);
	(void) fbo_formats_display;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
