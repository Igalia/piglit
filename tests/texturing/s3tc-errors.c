/*
 * Copyright 2012 VMware, Inc.
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

/**
 * Verify error checking for compressed texture functions, using s3tc as
 * the specific compression formats.
 * Some rendering is also tested, but it's not the focus here.
 * Other compressed formats could be added as well (the test should probably
 * be renamed at that point.)
 *
 * Brian Paul
 * Sep 20, 2012
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

    config.supports_gl_compat_version = 10;

    config.window_width = 200;
    config.window_height = 200;
    config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const float red[4] =   {1.0, 0.0, 0.0, 1.0};
static const float green[4] = {0.0, 1.0, 0.0, 1.0};
static const float blue[4] =  {0.0, 0.0, 1.0, 1.0};
static const float white[4] = {1.0, 1.0, 1.0, 1.0};


static const GLenum s3tc_formats[] = {
	GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
};


static bool
check_rendering_(int width, int height, int line)
{
	const int w = width / 2 - 2;
	const int h = height / 2 - 2;
	bool pass = true;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glColor3f(1, 1, 1);

	/* draw the texture */
	piglit_draw_rect_tex(0, 0, width, height, 0, 0, 1, 1);

	/* NOTE: don't probe the border pixels of the quadrants just to
	 * avoid potential off-by one errors.
	 */

	/* lower-left red */
	pass = piglit_probe_rect_rgb(1, 1, w, h, red) && pass;

	/* lower-right green */
	pass = piglit_probe_rect_rgb(width/2 + 1, 1, w, h, green) && pass;

	/* upper-left blue */
	pass = piglit_probe_rect_rgb(1, height/2 + 1, w, h, blue) && pass;

	/* upper-right white */
	pass = piglit_probe_rect_rgb(width/2 + 1, height/2 + 1,
				     w, h, white) && pass;

	piglit_present_results();

	if (!pass) {
		printf("s3tc-errors failure at line %d\n", line);
	}

	return pass;
}


#define check_rendering(w, h) check_rendering_(w, h, __LINE__)


/**
 * Check for either of two expected GL errors.
 * XXX this could be a piglit util function
 */
static bool
check_gl_error2_(GLenum err1, GLenum err2, int line)
{
	GLenum err = glGetError();
	if (err != err1 && err != err2) {
		printf("Unexpected error %s at %s:%d\n",
		       piglit_get_gl_error_name(err), __FILE__, line);
		return false;
	}
	return true;
}

#define check_gl_error2(err1, err2)  check_gl_error2_(err1, err2, __LINE__)


