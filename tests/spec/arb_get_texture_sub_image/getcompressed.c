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
 * Test glGetCompressedTextureSubImage() with 2D, 2D array, cubemap, and
 * cubemap array textures.
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


static bool
test_getsubimage(GLenum target,
		 GLsizei width, GLsizei height, GLsizei numSlices,
		 GLenum intFormat)
{
	GLubyte *texData;
	GLubyte *refData;
	GLubyte *testData;
	GLuint tex;
	int i, slice, compressedSize, compSize;
	int blockWidth, blockHeight, blockSize;
	bool pass = true;
	const int level = 0;
	int x0, y0, x1, y1, w0, h0, w1, h1;

	printf("Testing %s %s %d x %d\n",
	       piglit_get_gl_enum_name(target),
	       piglit_get_gl_enum_name(intFormat),
	       width, height);

	/* For all S3TC formats */
	blockWidth = blockHeight = 4;
	if (intFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ||
	    intFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) {
		blockSize = 8;
	}
	else {
		assert(intFormat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT ||
		       intFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
		blockSize = 16;
	}

	/* Size must be multiple of block dims */
	assert(width % blockWidth == 0);
	assert(height % blockHeight == 0);

	compressedSize = (width / blockWidth) * (height / blockHeight)
		* blockSize;
	if (0) {
		printf("byte per block row: %d\n",
		       (width / blockWidth) * blockSize);
		printf("compressed image size = %d\n", compressedSize);
	}

	/* initial texture data */
	texData = malloc(compressedSize);
	for (i = 0; i < compressedSize; i++) {
		texData[i] = (i+10) & 0xff;
	}

	glGenTextures(1, &tex);
	glBindTexture(target, tex);

	/* Define texture image */
	if (target == GL_TEXTURE_CUBE_MAP) {
		for (slice = 0; slice < 6; slice++) {
			glCompressedTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice,
				level, intFormat, width, height, 0,
				compressedSize, texData);
		}
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level,
					 GL_TEXTURE_COMPRESSED_IMAGE_SIZE,
					 &compSize);
		assert(numSlices == 6);
	}
	else if (target == GL_TEXTURE_CUBE_MAP_ARRAY) {
		assert(numSlices % 6 == 0);
		glCompressedTexImage3D(target, level, intFormat,
				       width, height, numSlices, 0,
				       compressedSize * numSlices, NULL);
		for (slice = 0; slice < numSlices; slice++) {
			glCompressedTexSubImage3D(target, level,
						  0, 0, slice,
						  width, height, 1,
						  intFormat,
						  compressedSize, texData);
		}
		glGetTexLevelParameteriv(target, level,
					 GL_TEXTURE_COMPRESSED_IMAGE_SIZE,
					 &compSize);
		compSize /= numSlices;
	}
	else if (target == GL_TEXTURE_2D_ARRAY) {
		glCompressedTexImage3D(target, level, intFormat,
				       width, height, numSlices, 0,
				       compressedSize * numSlices, NULL);
		for (slice = 0; slice < numSlices; slice++) {
			glCompressedTexSubImage3D(target, level,
						  0, 0, slice,
						  width, height, 1,
						  intFormat,
						  compressedSize, texData);
		}
		glGetTexLevelParameteriv(target, level,
					 GL_TEXTURE_COMPRESSED_IMAGE_SIZE,
					 &compSize);
		compSize /= numSlices;
	}
	else {
		assert(target == GL_TEXTURE_2D);
		glCompressedTexImage2D(target, level, intFormat,
				       width, height, 0,
				       compressedSize, texData);
		glGetTexLevelParameteriv(target, level,
					 GL_TEXTURE_COMPRESSED_IMAGE_SIZE,
					 &compSize);
		assert(numSlices == 1);
	}

	assert(compSize == compressedSize);

	/* Should be no GL errors */
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		pass = false;
	}

	refData = calloc(1, numSlices * compressedSize);
	testData = calloc(1, numSlices * compressedSize);

	/* compute pos/size of sub-regions */
	x0 = 0;
	y0 = 0;
	x1 = width / 4;  /* quarter width */
	y1 = height / 2;  /* half height */

	/* Position must be multiple of block dims */
	assert(x1 % blockWidth == 0);
	assert(y1 % blockHeight == 0);

	w0 = x1 - x0;
	w1 = width - x1;
	h0 = y1 - y0;
	h1 = height - y1;

	/* Sizes must be multiple of block dims */
	assert(w0 % blockWidth == 0);
	assert(w1 % blockWidth == 0);
	assert(h0 % blockHeight == 0);
	assert(h1 % blockHeight == 0);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ROW_LENGTH, width);
	glPixelStorei(GL_PACK_IMAGE_HEIGHT, height);
	glPixelStorei(GL_PACK_COMPRESSED_BLOCK_WIDTH, blockWidth);
	glPixelStorei(GL_PACK_COMPRESSED_BLOCK_HEIGHT, blockHeight);
	glPixelStorei(GL_PACK_COMPRESSED_BLOCK_SIZE, blockSize);

	/* Should be no GL errors */
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		pass = false;
	}

	/*
	 * Get whole compressed image (the reference)
	 */
	if (target == GL_TEXTURE_CUBE_MAP) {
		for (slice = 0; slice < 6; slice++) {
			glGetCompressedTexImage(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice,
				level,
				refData + slice * compressedSize);
		}
	}
	else {
		glGetCompressedTexImage(target, level, refData);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		pass = false;
	}

	/*
	 * Now get four sub-regions which should be equivalent
	 * to the whole reference image.
	 */

	/* lower-left */
	glPixelStorei(GL_PACK_SKIP_PIXELS, x0);
	glPixelStorei(GL_PACK_SKIP_ROWS, y0);
	glGetCompressedTextureSubImage(tex, level,
				       x0, y0, 0, w0, h0, numSlices,
				       numSlices * compressedSize, testData);

	/* lower-right */
	glPixelStorei(GL_PACK_SKIP_PIXELS, x1);
	glPixelStorei(GL_PACK_SKIP_ROWS, y0);
	glGetCompressedTextureSubImage(tex, level,
				       x1, y0, 0, w1, h0, numSlices,
				       numSlices * compressedSize, testData);

	/* upper-left */
	glPixelStorei(GL_PACK_SKIP_PIXELS, x0);
	glPixelStorei(GL_PACK_SKIP_ROWS, y1);
	glGetCompressedTextureSubImage(tex, level,
				       x0, y1, 0, w0, h1, numSlices,
				       numSlices * compressedSize, testData);

	/* upper-right */
	glPixelStorei(GL_PACK_SKIP_PIXELS, x1);
	glPixelStorei(GL_PACK_SKIP_ROWS, y1);
	glGetCompressedTextureSubImage(tex, level,
				       x1, y1, 0, w1, h1, numSlices,
				       numSlices * compressedSize, testData);

	/* defaults */
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);

	/* Should be no GL errors */
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		pass = false;
	}

	/* now compare the images */
	for (slice = 0; slice < numSlices; slice++) {
		int sliceStart = slice * compressedSize;
		if (memcmp(refData + sliceStart,
			   testData + sliceStart,
			   compressedSize)) {
			int i;
			for (i = 0; i < compressedSize; i++) {
				if (refData[sliceStart + i] !=
				    testData[sliceStart + i]) {
					printf("fail in slice/face %d at offset %d\n",
					       slice, i);
					printf("expected %d, found %d\n",
					       refData[sliceStart + i],
					       testData[sliceStart + i]);
					break;
				}
			}
			printf("Failure for %s %s\n",
			       piglit_get_gl_enum_name(target),
			       piglit_get_gl_enum_name(intFormat));
			pass = false;
		}
	}

	free(texData);
	free(refData);
	free(testData);

	glDeleteTextures(1, &tex);

	return pass;
}


