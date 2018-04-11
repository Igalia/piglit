/*
 * Copyright 2015 VMware, Inc.
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
 * Test glGetTextureSubImage() with most texture types.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
			       PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


/* XXX this could potentially be a piglit utility function */
static bool
minify(GLenum target, int level, int width, int height, int depth,
       int *mip_width, int *mip_height, int *mip_depth)
{
	switch (target) {
	case GL_TEXTURE_1D:
		assert(height == 1);
		assert(depth == 1);
		if (width >> level == 0)
			return false;
		*mip_width = width >> level;
		*mip_height = 1;
		*mip_depth = 1;
		return true;
	case GL_TEXTURE_1D_ARRAY:
		assert(depth == 1);
		if (width >> level == 0)
			return false;
		*mip_width = width >> level;
		*mip_height = height;
		*mip_depth = 1;
		return true;
	case GL_TEXTURE_2D:
		assert(depth == 1);
		if (width >> level == 0 && height >> level == 0)
			return false;
		*mip_width = MAX2(1, width >> level);
		*mip_height = MAX2(1, height >> level);
		*mip_depth = 1;
		return true;
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		if (width >> level == 0 && height >> level == 0)
			return false;
		*mip_width = MAX2(1, width >> level);
		*mip_height = MAX2(1, height >> level);
		*mip_depth = depth;
		return true;
	case GL_TEXTURE_3D:
		if (width >> level == 0 &&
		    height >> level == 0 &&
		    depth >> level == 0)
			return false;
		*mip_width = MAX2(1, width >> level);
		*mip_height = MAX2(1, height >> level);
		*mip_depth = MAX2(1, depth >> level);
		return true;
	case GL_TEXTURE_RECTANGLE:
		assert(depth == 1);
		if (level > 0)
			return false;
		*mip_width = width;
		*mip_height = height;
		*mip_depth = 1;
		return true;
	default:
		return false;
	}
}


