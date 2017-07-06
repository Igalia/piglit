/*
 * Copyright © 2017 Intel Corporation
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

/** @file s3tc-targeted.c
 *
 * Tests the cases of S3TC DXT1 decompression in which the bitmap contains the
 * value b'11. The chosen tests help to determine that the color comparison
 * portion of decompression works correctly and that any internal driver
 * swizzling of the alpha channel is performed correctly.
 *
 * Bugzilla: https://bugs.freedesktop.org/show_bug.cgi?id=100925
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	/* We need OpenGL 1.3 for the *TexImage* functions used in this file. */
	config.supports_gl_compat_version = 13;
	config.requires_displayed_window = false;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static bool
test_block(GLenum internal_fmt, const char * base_fmt_str,
	   const uint8_t * dxt1_block, uint16_t expected_result)
{
	/* Upload the DXT1 block. */
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, internal_fmt, 1, 1, 0,
			       8 /* 64 bits */, dxt1_block);

	/* Decompress the only defined pixel in the DXT1 block. */
	uint16_t actual_pixel = 0xBEEF;
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4,
	              &actual_pixel);

	/* Test the result. */
	if (actual_pixel != expected_result) {
		fprintf(stderr, "Sampled %#.4x (R4G4B4A4_PACK32), but "
			"expected %#.4x from %s DXT1 texture.\n",
			actual_pixel, expected_result, base_fmt_str);
		return false;
	}

	return true;
}

#define TEST(fmt, block, pixel) \
	test_block(GL_COMPRESSED_ ## fmt ## _S3TC_DXT1_EXT, #fmt, block, pixel)

/* Test 4 out of 16 DXT1 decompression paths:
 *   (RGB0+2*RGB1)/3,   if color0  > color1 and code(x,y) == 3
 *   BLACK,             if color0 <= color1 and code(x,y) == 3
 */
enum piglit_result
piglit_display(void)
{
	/* Store the 64 bit DXT1 blocks to be tested. From the lowest address
	 * to the highest, the following bytes of interest are stored:
	 *    c0_lo, c0_hi, c1_lo, c1_hi, texels (0,0-4), ... 
	 */
	const uint8_t black_block[8]     = { 0xFF, 0xFF, 0xFF, 0xFF, 0x03, };
	const uint8_t one_third_block[8] = { 0xFF, 0xFF,    0,    0, 0x03, };

	const bool pass = TEST(RGB , black_block, 0x000F) &&
	                  TEST(RGBA, black_block, 0x0000) &&
	                  TEST(RGB , one_third_block, 0x555F) &&
	                  TEST(RGBA, one_third_block, 0x555F);
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_texture_compression_s3tc");
}