void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_ARB_get_texture_sub_image");
	piglit_require_extension("GL_ARB_compressed_texture_pixel_storage");

	if (!piglit_is_extension_supported("GL_EXT_texture_compression_s3tc") &&
	    !(piglit_is_extension_supported("GL_EXT_texture_compression_dxt1") &&
	      piglit_is_extension_supported("GL_ANGLE_texture_compression_dxt3") &&
	      piglit_is_extension_supported("GL_ANGLE_texture_compression_dxt5"))) {
                printf("Test requires either GL_EXT_texture_compression_s3tc "
		       "or GL_EXT_texture_compression_dxt1, "
		       "GL_ANGLE_texture_compression_dxt3, and "
		       "GL_ANGLE_texture_compression_dxt5\n");
                piglit_report_result(PIGLIT_SKIP);
        }

	pass = test_getsubimage(GL_TEXTURE_2D, 256, 128, 1,
				GL_COMPRESSED_RGB_S3TC_DXT1_EXT) && pass;

	pass = test_getsubimage(GL_TEXTURE_2D, 80, 40, 1,
				GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) && pass;

	pass = test_getsubimage(GL_TEXTURE_2D, 32, 32, 1,
				GL_COMPRESSED_RGBA_S3TC_DXT3_EXT) && pass;

	pass = test_getsubimage(GL_TEXTURE_2D, 32, 32, 1,
				GL_COMPRESSED_RGBA_S3TC_DXT5_EXT) && pass;

	/* NOTE: texture rectangle not supported with S3TC */

	pass = test_getsubimage(GL_TEXTURE_CUBE_MAP, 16, 16, 6,
				GL_COMPRESSED_RGBA_S3TC_DXT5_EXT) && pass;

	if (piglit_is_extension_supported("GL_EXT_texture_array")) {
		pass = test_getsubimage(GL_TEXTURE_2D_ARRAY, 16, 32, 10,
					GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
			&& pass;

		pass = test_getsubimage(GL_TEXTURE_2D_ARRAY, 32, 16, 1,
					GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
			&& pass;
	}

	if (piglit_is_extension_supported("GL_ARB_texture_cube_map_array")) {
		pass = test_getsubimage(GL_TEXTURE_CUBE_MAP_ARRAY, 16, 16, 18,
					GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
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