static bool
test_getsubimage(GLenum target,
		 GLsizei width, GLsizei height, GLsizei depth,
		 GLenum intFormat)
{
	const GLint bufSize = width * height * depth * 4 * sizeof(GLubyte);
	GLubyte *texData = malloc(bufSize);
	GLubyte *refData = malloc(bufSize);
	GLubyte *testData = malloc(bufSize);
	GLuint tex;
	int i, level, bytes;
	bool pass = true;
	GLsizei mip_width, mip_height, mip_depth;

	printf("Testing %s %s %d x %d x %d\n",
	       piglit_get_gl_enum_name(target),
	       piglit_get_gl_enum_name(intFormat),
	       width, height, depth);

	/* initial texture data */
	for (i = 0; i < width * height * depth * 4; i++) {
		texData[i] = i & 0xff;
	}

	glGenTextures(1, &tex);
	glBindTexture(target, tex);

	mip_width = width;
	mip_height = height;
	mip_depth = depth;

	/* make mipmapped texture */
	for (level = 0; ; level++) {
		if (!minify(target, level, width, height, depth,
			    &mip_width, &mip_height, &mip_depth)) {
			break;
		}

		switch (target) {
		case GL_TEXTURE_1D:
			glTexImage1D(GL_TEXTURE_1D, level, intFormat,
				     mip_width, 0,
				     GL_RGBA, GL_UNSIGNED_BYTE, texData);
			break;
		case GL_TEXTURE_2D:
		case GL_TEXTURE_RECTANGLE:
		case GL_TEXTURE_1D_ARRAY:
			glTexImage2D(target, level, intFormat,
				     mip_width, mip_height, 0,
				     GL_RGBA, GL_UNSIGNED_BYTE, texData);
			break;
		case GL_TEXTURE_3D:
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_CUBE_MAP_ARRAY:
			glTexImage3D(target, level, intFormat,
				     mip_width, mip_height, mip_depth, 0,
				     GL_RGBA, GL_UNSIGNED_BYTE, texData);
			break;
		case GL_TEXTURE_CUBE_MAP:
			/* specify dimensions and format for all faces to make texture cube complete,
			   but specify data for only the +Y face as it is the only one read back */
			for (i = 0; i < 6; i++) {
				GLubyte *faceData = i == 2 ? texData : NULL;
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					     level, intFormat,
					     mip_width, mip_height, 0,
					     GL_RGBA, GL_UNSIGNED_BYTE,
					     faceData);
			}
			break;
		}
	}

	/* compare glGetTexImage() vs. glGetTextureSubImage() */
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	mip_width = width;
	mip_height = height;
	mip_depth = depth;

	for (level = 0; ; level++) {
		GLint x0, y0, z0, x1, y1, z1, w0, h0, w1, h1, d0, d1;

		if (!minify(target, level, width, height, depth,
			    &mip_width, &mip_height, &mip_depth)) {
			break;
		}

		/* compute pos/size of sub-regions */
		x0 = 0;
		y0 = 0;
		z0 = 0;
		x1 = MAX2(1, mip_width / 3);
		y1 = MAX2(1, mip_height / 3);
		z1 = MAX2(1, mip_depth / 3);
		if (intFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT) {
			/* x1, y1 must be a multipe of 4 */
			x1 &= ~0x3;
			y1 &= ~0x3;
		}

                /* Note that any of these widths, heights, depths can be zero
                 * but that's legal and should work fine.
                 */
		w0 = x1 - x0;
		w1 = mip_width - x1;
		h0 = y1 - y0;
		h1 = mip_height - y1;
		d0 = z1 - z0;
		d1 = mip_depth - z1;

		memset(refData, 0, bufSize);
		memset(testData, 0, bufSize);

		switch (target) {
		case GL_TEXTURE_1D:
			/*
			 * Get whole image (the reference)
			 */
			glGetTexImage(GL_TEXTURE_1D, level,
				      GL_RGBA, GL_UNSIGNED_BYTE, refData);

			/*
			 * Now get two sub-regions which should be equivalent
			 * to the whole reference image.
			 */

			/* left part */
			glPixelStorei(GL_PACK_SKIP_PIXELS, x0);
			glGetTextureSubImage(tex, level,
					     x0, 0, 0, w0, 1, 1,
					     GL_RGBA, GL_UNSIGNED_BYTE,
					     bufSize, testData);
			/* right part */
			glPixelStorei(GL_PACK_SKIP_PIXELS, x1);
			glGetTextureSubImage(tex, level,
					     x1, 0, 0, w1, 1, 1,
					     GL_RGBA, GL_UNSIGNED_BYTE,
					     bufSize, testData);

			/* defaults */
			glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

			/* now compare the images */
			bytes = mip_width * 4;
			if (memcmp(refData, testData, bytes)) {
				printf("Failure for GL_TEXTURE_1D:\n");
				pass = false;
			}
			break;

		case GL_TEXTURE_1D_ARRAY:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_RECTANGLE:
		case GL_TEXTURE_CUBE_MAP:
			glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
			glPixelStorei(GL_PACK_ROW_LENGTH, mip_width);

			if (target == GL_TEXTURE_CUBE_MAP) {
				/* only get +Y face */
				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                                              level, GL_RGBA, GL_UNSIGNED_BYTE,
                                              refData);
				z0 = 2; /* positive Y face */
			}
			else {
				/* Get whole texture */
				glGetTexImage(target, level,
					      GL_RGBA, GL_UNSIGNED_BYTE,
                                              refData);
				z0 = 0;
			}

			/*
			 * Now get four sub-regions which should be equivalent
			 * to the whole reference image.
			 */

			/* lower-left */
			glPixelStorei(GL_PACK_SKIP_PIXELS, x0);
			glPixelStorei(GL_PACK_SKIP_ROWS, y0);
			glGetTextureSubImage(tex, level,
					     x0, y0, z0, w0, h0, 1,
					     GL_RGBA, GL_UNSIGNED_BYTE,
					     bufSize, testData);
			/* lower-right */
			glPixelStorei(GL_PACK_SKIP_PIXELS, x1);
			glPixelStorei(GL_PACK_SKIP_ROWS, y0);
			glGetTextureSubImage(tex, level,
					     x1, y0, z0, w1, h0, 1,
					     GL_RGBA, GL_UNSIGNED_BYTE,
					     bufSize, testData);

			/* upper-left */
			glPixelStorei(GL_PACK_SKIP_PIXELS, x0);
			glPixelStorei(GL_PACK_SKIP_ROWS, y1);
			glGetTextureSubImage(tex, level,
					     x0, y1, z0, w0, h1, 1,
					     GL_RGBA, GL_UNSIGNED_BYTE,
					     bufSize, testData);

			/* upper-right */
			glPixelStorei(GL_PACK_SKIP_PIXELS, x1);
			glPixelStorei(GL_PACK_SKIP_ROWS, y1);
			glGetTextureSubImage(tex, level,
					     x1, y1, z0, w1, h1, 1,
					     GL_RGBA, GL_UNSIGNED_BYTE,
					     bufSize, testData);

			/* defaults */
			glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
			glPixelStorei(GL_PACK_SKIP_ROWS, 0);

			/* now compare the images */
			bytes = mip_width * mip_height * 4;
			if (memcmp(refData, testData, bytes)) {
				printf("Failure for %s\n",
				       piglit_get_gl_enum_name(target));
				pass = false;
			}

			break;

		case GL_TEXTURE_3D:
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_CUBE_MAP_ARRAY:
			glPixelStorei(GL_PACK_ROW_LENGTH, mip_width);
			glPixelStorei(GL_PACK_IMAGE_HEIGHT, mip_height);

			/*
			 * Get whole image (the reference)
			 */
			glGetTexImage(target, level,
				      GL_RGBA, GL_UNSIGNED_BYTE, refData);

			/*
			 * Now get four sub-regions which should be equivalent
			 * to the whole reference image.
			 */

			/* front-left block */
			glPixelStorei(GL_PACK_SKIP_PIXELS, x0);
			glPixelStorei(GL_PACK_SKIP_ROWS, y0);
			glPixelStorei(GL_PACK_SKIP_IMAGES, z0);
			glGetTextureSubImage(tex, level,
					     x0, y0, z0, w0, h0+h1, d0,
					     GL_RGBA, GL_UNSIGNED_BYTE,
					     bufSize, testData);
			/* front-right block */
			glPixelStorei(GL_PACK_SKIP_PIXELS, x1);
			glPixelStorei(GL_PACK_SKIP_ROWS, y0);
			glPixelStorei(GL_PACK_SKIP_IMAGES, z0);
			glGetTextureSubImage(tex, level,
					     x1, y0, 0, w1, h0+h1, d0,
					     GL_RGBA, GL_UNSIGNED_BYTE,
					     bufSize, testData);

			/* back-left block */
			glPixelStorei(GL_PACK_SKIP_PIXELS, x0);
			glPixelStorei(GL_PACK_SKIP_ROWS, y0);
			glPixelStorei(GL_PACK_SKIP_IMAGES, z1);
			glGetTextureSubImage(tex, level,
					     x0, y0, z1, w0, h0+h1, d1,
					     GL_RGBA, GL_UNSIGNED_BYTE,
					     bufSize, testData);

			/* back-right block */
			glPixelStorei(GL_PACK_SKIP_PIXELS, x1);
			glPixelStorei(GL_PACK_SKIP_ROWS, y0);
			glPixelStorei(GL_PACK_SKIP_IMAGES, z1);
			glGetTextureSubImage(tex, level,
					     x1, y0, z1, w1, h0+h1, d1,
					     GL_RGBA, GL_UNSIGNED_BYTE,
					     bufSize, testData);

			/* defaults */
			glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
			glPixelStorei(GL_PACK_SKIP_ROWS, 0);
			glPixelStorei(GL_PACK_SKIP_IMAGES, 0);

			/* now compare the images */
			bytes = mip_width * mip_height * mip_depth * 4;
			if (memcmp(refData, testData, bytes)) {
				printf("Failure for %s\n",
				       piglit_get_gl_enum_name(target));
				pass = false;
			}

			break;
		}

		/* Should be no GL errors */
		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			pass = false;
		}
	}

	free(texData);
	free(refData);
	free(testData);

	return pass;
}