static bool
test_format(int width, int height, GLfloat *image, GLenum requested_format)
{
	GLubyte *compressed_image;
	GLenum format2;
	int x, y, w, h;
	GLuint tex;
	bool pass = true;
	GLuint expected_size;
	GLint is_compressed;
	GLint compressed_size;
	GLint format;

	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, width);

	/* Setup initial texture */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, requested_format, width, height, 0,
		     GL_RGBA, GL_FLOAT, image);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	pass = check_rendering(width, height) && pass;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED,
				 &is_compressed);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT,
				 &format);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
				 GL_TEXTURE_COMPRESSED_IMAGE_SIZE,
				 &compressed_size);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	if (!is_compressed) {
		printf("Image was not compressed\n");
		pass = false;
	}

	if (format != requested_format) {
		printf("Internal Format mismatch. Found: 0x%04x Expected: 0x%04x\n",
		       format, requested_format);
		pass = false;
	}

	expected_size = piglit_compressed_image_size(requested_format, width,
			height);

	if (compressed_size != expected_size) {
		printf("Compressed image size mismatch. Found: %u Expected: %u\n",
		       compressed_size, expected_size);
		pass = false;
	}

	/* Use GL_TEXTURE_COMPRESSED_IMAGE_SIZE even if it wasn't what we
	 * expected to avoid corruption due to under-allocated buffer.
	 */
	compressed_image = malloc(compressed_size);

	/* Read back the compressed image data */
	glGetCompressedTexImage(GL_TEXTURE_2D, 0, compressed_image);

	/* Try texsubimage on 4-texel boundary - should work */
	x = 20;
	y = 12;
	w = 16;
	h = 8;
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, y);
	glTexSubImage2D(GL_TEXTURE_2D, 0,
			x, y, w, h,
			GL_RGBA, GL_FLOAT, image);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	pass = check_rendering(width, height) && pass;

	/* Try texsubimage on non 4-texel boundary - should not work */
	x = 10;
	y = 11;
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, y);
	glTexSubImage2D(GL_TEXTURE_2D, 0,
			x, y, w, h,
			GL_RGBA, GL_FLOAT, image);

	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	/* Try compressed subimage on 4-texel boundary - should work */
	x = 12;
	y = 8;
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, y);
	glCompressedTexSubImage2D(GL_TEXTURE_2D, 0,
				  x, y, w, h,
				  format,
				  piglit_compressed_image_size(format, w, h),
				  compressed_image +
				  piglit_compressed_pixel_offset(format, width, x, y));

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	pass = check_rendering(width, height) && pass;

	/* Try compressed subimage on non 4-texel boundary - should not work */
	x = 14;
	y = 9;
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, y);
	glCompressedTexSubImage2D(GL_TEXTURE_2D, 0,
				  x, y, w, h,
				  format,
				  piglit_compressed_image_size(format, w, h),
				  compressed_image +
				  piglit_compressed_pixel_offset(format, width, 0, 0));

	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	/* Try compressed subimage with size not a multiple of 4 -
	 * should not work
	 */
	x = 8;
	y = 8;
	w = 14;
	h = 10;
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, y);
	glCompressedTexSubImage2D(GL_TEXTURE_2D, 0,
				  x, y, w, h,
				  format,
				  piglit_compressed_image_size(format, 4, 4),
				  compressed_image +
				  piglit_compressed_pixel_offset(format, width, x, y));
	/* Note, we can get either of these errors depending on the order
	 * in which glCompressedTexSubImage parameters are checked.
	 * INVALID_OPERATION for the bad size or INVALID_VALUE for the
	 * wrong compressed image size.
	 */
	pass = check_gl_error2(GL_INVALID_OPERATION, GL_INVALID_VALUE) && pass;

	/* Try compressed subimage with invalid offset - should not work */
	x = -4;
	y = 8;
	w = 4;
	h = 4;
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glCompressedTexSubImage2D(GL_TEXTURE_2D, 0,
				  x, y, w, h,
				  format,
				  piglit_compressed_image_size(format, w, h),
				  compressed_image +
				  piglit_compressed_pixel_offset(format, width, 0, 0));

	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	/* Try compressed subimage with too large of image - should not work */
	x = 16;
	y = 8;
	w = width * 2;
	h = height * 2;
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, y);
	glCompressedTexSubImage2D(GL_TEXTURE_2D, 0,
				  x, y, w, h,
				  format,
				  piglit_compressed_image_size(format, w, h),
				  compressed_image +
				  piglit_compressed_pixel_offset(format, width, x, y));

	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	/* Try compressed subimage with different format - should not work */
	if (format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
		format2 = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	else
		format2 = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	x = 4;
	y = 4;
	w = 4;
	h = 4;
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, y);
	glCompressedTexSubImage2D(GL_TEXTURE_2D, 0,
				  x, y, w, h,
				  format2,
				  piglit_compressed_image_size(format2, w, h),
				  compressed_image +
				  piglit_compressed_pixel_offset(format2, width, x, y));

	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	/* Try zero-sized subimage - should not be an error */
	x = 4;
	y = 4;
	w = 0;
	h = 0;
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, y);
	glCompressedTexSubImage2D(GL_TEXTURE_2D, 0,
				  x, y, w, h,
				  format,
				  piglit_compressed_image_size(format, w, h),
				  compressed_image +
				  piglit_compressed_pixel_offset(format, width, x, y));

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;


	/* Try CompressedTexSubImage into level 1 (which is missing) */
	x = 0;
	y = 0;
	w = 4;
	h = 4;
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, y);
	glCompressedTexSubImage2D(GL_TEXTURE_2D, 1,
				  x, y, w, h,
				  format,
				  piglit_compressed_image_size(format, w, h),
				  compressed_image +
				  piglit_compressed_pixel_offset(format, width, x, y));

	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	/* Try CompressedTexImage of size zero - should not be an erorr */
	w = 0;
	h = 0;
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0,
			       piglit_compressed_image_size(format, w, h),
			       compressed_image);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Try CompressedTexImage with size which is a not a multiple of the
         * block size - should not be an erorr
         */
	w = width - 1;
	h = height - 1;
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0,
			       piglit_compressed_image_size(format, w, h),
			       compressed_image);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	pass = check_rendering(width, height) && pass;

	glDeleteTextures(1, &tex);

	free(compressed_image);

	return pass;
}