void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_ARB_get_texture_sub_image");

	/* Test assorted targets, sizes (including NPOT) and internal formats */
	pass = test_getsubimage(GL_TEXTURE_1D, 64, 1, 1, GL_RGB) && pass;

	pass = test_getsubimage(GL_TEXTURE_2D, 256, 128, 1, GL_RGBA) && pass;

	pass = test_getsubimage(GL_TEXTURE_2D, 30, 40, 1, GL_ALPHA) && pass;

	pass = test_getsubimage(GL_TEXTURE_3D, 8, 4, 16, GL_RGBA) && pass;

	pass = test_getsubimage(GL_TEXTURE_RECTANGLE, 16, 8, 1, GL_RGB) && pass;

	pass = test_getsubimage(GL_TEXTURE_CUBE_MAP, 32, 32, 1, GL_RGB) && pass;

	if (piglit_is_extension_supported("GL_EXT_texture_array")) {
		pass = test_getsubimage(GL_TEXTURE_1D_ARRAY, 64, 9, 1, GL_ALPHA)
			&& pass;
		pass = test_getsubimage(GL_TEXTURE_2D_ARRAY, 32, 32, 9, GL_RGBA)
			&& pass;
	}

	if (piglit_is_extension_supported("GL_ARB_texture_cube_map_array")) {
		pass = test_getsubimage(GL_TEXTURE_CUBE_MAP_ARRAY,
                                        8, 8, 6, GL_RGBA) && pass;
		pass = test_getsubimage(GL_TEXTURE_CUBE_MAP_ARRAY,
                                        32, 32, 18, GL_ALPHA) && pass;
	}

	if (piglit_is_extension_supported("GL_EXT_texture_compression_s3tc")) {
		pass = test_getsubimage(GL_TEXTURE_2D, 128, 128, 1,
					GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
			&& pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* never called */
	return PIGLIT_PASS;
}