static bool
test_small_mipmap_level(void)
{
	bool pass = true;
	GLuint tex;
	GLubyte buf[100];
	int width, height;
	int format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;

	memset(buf, 0, sizeof(buf));

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	/* test sizes 1x1, 1x2, 2x1, .. 2x4, 4x4 */
	for (width = 1; width <= 4; width *= 2) {
		for (height = 1; height <= 4; height *= 2) {
			/* Initial image */
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height,
				     0, GL_RGBA, GL_UNSIGNED_BYTE, buf);

			pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

			/* Try TexSubImage of whole texture */
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
					GL_RGBA, GL_UNSIGNED_BYTE, buf);

			pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
		}
	}

	glDeleteTextures(1, &tex);

	return pass;
}


static bool
test_non_power_of_two(void)
{
	bool pass = true;

	if (piglit_is_extension_supported("GL_ARB_texture_non_power_of_two")) {
		GLuint tex;
		GLubyte buf[800];
		int width = 11, height = 14;
		int format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;

		memset(buf, 0, sizeof(buf));

		/* Setup initial texture */
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, buf);

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		/* Try TexSubImage of partial block on right edge */
		glTexSubImage2D(GL_TEXTURE_2D, 0,
				width-3, 0,  /* position */
				3, 4,        /* size */
				GL_RGBA, GL_UNSIGNED_BYTE, buf);

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		/* Try TexSubImage of partial block on top edge */
		glTexSubImage2D(GL_TEXTURE_2D, 0,
				0, height-2,  /* position */
				4, 2,         /* size */
				GL_RGBA, GL_UNSIGNED_BYTE, buf);

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		/* Try TexSubImage of larger partial block on right edge */
		glTexSubImage2D(GL_TEXTURE_2D, 0,
				width-3-4, 0,  /* position */
				3+4, 4,        /* size */
				GL_RGBA, GL_UNSIGNED_BYTE, buf);

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		/* Try TexSubImage of larger partial block on top edge */
		glTexSubImage2D(GL_TEXTURE_2D, 0,
				0, height-2-4,  /* position */
				4, 2+4,         /* size */
				GL_RGBA, GL_UNSIGNED_BYTE, buf);

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		glDeleteTextures(1, &tex);

	}

	return pass;
}


static bool
test_formats(void)
{
	const int num_formats = ARRAY_SIZE(s3tc_formats);
	const int width = 128, height = 64;
	GLfloat *image = piglit_rgbw_image(GL_RGBA, width, height,
					   GL_FALSE, /* alpha */
					   GL_UNSIGNED_NORMALIZED);
	int i;
	bool pass = true;

	for (i = 0; i < num_formats; i++) {
		const GLenum format = s3tc_formats[i];
		pass = test_format(width, height, image, format) && pass;
	}

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

	free(image);

	return pass;
}


enum piglit_result
piglit_display(void)
{
	bool pass = test_formats();
	pass = test_small_mipmap_level() && pass;
	pass = test_non_power_of_two() && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_texture_compression_s3tc");
}
